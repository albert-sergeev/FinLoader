#include "fasttasksholder.h"

//--------------------------------------------------------------------------------------------------------
FastTasksHolder::FastTasksHolder()
{

}

//--------------------------------------------------------------------------------------------------------
void FastTasksHolder::PacketReceived(dataFastLoadTask &data,
                                     Storage &stStore,
                                     std::map<int,std::shared_ptr<GraphHolder>>& Holders,
                                     BlockFreeQueue<dataFastLoadTask> &queueFastTasks,
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
        Holders[iTickerID] = std::make_shared<GraphHolder>(GraphHolder{iTickerID});
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
        mUtilityMutexes[iTickerID];
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
             /*&& data.RepushCount()<1000000000*/ && data.TimeTilCreate() < 120000ms
             ){
        /// 3.2 push again if wrong number
        data.IncrementRepush();
        queueFastTasks.Push(data);
        conditionFastData.notify_one();
    }
    else{
        if (llPacketsCounter < data.llPackesCounter){
            ThreadFreeCout pcout;
            pcout <<"{"<<iTickerID<<"} fail recovery. drop data\n";
        }
        llPacketsCounter = data.llPackesCounter + 1;
        /// 3.3 Adding
        if(data.vV.size()>0){
            //////////////////////////////////////////////////////
            /// adding to holder
            std::pair<std::time_t,std::time_t> pairRange;
            dataAmiPipeAnswer answ;
            answ.ptrHolder = std::make_shared<GraphHolder>(GraphHolder{iTickerID});

            if (!holder->AddBarsListsFast(data.vV,stHolderTimeSet,pairRange,*answ.ptrHolder) &&
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
            if (tLast < data.vV.back().Period()){
                while(!lastTimePacketReceived.compare_exchange_weak(tLast,data.vV.back().Period())){
                    if (tLast < data.vV.back().Period()){
                        break;
                    }
                }
            }

//            milliseconds tActivityCount = std::chrono::steady_clock::now() - dtActivity;
//            if (tActivityCount > 100ms){
//                dtActivity = std::chrono::steady_clock::now();
//                //
//                if (iTickerID == 1){ // TODO: delete. for tests
//                    dataAmiPipeAnswer answ;
//                    answ.SetTickerID(iTickerID);
//                    answ.SetType(dataAmiPipeAnswer::testTimeEvent);
//                    answ.SetTime(data.vV.back().Period());
//                    queuePipeAnswers.Push(answ);
//                }
//            }
            //////////////////////////////////////////////////////
            /// writing to database
            WriteVectorToStorage(iTickerID,tLastTime,strBuff,stTimeSet,stStore,data.vV,queuePipeAnswers);
            //////////////////////////////////////////////////////
            /// set new last time
            tLastTime = data.vV.back().Period();
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
    }
}
//--------------------------------------------------------------------------------------------------------
void FastTasksHolder::WriteVectorToStorage(int iTickerID,
                                           std::time_t tLastTime,
                                           std::string &strBuff,
                                           std::set<std::time_t>   & stTimeSet,
                                           Storage &stStore,
                                           std::vector<BarTick> v,
                                           BlockFreeQueue<dataAmiPipeAnswer>  &queuePipeAnswers)
{




    char *cOutBuff = strBuff.data();
    int iOutBuffPointer {0};

    BarTick bb(0,0,0);
    BarTickMemcopier bM(bb);
    int iBlockSize (  sizeof (Storage::data_type)
                    + sizeof (bb.Close())
                    + sizeof (bb.Volume())
                    + sizeof (bb.Period())
                    );

    Storage::data_type iState{Storage::data_type::new_sec};

    std::time_t tCurrentMonth = Storage::dateCastToMonth(tLastTime);

    std::time_t tMonth{0};
    std::time_t tSec{0};

    std::stringstream ssErr;

    Storage::MutexDefender<std::shared_lock<std::shared_mutex>> defSlk;
    Storage::MutexDefender<std::unique_lock<std::shared_mutex>> defUlk;

    bool bWasSuccessfull{true};

    for (const BarTick &b:v){
        tMonth = Storage::dateCastToMonth(b.Period());
        bb = b;
        tSec = b.Period();
        if (tCurrentMonth != tMonth){
            // switch month and write tail
            std::time_t tB{tSec}; // begin of needed to clean period
            std::time_t tE{tSec}; // end of needed to clean period

            auto ItB (stTimeSet.lower_bound(tSec));
            auto ItE (ItB);//(stTimeSet.upper_bound(tSec));

            if (ItB !=stTimeSet.end() && ItB != stTimeSet.begin()){
                --ItB;
                tB = (*ItB) + 1;
            }
            if (ItE != stTimeSet.end()){
                tE = (*ItE) - 1;
            }

            std::time_t tTmpMonth = tCurrentMonth;

            while(tTmpMonth < tMonth){
                iOutBuffPointer = createCleanPackets(tTmpMonth, cOutBuff,iOutBuffPointer,tB, tE);

                if (iOutBuffPointer > 0){
                    if(!stStore.WriteMemblockToStore(defSlk,defUlk,iTickerID, tCurrentMonth, cOutBuff,iOutBuffPointer, ssErr)){
                        dataAmiPipeAnswer answ;
                        answ.SetTickerID(iTickerID);
                        answ.SetType(dataAmiPipeAnswer::ErrMessage);
                        queuePipeAnswers.Push(answ);
                        bWasSuccessfull = false;
                        break; //exit
                    }
                }
                tTmpMonth   = Storage::dateAddMonth(tTmpMonth);
                iOutBuffPointer = 0;
            }
            iOutBuffPointer = createCleanPackets(tTmpMonth, cOutBuff,iOutBuffPointer,tB, tE);

            tCurrentMonth = tMonth;
        }
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
        if (stTimeSet.find(tSec) == stTimeSet.end()){

            std::time_t tB{tSec};
            std::time_t tE{tSec};

            auto ItB (stTimeSet.lower_bound(tSec));
            auto ItE (ItB);//(stTimeSet.upper_bound(tSec));

            if (ItB !=stTimeSet.end() && ItB != stTimeSet.begin()){
                --ItB;
                tB = (*ItB) + 1;
            }
            if (ItE != stTimeSet.end()){
                tE = (*ItE) - 1;
            }
            if (tB < tE || (tB == tE && tB != tSec)){
                iOutBuffPointer = createCleanPackets(tCurrentMonth, cOutBuff,iOutBuffPointer,tB, tE);
            }

            stTimeSet.emplace(tSec);
            iState  = Storage::data_type::new_sec;
        }
        else{
            iState  = Storage::data_type::usual;
        }

        {
            memcpy(cOutBuff + iOutBuffPointer,&iState,   sizeof (Storage::data_type));      iOutBuffPointer += sizeof (Storage::data_type);
            memcpy(cOutBuff + iOutBuffPointer,&bM.Close(),  sizeof (bM.Close()));           iOutBuffPointer += sizeof (bM.Close());
            memcpy(cOutBuff + iOutBuffPointer,&bM.Volume(), sizeof (bM.Volume()));          iOutBuffPointer += sizeof (bM.Volume());
            memcpy(cOutBuff + iOutBuffPointer,&bM.Period(), sizeof (bM.Period()));          iOutBuffPointer += sizeof (bM.Period());
        }
    }
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
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
