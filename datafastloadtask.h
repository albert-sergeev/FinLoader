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

#ifndef DATAFASTLOADTASK_H
#define DATAFASTLOADTASK_H

#include <vector>
#include<atomic>
#include "bartick.h"

//using namespace std::chrono_literals;
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
