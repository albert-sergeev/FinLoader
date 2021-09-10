#ifndef DATAFASTLOADTASK_H
#define DATAFASTLOADTASK_H

#include <vector>
#include "bartick.h"

class dataFastLoadTask
{
public:
    enum eTaskType:int {Nop,NewTicks};
private:
    eTaskType taskType;

public:
    std::vector<BarTick>    vV;
    int                     iTickerID;
    long                    lTask;
    long long               llPackesCounter;

public:
    dataFastLoadTask(eTaskType Type = eTaskType::Nop):taskType{Type}{;};
    dataFastLoadTask(const dataFastLoadTask &) = default;

    inline eTaskType TaskType() const {return taskType;};
};

#endif // DATAFASTLOADTASK_H
