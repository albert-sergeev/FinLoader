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
                                     BlockFreeQueue<dataBuckgroundThreadAnswer>  &queueTrdAnswers
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
        if(data.vV.size()>0){
            mLastTime[iTickerID] = data.vV.back().Period();
        }
        else{
            mLastTime[iTickerID] = 0;
        }
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
            if(data.vV.size()>0){
                mLastTime[iTickerID] = data.vV.front().Period();
            }
            else{
                mLastTime[iTickerID] = 0;
            }
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
        mPacketsCounter[iTickerID] = data.llPackesCounter + 1;
        /// 3.1 Adding
        if(data.vV.size()>0){
            // calculate range
            std::time_t tBegin{0};
            if (mLastTime[iTickerID] != 0){
                tBegin = (mLastTime[iTickerID] + 1 ) < data.vV.front().Period() ?
                            (mLastTime[iTickerID] + 1 ) : data.vV.front().Period();
            }
            else{
                tBegin = data.vV.front().Period();
            }
            std::time_t tEnd = data.vV.back().Period();
            //////////////////////////////////////////////////////
            std::vector<std::vector<BarTick>> vvV;
            vvV.emplace_back(std::move(data.vV));
            holder->AddBarsLists(vvV,tBegin,tEnd);
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
            mLastTime[iTickerID] = tEnd;
        }
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
//--------------------------------------------------------------------------------------------------------
