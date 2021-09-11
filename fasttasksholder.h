#ifndef FASTTASKSHOLDER_H
#define FASTTASKSHOLDER_H

#include<shared_mutex>
#include<map>

#include "datafastloadtask.h"
#include "storage.h"
#include "graphholder.h"
#include "blockfreequeue.h"
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

public:
    FastTasksHolder();

    void PacketReceived(dataFastLoadTask &data,
                        Storage &stStore,
                        std::map<int,std::shared_ptr<GraphHolder>>& Holders,
                        BlockFreeQueue<dataFastLoadTask> &queueFastTasks,
                        BlockFreeQueue<dataBuckgroundThreadAnswer>  &queueTrdAnswers);
};

#endif // FASTTASKSHOLDER_H
