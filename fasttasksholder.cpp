#include "fasttasksholder.h"

//--------------------------------------------------------------------------------------------------------
FastTasksHolder::FastTasksHolder()
{

}

//--------------------------------------------------------------------------------------------------------
void FastTasksHolder::PacketReceived(dataFastLoadTask &data,Storage &stStore,std::map<int,std::shared_ptr<GraphHolder>>& Holders)
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
    }
    else{
        if (mTask[iTickerID] > data.lTask){
            return; // old task, drop packet
        }
        else if (mTask[iTickerID] < data.lTask){
            // new task. reinit
            mTask[iTickerID] = data.lTask;
            mPacketsCounter[iTickerID] = 1;
        }
    }
    /// 2. PacketNamberCheck
    if (mPacketsCounter[iTickerID] != data.llPackesCounter){
        /// 3.2 push again if wrong number
    }
    else{
        mPacketsCounter[iTickerID]++;
        /// 3.1 Adding
//        if (iTickerID == 1){
//            ThreadFreeCout pcout;
//            pcout <<"TickerID<"<<iTickerID<<"> task<"<<data.lTask<<"> packet {"<<data.llPackesCounter<<"}\n";
//        }

    }


    //std::map<int,std::time_t>   mLastTime;

}
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
