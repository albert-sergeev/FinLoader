/****************************************************************************
*  This is part of FinLoader
*  Copyright (C) 2021  Albert Sergeyev
*  Contact: albert.s.sergeev@mail.ru
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <https://www.gnu.org/licenses/>.
****************************************************************************/

#include "fasttasksholder.h"

//--------------------------------------------------------------------------------------------------------
FastTasksHolder::FastTasksHolder()
{

}

//--------------------------------------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Processing data chunks(ticks) from pipe
/// \param data                 -   chunks of data
/// \param stStore              -   object to work with database files
/// \param Holders              -   current LSM-tree
/// \param queuePipeAnswers     -   queue to send events
///
void FastTasksHolder::PacketReceived(dataFastLoadTask &data,
                                     Storage &stStore,
                                     std::map<int,std::shared_ptr<GraphHolder>>& Holders,
                                     BlockFreeQueue<dataFastLoadTask> &/*queueFastTasks*/,
                                     BlockFreeQueue<dataAmiPipeAnswer>  &queuePipeAnswers
                                     )
{
    ////////////////////////////////////////////////////////////////////////////
    const int iTickerID = data.iTickerID;
    ////////////////////////////////////////////////////////////////////////////
    /// get holder or create if needed
    ///
    std::shared_lock lk(mutexMainHolder);
    if (Holders.find(iTickerID) == Holders.end()){
        lk.unlock();
        std::shared_lock lk2(mutexMainHolder);
        Holders[iTickerID] = std::make_shared<GraphHolder>(GraphHolder{iTickerID}); //create new branch to tree
        lk2.unlock();
        lk.lock();
    }
    //
    std::shared_ptr<GraphHolder> holder  = Holders[iTickerID];
    lk.unlock();
    ////////////////////////////////////////////////////////////////////////////
    /// 1. Lock global mutex
    /// 1.1 Getting special mutex (dont lock)
    /// 2. TaskCheck
    /// 2.1 Getting ref to variables
    /// 2.2 Unlock global mutex
    /// 2.3 lock special mutex
    /// 3. PacketNamberCheck
    /// 3.1 drop
    /// 3.2 push again if wrong number
    /// 3.3 Adding

    ////////////////////////////////////////////////////////////////////////////
    /// 1. Lock global mutex
    std::shared_lock lkMap (mutexUtilityMaps);

    /// get mutex for work with ticker
    if (mUtilityMutexes.find(iTickerID) == mUtilityMutexes.end()){
        lkMap.unlock();
        std::unique_lock lkMap2 (mutexUtilityMaps);
        mUtilityMutexes[iTickerID];         // create new mutex if needed
        lkMap2.unlock();
        lkMap.lock();
    }

    /// 2. TaskCheck
    if (mTask.find(iTickerID) == mTask.end()){
        mTask[iTickerID] = data.lTask;
        mPacketsCounter[iTickerID] = 1;
        mLastTime[iTickerID] = 0;
        mDtActivity[iTickerID] = std::chrono::steady_clock::now();
        mBuff[iTickerID].resize(iOutBuffMax);
        mHolderTimeSet[iTickerID].clear();
        mTimeSet[iTickerID].clear();
        mWrongNumberPacketsQueue[iTickerID].clear();
    }
    else{
        if (mTask[iTickerID] > data.lTask){
            return; // old task, drop packet
        }
        else if (mTask[iTickerID] < data.lTask){
            // new task. reinit
            mTask[iTickerID] = data.lTask;
            mPacketsCounter[iTickerID] = 1;
            mLastTime[iTickerID] = 0;
            mDtActivity[iTickerID] = std::chrono::steady_clock::now();
            mBuff[iTickerID].resize(iOutBuffMax);
            mHolderTimeSet[iTickerID].clear();
            mTimeSet[iTickerID].clear();
            mWrongNumberPacketsQueue[iTickerID].clear();
        }
    }


    ///////////////////////////////////////////////////////////////////////////
    /// 2.1 getting references to utility variables for thread safety
    ///
    std::shared_mutex       & mutexCurrent                          = mUtilityMutexes[iTickerID];
    //long                    & lTask                                 = mTask[iTickerID];
    long long               & llPacketsCounter                      = mPacketsCounter[iTickerID];
    std::time_t             & tLastTime                             = mLastTime[iTickerID];
    //std::chrono::time_point<std::chrono::steady_clock> & dtActivity = mDtActivity[iTickerID];
    std::string             & strBuff                               = mBuff[iTickerID];
    std::set<std::time_t>   & stTimeSet                             = mTimeSet[iTickerID];
    std::set<std::time_t>   & stHolderTimeSet                       = mHolderTimeSet[iTickerID];
    std::map<long long,dataFastLoadTask> &mOldPacketsQueue          = mWrongNumberPacketsQueue[iTickerID];

    std::shared_ptr<std::map<int,Market::SessionTable_type>> shptrRepoM = getRepoTable();

    ////////////////////////////////////////////////////////////////////////////
    /// 2.2 Unlock global mutex
    lkMap.unlock();
    /// 2.3 lock special mutex
    std::unique_lock lkCurrent (mutexCurrent); // to garanty we are the one ;)
    ////////////////////////////////////////////////////////////////////////////

    /// 3. PacketNamberCheck
    if (llPacketsCounter > data.llPackesCounter){
        /// 3.1 drop
    }
    else if (llPacketsCounter < data.llPackesCounter
             /*&& data.RepushCount()<1000000000 && data.TimeTilCreate() < 120000ms*/
             ){
        /// 3.2 push again if wrong number
        mOldPacketsQueue[data.llPackesCounter] = data;
//        data.IncrementRepush();
//        queueFastTasks.Push(data);
//        conditionFastData.notify_one();
    }
    else{
        llPacketsCounter = data.llPackesCounter + 1;
        /// 3.3 Adding
        ///

        std::vector<BarTick> v = std::move(data.vV);
        auto ItOldQueue = mOldPacketsQueue.begin();

        while(true){
            if(v.size()>0){
                std::time_t tLastPacketTime = v.back().Period();
                //////////////////////////////////////////////////////
                /// writing to database
                WriteVectorToStorage(iTickerID,tLastTime,strBuff,stTimeSet,stStore,v,queuePipeAnswers);
                //////////////////////////////////////////////////////
                /// set new last time
                tLastTime = v.back().Period();
                //////////////////////////////////////////////////////
                /// adding to holder
                std::pair<std::time_t,std::time_t> pairRange;
                dataAmiPipeAnswer answ;
                answ.ptrHolder = std::make_shared<GraphHolder>(GraphHolder{iTickerID});

                if (shptrRepoM){
                    FilterPacket(v,(*shptrRepoM)[iTickerID]);
                }
                else{
                    std::stringstream ss;
                    ss <<"Session (repo) table for ticker  {"<<iTickerID<<"} not set!";
                    answ.SetErrString(ss.str());
                    queuePipeAnswers.Push(answ);
                }

                if (!holder->AddBarsListsFast(v,stHolderTimeSet,pairRange,*answ.ptrHolder) &&
                        !this_thread_flagInterrup.isSet()){
                    dataAmiPipeAnswer answ; answ.SetTickerID(iTickerID); answ.SetType(dataAmiPipeAnswer::ErrMessage);
                    std::stringstream ss;
                    ss <<"Error during adding fast ticks to holder {"<<iTickerID<<"}";
                    answ.SetErrString(ss.str());
                    queuePipeAnswers.Push(answ);

                }
                //////////////////////////////////////////////////////
                /// send fast repaint event
                if (pairRange.first !=0 && pairRange.second != 0)
                {
                    answ.SetTickerID(iTickerID);
                    answ.SetType(dataAmiPipeAnswer::FastShowEvent);
                    answ.tBegin = pairRange.first;
                    answ.tEnd = pairRange.second;
                    queuePipeAnswers.Push(answ);
                }
                //////////////////////////////////////////////////////
                /// show activity
                std::time_t tLast = lastTimePacketReceived.load();
                if (tLast < tLastPacketTime){
                    while(!lastTimePacketReceived.compare_exchange_weak(tLast,tLastPacketTime)){
                        if (tLast < tLastPacketTime){
                            break;
                        }
                    }
                }
                //////////////////////////////////////////////////////
                /// cleaning too old sets for 24/7 worktime
                if(!stTimeSet.empty()){
                    std::time_t tD = Bar::DateAccommodate((*stTimeSet.rbegin()),Bar::pDay);
                    auto It = stTimeSet.lower_bound(tD);
                    if (It != stTimeSet.begin()){
                        stTimeSet.erase(stTimeSet.begin(),It);
                    }
                }
                if(!stHolderTimeSet.empty()){
                    std::time_t tD = Bar::DateAccommodate((*stHolderTimeSet.rbegin()),Bar::pDay);
                    auto It = stHolderTimeSet.lower_bound(tD);
                    if (It != stHolderTimeSet.begin()){
                        stHolderTimeSet.erase(stHolderTimeSet.begin(),It);
                    }
                }
                /////////////////////////////////////////////////////////////
            }
            // prepare next packet to read
            if(ItOldQueue == mOldPacketsQueue.end()) break;

            if (ItOldQueue->first == llPacketsCounter){
                v = std::move(ItOldQueue->second.vV);
                ++llPacketsCounter;
                ++ItOldQueue;
            }
            else{
                break;
            }
        }
        // erasing processed packets
        if (!mOldPacketsQueue.empty()){
            mOldPacketsQueue.erase(mOldPacketsQueue.begin(),ItOldQueue);
        }
    }
}
//--------------------------------------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Direct write new ticks to ss-table (bypassing lsm-tree)
/// \param iTickerID                    - current ticker
/// \param tLastTime                    - time of previous packet (for correct choseing of data file)
/// \param strBuff                      - work buffer
/// \param stTimeSet                    - set to store packets time
/// \param stStore                      - storage object
/// \param v                            - vector with packets to process
/// \param queuePipeAnswers             - queue to sending events
///
void FastTasksHolder::WriteVectorToStorage(int iTickerID,
                                           std::time_t tLastTime,
                                           std::string &strBuff,
                                           std::set<std::time_t>   & stTimeSet,
                                           Storage &stStore,
                                           std::vector<BarTick> v,
                                           BlockFreeQueue<dataAmiPipeAnswer>  &queuePipeAnswers)
{

    //////////////////////////////////////////////////////////////////////////////////////////
    // algotithm
    // 1. prepare variables
    // 2. loop through ticks and fill data flow to write ss-tree
    // 2.1 check time of tick to switch ss-file if nessesuary
    // 2.2 switch ss-files if needed
    // 2.3 write data flow to ss-file if buffer is full
    // 2.4 store current time in set and make clean packet if needed
    // 2.5 store data to flow
    // 3. store tail of data to flow



    /////////////////////////////////////////////////////////////////////////////////
    // 1. prepare variables

    char *cOutBuff = strBuff.data();                // work bussef
    int iOutBuffPointer {0};                        // pointer to buffer

    BarTick bb(0,0,0);                              // tick to process
    BarTickMemcopier bM(bb);                        // direct memory accessor
    int iBlockSize (  sizeof (Storage::data_type)   // size of tick
                    + sizeof (bb.Close())
                    + sizeof (bb.Volume())
                    + sizeof (bb.Period())
                    );

    Storage::data_type iState{Storage::data_type::new_sec};                 // indicator of dataflow type

    std::time_t tCurrentMonth = Storage::dateCastToMonth(tLastTime);        // current set of ss-files (ie divided by month)

    std::time_t tMonth{0};
    std::time_t tSec{0};

    std::stringstream ssErr;                                                // stream fro errorrs

    Storage::MutexDefender<std::shared_lock<std::shared_mutex>> defSlk;     // shared mutex for ss-tree
    Storage::MutexDefender<std::unique_lock<std::shared_mutex>> defUlk;     // uniqur mutex for ss-tree

    /////////////////////////////////////////////////////////////////////////////////

    // 2. loop through ticks and fill data flow to write ss-tree

    bool bWasSuccessfull{true};

    for (const BarTick &b:v){
        // 2.1 check time of tick to switch ss-file if nessesuary
        tMonth = Storage::dateCastToMonth(b.Period());
        bb = b;
        tSec = b.Period(); // time of current packet

        // 2.2 switch ss-files if needed
        if (tCurrentMonth != tMonth){

            //  calculate data period needed to clean for previous months (and write clean packet to ss-fie)

            // switch month and write tail
            std::time_t tB{tSec}; // begin of needed to clean period
            std::time_t tE{tSec}; // end of needed to clean period

            auto ItB (stTimeSet.lower_bound(tSec));
            auto ItE (ItB);//(stTimeSet.upper_bound(tSec));

            // get (time + 1 sec) from previous packet
            if (ItB !=stTimeSet.end() && ItB != stTimeSet.begin()){
                --ItB;
                tB = (*ItB) + 1;
            }
            // get (time - 1 sec) from next packet
            if (ItE != stTimeSet.end()){
                tE = (*ItE) - 1;
            }

            std::time_t tTmpMonth = tCurrentMonth;

            // loop by month previous months (ss-files are divided by month)
            while(tTmpMonth < tMonth){
                // prepare clean packet
                iOutBuffPointer = createCleanPackets(tTmpMonth, cOutBuff,iOutBuffPointer,tB, tE);

                //if there left data write it to ss-file
                if (iOutBuffPointer > 0){
                    if(!stStore.WriteMemblockToStore(defSlk,defUlk,iTickerID, tCurrentMonth, cOutBuff,iOutBuffPointer, ssErr)){
                        // send activity event
                        dataAmiPipeAnswer answ;
                        answ.SetTickerID(iTickerID);
                        answ.SetType(dataAmiPipeAnswer::ErrMessage);
                        queuePipeAnswers.Push(answ);
                        bWasSuccessfull = false;
                        break; //exit
                    }
                }
                // prepare to next
                tTmpMonth   = Storage::dateAddMonth(tTmpMonth);
                iOutBuffPointer = 0;
            }
            // in current month - create clean packet
            iOutBuffPointer = createCleanPackets(tTmpMonth, cOutBuff,iOutBuffPointer,tB, tE);

            tCurrentMonth = tMonth;
        }
        // 2.3 write data flow to ss-file if buffer is full
        else if( iOutBuffPointer + iBlockSize * 4 > iOutBuffMax){ // do write if full

            if(!stStore.WriteMemblockToStore(defSlk,defUlk,iTickerID, tCurrentMonth, cOutBuff,iOutBuffPointer, ssErr)){
                dataAmiPipeAnswer answ;
                answ.SetTickerID(iTickerID);
                answ.SetType(dataAmiPipeAnswer::ErrMessage);
                queuePipeAnswers.Push(answ);
                bWasSuccessfull = false;
                break; //exit
            }

            iOutBuffPointer = 0;
        }
        // 2.4 store current time in set and make clean packet if needed
        if (stTimeSet.find(tSec) == stTimeSet.end()){

            std::time_t tB{tSec};
            std::time_t tE{tSec};

            auto ItB (stTimeSet.lower_bound(tSec));
            auto ItE (ItB);//(stTimeSet.upper_bound(tSec));

            // get (time + 1 sec) from previous packet
            if (ItB !=stTimeSet.end() && ItB != stTimeSet.begin()){
                --ItB;
                tB = (*ItB) + 1;
            }
            // get (time - 1 sec) from next packet
            if (ItE != stTimeSet.end()){
                tE = (*ItE) - 1;
            }

            // if distance is more then 1 sec - make clean packet
            if (tB < tE || (tB == tE && tB != tSec)){
                iOutBuffPointer = createCleanPackets(tCurrentMonth, cOutBuff,iOutBuffPointer,tB, tE);
            }

            stTimeSet.emplace(tSec);
            iState  = Storage::data_type::new_sec;
        }
        else{
            iState  = Storage::data_type::usual;
        }

        // 2.5 store data to flow
        {
            memcpy(cOutBuff + iOutBuffPointer,&iState,   sizeof (Storage::data_type));      iOutBuffPointer += sizeof (Storage::data_type);
            memcpy(cOutBuff + iOutBuffPointer,&bM.Close(),  sizeof (bM.Close()));           iOutBuffPointer += sizeof (bM.Close());
            memcpy(cOutBuff + iOutBuffPointer,&bM.Volume(), sizeof (bM.Volume()));          iOutBuffPointer += sizeof (bM.Volume());
            memcpy(cOutBuff + iOutBuffPointer,&bM.Period(), sizeof (bM.Period()));          iOutBuffPointer += sizeof (bM.Period());
        }
    }

    // 3. store tail of data to flow
    if (bWasSuccessfull && iOutBuffPointer > 0){
        ////////////////////////
        /// write tail
        if(!stStore.WriteMemblockToStore(defSlk,defUlk,iTickerID, tCurrentMonth, cOutBuff,iOutBuffPointer, ssErr)){
            dataAmiPipeAnswer answ;
            answ.SetTickerID(iTickerID);
            answ.SetType(dataAmiPipeAnswer::ErrMessage);
            queuePipeAnswers.Push(answ);
            //bWasSuccessfull = false;
            //exit
        }
        /// ////////////////////////
    }

}
//--------------------------------------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Utility to create clean packet (to buffer)
/// \param tMonth               -   current month (ie ss-table file-set)
/// \param cBuff                -   buffer to use
/// \param iBuffPointer         -   write begin pointer
/// \param tBegin               -   clean time begin
/// \param tEnd                 -   clean time end
/// \return
///
int FastTasksHolder::createCleanPackets(std::time_t tMonth, char* cBuff,int iBuffPointer,std::time_t tBegin, std::time_t tEnd)
{

    if (tBegin < tMonth){
        tBegin = tMonth;
    }
    if (tEnd >= Storage::dateAddMonth(tMonth)){
        tEnd = Storage::dateAddMonth(tMonth)-1;
    }
    if (tBegin > tEnd) return iBuffPointer;


    Bar bb(0,0,0,0,0,0);
    BarMemcopier bM(bb);


    ;
    Storage::data_type iState;
    iState = Storage::data_type::del_from;

    bb.setPeriod(tBegin);

    memcpy(cBuff + iBuffPointer,&iState,   sizeof (Storage::data_type));      iBuffPointer += sizeof (Storage::data_type);
    memcpy(cBuff + iBuffPointer,&bM.Close(),  sizeof (bM.Close()));           iBuffPointer += sizeof (bM.Close());
    memcpy(cBuff + iBuffPointer,&bM.Volume(), sizeof (bM.Volume()));          iBuffPointer += sizeof (bM.Volume());
    memcpy(cBuff + iBuffPointer,&bM.Period(), sizeof (bM.Period()));          iBuffPointer += sizeof (bM.Period());

    iState = Storage::data_type::del_to;
    bb.setPeriod(tEnd);

    memcpy(cBuff + iBuffPointer,&iState,   sizeof (Storage::data_type));      iBuffPointer += sizeof (Storage::data_type);
    memcpy(cBuff + iBuffPointer,&bM.Close(),  sizeof (bM.Close()));           iBuffPointer += sizeof (bM.Close());
    memcpy(cBuff + iBuffPointer,&bM.Volume(), sizeof (bM.Volume()));          iBuffPointer += sizeof (bM.Volume());
    memcpy(cBuff + iBuffPointer,&bM.Period(), sizeof (bM.Period()));          iBuffPointer += sizeof (bM.Period());


    return iBuffPointer;
}
//--------------------------------------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Thread free utility to set repo table
/// \param mappedRepoTable
///
void FastTasksHolder::setRepoTable(const std::map<int,Market::SessionTable_type>&  mappedRepoTable)
{
    std::unique_lock lk(mutexSessionTables);

    shptrMappedRepoTable = std::make_shared<std::map<int,Market::SessionTable_type>>(std::map<int,Market::SessionTable_type>{mappedRepoTable});
}
//--------------------------------------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Thread free utility to get repo table
/// \return
///
std::shared_ptr<std::map<int,Market::SessionTable_type>> FastTasksHolder::getRepoTable()
{
    std::shared_lock lk(mutexSessionTables);
    std::shared_ptr<std::map<int,Market::SessionTable_type>> shRet = shptrMappedRepoTable;
    return shRet;
}
//--------------------------------------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Loops through vector and check if time is in session table. Filters packets inplace.
/// \param v            - vector to filter
/// \param repoTable    - session time table
///
void FastTasksHolder::FilterPacket(std::vector<BarTick> &v,Market::SessionTable_type &repoTable)
{
    auto It (v.begin());
    auto ItEnd = std::accumulate(v.begin(),v.end(),It,[&](auto &It,const auto &b){
                    if(!Market::IsInSessionTabe(repoTable,b.Period())){
                        if (&(*It) != &b) *It = b;
                        ++It;
                    }
                    return It;
                });

    //
    size_t tNewSize = std::distance(v.begin(),ItEnd);
    v.resize(tNewSize);
}
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
