#ifndef FASTTASKSHOLDER_H
#define FASTTASKSHOLDER_H

#include<shared_mutex>
#include<map>
#include<set>

#include "datafastloadtask.h"
#include "storage.h"
#include "graphholder.h"
#include "blockfreequeue.h"
#include "dataamipipeanswer.h"
#include "databuckgroundthreadanswer.h"

inline std::shared_mutex mutexMainHolder;

class FastTasksHolder
{
    std::shared_mutex mutexUtilityMaps;

    std::map<int,std::shared_mutex>   mUtilityMutexes;

    std::map<int,long>   mTask;
    std::map<int,long long>   mPacketsCounter;
    std::map<int,std::time_t>   mLastTime;

    std::map<int,std::chrono::time_point<std::chrono::steady_clock>> mDtActivity;

    std::map<int,std::string> mBuff;
    std::map<int,std::set<std::time_t>> mHolderTimeSet;
    std::map<int,std::set<std::time_t>> mTimeSet;


    static const int iOutBuffMax {8192};


public:
    FastTasksHolder();

    void PacketReceived(dataFastLoadTask &data,
                        Storage &stStore,
                        std::map<int,std::shared_ptr<GraphHolder>>& Holders,
                        BlockFreeQueue<dataFastLoadTask> &queueFastTasks,
                        BlockFreeQueue<dataAmiPipeAnswer>  &queuePipeAnswers);

private:
    void WriteVectorToStorage(int iTickerID,
                              std::time_t tLastTime,
                              std::string &strBuff,
                              std::set<std::time_t>   & stTimeSet,
                              Storage &stStore,
                              std::vector<BarTick> v,
                              BlockFreeQueue<dataAmiPipeAnswer>  &queuePipeAnswers);

    int createCleanPackets(std::time_t tMonth,
                           char* cBuff,
                           int iBuffPointer,
                           std::time_t tBegin,
                           std::time_t tEnd);
};

#endif // FASTTASKSHOLDER_H
