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
    /// 1. TaskCheck
    /// 2. PacketNamberCheck
    /// 3.1 Adding
    /// 3.2 push again if wrong number

    ////////////////////////////////////////////////////////////////////////////
    /// get mutex for work with ticker
    std::shared_lock lkMap (mutexUtilityMaps);
    if (mUtilityMutexes.find(iTickerID) == mUtilityMutexes.end()){
        lkMap.unlock();
        std::unique_lock lkMap2 (mutexUtilityMaps);
        mUtilityMutexes[iTickerID];
        lkMap2.unlock();
        lkMap.lock();
    }
    std::shared_mutex &mutexCurrent = mUtilityMutexes[iTickerID];
    lkMap.unlock();
    std::unique_lock lkCurrent (mutexCurrent);
    ////////////////////////////////////////////////////////////////////////////
    /// 1. TaskCheck
    if (mTask.find(iTickerID) == mTask.end()){
        mTask[iTickerID] = data.lTask;
        mPacketsCounter[iTickerID] = 1;
        mDtActivity[iTickerID] = std::chrono::steady_clock::now();
        mLastTime[iTickerID] = 0;
    }
    else{
        if (mTask[iTickerID] > data.lTask){
            return; // old task, drop packet
        }
        else if (mTask[iTickerID] < data.lTask){
            // new task. reinit
            mTask[iTickerID] = data.lTask;
            mPacketsCounter[iTickerID] = 1;
            mDtActivity[iTickerID] = std::chrono::steady_clock::now();
            mLastTime[iTickerID] = 0;
        }
    }
    /// 2. PacketNamberCheck
    if (mPacketsCounter[iTickerID] > data.llPackesCounter){
        // drop
    }
    else if (mPacketsCounter[iTickerID] < data.llPackesCounter &&
             /*data.RepushCount()<1000000 &&*/ data.TimeTilCreate() < 2000ms
             ){
        /// 3.2 push again if wrong number
        data.IncrementRepush();
        queueFastTasks.Push(data);
        conditionFastData.notify_one();
    }
    else{
        if (mPacketsCounter[iTickerID] < data.llPackesCounter){
            ThreadFreeCout pcout;
            pcout <<"{"<<iTickerID<<"} fail recovery. drop data\n";
        }
        mPacketsCounter[iTickerID] = data.llPackesCounter + 1;
        /// 3.1 Adding
        if(data.vV.size()>0){
            // calculate range
            std::time_t tBegin{0};
            if (mLastTime[iTickerID] != 0){
                tBegin = mLastTime[iTickerID] + 1;
            }
            else{
                tBegin = data.vV.front().Period();
            }
            std::time_t tEnd = data.vV.back().Period();
            //////////////////////////////////////////////////////
            std::vector<std::vector<BarTick>> vvV;
            vvV.emplace_back(std::move(data.vV));
            holder->AddBarsLists(vvV,tBegin,tEnd,true);
            //
            milliseconds tActivityCount = std::chrono::steady_clock::now() - mDtActivity[iTickerID];
            if (tActivityCount > 100ms){
                mDtActivity[iTickerID] = std::chrono::steady_clock::now();
                //
//                dataBuckgroundThreadAnswer dt(iTickerID,dataBuckgroundThreadAnswer::eAnswerType::storagLoadToGraphEnd,nullptr);
//                dt.SetBeginDate(tBegin);
//                dt.SetEndDate(tEnd);
//                dt.SetSuccessfull(true);
//                queueTrdAnswers.Push(dt);
            }
            //////////////////////////////////////////////////////
            WriteVectorToStorage(iTickerID,mLastTime[iTickerID],stStore,vvV.back(),queuePipeAnswers);
            //////////////////////////////////////////////////////
            mLastTime[iTickerID] = tEnd;
        }
    }
}
//--------------------------------------------------------------------------------------------------------
void FastTasksHolder::WriteVectorToStorage(int iTickerID,
                                           std::time_t tLastTime,
                                           Storage &stStore,
                                           std::vector<BarTick> v,
                                           BlockFreeQueue<dataAmiPipeAnswer>  &queuePipeAnswers)
{
    const int iOutBuffMax {8192};

    if (mBuff.find(iTickerID) == mBuff.end()){
        mBuff[iTickerID].resize(iOutBuffMax);
    }

    char *cOutBuff = mBuff[iTickerID].data();
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
    std::time_t tCurrSec = tLastTime;

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
            if(!stStore.WriteMemblockToStore(defSlk,defUlk,iTickerID, tCurrentMonth, cOutBuff,iOutBuffPointer, ssErr)){
                dataAmiPipeAnswer answ;
                answ.SetTickerID(iTickerID);
                answ.setType(dataAmiPipeAnswer::ErrMessage);
                queuePipeAnswers.Push(answ);
                bWasSuccessfull = false;
                break; //exit
            }

            tCurrentMonth = tMonth;
        }
        else if( iOutBuffPointer + iBlockSize > iOutBuffMax){ // do write to iSecChangedPointer

            if(!stStore.WriteMemblockToStore(defSlk,defUlk,iTickerID, tCurrentMonth, cOutBuff,iOutBuffPointer, ssErr)){
                dataAmiPipeAnswer answ;
                answ.SetTickerID(iTickerID);
                answ.setType(dataAmiPipeAnswer::ErrMessage);
                queuePipeAnswers.Push(answ);
                bWasSuccessfull = false;
                break; //exit
            }

            iOutBuffPointer = 0;
        }
        if(tCurrSec != tSec){
            tCurrSec = tSec;
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
            answ.setType(dataAmiPipeAnswer::ErrMessage);
            queuePipeAnswers.Push(answ);
            bWasSuccessfull = false;
            //exit
        }
        /// ////////////////////////
    }

}
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
