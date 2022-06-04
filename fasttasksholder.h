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
    std::shared_mutex mutexSessionTables;

    std::map<int,long>   mTask;
    std::map<int,long long>   mPacketsCounter;
    std::map<int,std::time_t>   mLastTime;

    std::map<int,std::chrono::time_point<std::chrono::steady_clock>> mDtActivity;

    std::map<int,std::string> mBuff;
    std::map<int,std::set<std::time_t>> mHolderTimeSet;
    std::map<int,std::set<std::time_t>> mTimeSet;

    std::map<int,std::map<long long,dataFastLoadTask>> mWrongNumberPacketsQueue;

    std::shared_ptr<std::map<int,Market::SessionTable_type>>  shptrMappedRepoTable;

    static const int iOutBuffMax {8192};


public:
    FastTasksHolder();

    void PacketReceived(dataFastLoadTask &data,
                        Storage &stStore,
                        std::map<int,std::shared_ptr<GraphHolder>>& Holders,
                        BlockFreeQueue<dataFastLoadTask> &queueFastTasks,
                        BlockFreeQueue<dataAmiPipeAnswer>  &queuePipeAnswers);

    void setRepoTable(const std::map<int,Market::SessionTable_type>&  mappedRepoTable);

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


    std::shared_ptr<std::map<int,Market::SessionTable_type>> getRepoTable();

    void FilterPacket(std::vector<BarTick> &v,Market::SessionTable_type &repoTable);
};

#endif // FASTTASKSHOLDER_H
