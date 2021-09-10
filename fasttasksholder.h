#ifndef FASTTASKSHOLDER_H
#define FASTTASKSHOLDER_H

#include<shared_mutex>
#include<map>

#include "datafastloadtask.h"
#include "storage.h"
#include "graphholder.h"

inline std::shared_mutex mutexMainHolder;

class FastTasksHolder
{
    std::shared_mutex mutexUtilityMaps;

    std::map<int,std::shared_mutex>   mUtilityMutexes;

    std::map<int,long>   mTask;
    std::map<int,long long>   mPacketsCounter;
    std::map<int,std::time_t>   mLastTime;


public:
    FastTasksHolder();

    void PacketReceived(dataFastLoadTask &data,Storage &stStore,std::map<int,std::shared_ptr<GraphHolder>>& Holders);
};

#endif // FASTTASKSHOLDER_H
