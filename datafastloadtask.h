#ifndef DATAFASTLOADTASK_H
#define DATAFASTLOADTASK_H

#include <vector>
#include<atomic>
#include "bartick.h"

using namespace std::chrono_literals;
using seconds=std::chrono::duration<double>;
using milliseconds=std::chrono::duration<double,
    std::ratio_multiply<seconds::period,std::milli>
    >;


inline std::mutex                   mutexConditionFastData;
inline std::condition_variable      conditionFastData;

inline std::atomic<std::time_t>     lastTimePacketReceived{0};

class dataFastLoadTask
{
public:
    enum eTaskType:int {Nop,NewTicks};
private:
    eTaskType taskType;
    std::chrono::time_point<std::chrono::steady_clock> dtCreationTime;
    int iRepushCount;

public:
    std::vector<BarTick>    vV;
    int                     iTickerID;
    long                    lTask;
    long long               llPackesCounter;

public:
    dataFastLoadTask(eTaskType Type = eTaskType::Nop):taskType{Type},iRepushCount{0}{ dtCreationTime = std::chrono::steady_clock::now();};
    dataFastLoadTask(const dataFastLoadTask &) = default;

    inline eTaskType TaskType()     const       { return taskType;};

    inline void IncrementRepush()               { iRepushCount++;}
    inline int RepushCount()        const       { return iRepushCount;}

    milliseconds TimeTilCreate()    const       { return std::chrono::steady_clock::now() - dtCreationTime;}
};

#endif // DATAFASTLOADTASK_H
