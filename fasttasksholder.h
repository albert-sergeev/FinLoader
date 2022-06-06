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

///////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Used to communicate with AmiBrocker fromat pipes
///
/// Hub for events from multi thread pool about receiving packets from ami pipes.
/// Arranges ticks in correct order, call LSM-tree building procedures etc.
///
///
class FastTasksHolder
{
    // mutex to order incoming events
    std::shared_mutex mutexUtilityMaps;
    std::map<int,std::shared_mutex>   mUtilityMutexes;
    std::shared_mutex mutexSessionTables;
    //

    // core data of incoming(receiving) packet queue
    std::map<int,long>   mTask;
    std::map<int,long long>   mPacketsCounter;
    std::map<int,std::time_t>   mLastTime;
    //

    // activity indicator
    std::map<int,std::chrono::time_point<std::chrono::steady_clock>> mDtActivity;

    // sets for received packet analize
    std::map<int,std::string> mBuff;
    std::map<int,std::set<std::time_t>> mHolderTimeSet;
    std::map<int,std::set<std::time_t>> mTimeSet;

    // queue for packets received too early (for future use)
    std::map<int,std::map<long long,dataFastLoadTask>> mWrongNumberPacketsQueue;

    // pointer to work period (session/repo time)  of market
    std::shared_ptr<std::map<int,Market::SessionTable_type>>  shptrMappedRepoTable;

    static const int iOutBuffMax {8192};


public:
    FastTasksHolder();

    // main procedure to arrange received packets
    void PacketReceived(dataFastLoadTask &data,
                        Storage &stStore,
                        std::map<int,std::shared_ptr<GraphHolder>>& Holders,
                        BlockFreeQueue<dataFastLoadTask> &queueFastTasks,
                        BlockFreeQueue<dataAmiPipeAnswer>  &queuePipeAnswers);

    // initial function to set market work time (session/repo time)
    void setRepoTable(const std::map<int,Market::SessionTable_type>&  mappedRepoTable);

private:
    // utility to store received tick range to storage (to SS-table files)
    void WriteVectorToStorage(int iTickerID,
                              std::time_t tLastTime,
                              std::string &strBuff,
                              std::set<std::time_t>   & stTimeSet,
                              Storage &stStore,
                              std::vector<BarTick> v,
                              BlockFreeQueue<dataAmiPipeAnswer>  &queuePipeAnswers);

    // utility for construct clean range packet for SS-table
    int createCleanPackets(std::time_t tMonth,
                           char* cBuff,
                           int iBuffPointer,
                           std::time_t tBegin,
                           std::time_t tEnd);


    // reference to session table
    std::shared_ptr<std::map<int,Market::SessionTable_type>> getRepoTable();

    // procedure to filter tickets range by market work time table
    void FilterPacket(std::vector<BarTick> &v,Market::SessionTable_type &repoTable);
};

#endif // FASTTASKSHOLDER_H
