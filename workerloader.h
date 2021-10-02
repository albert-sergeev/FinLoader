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

#ifndef WORKERLOADERFINAM_H
#define WORKERLOADERFINAM_H


#include<chrono>


#include "blockfreequeue.h"
#include "datafinloadtask.h"
#include "databuckgroundthreadanswer.h"
#include "datafinquotesparse.h"
#include "amipipeholder.h"
#include "dataamipipetask.h"
#include "dataamipipeanswer.h"
#include "datafastloadtask.h"
#include "fasttasksholder.h"



////////////////////////////////////////////////////////////////////
/// \brief Container class for static functions used in background tasks for threadpool.
/// All included tasks can be interupted by setting thread_local this_thread_flagInterrup atomic flag (usualy set by threadpool class instance)
///
class workerLoader
{
public:
    workerLoader(){;};

    /////////////////////
    /// \brief workerDataBaseWork Base process for database works like importing/check/optimisation data tasks.
    /// Process have limited lifetime (until complite all tasks). If needed so, environment must run more instances of the process
    /// \param queueFinQuotesLoad queue for task from wich process pull next task until queue will become empty, then exit.
    /// \param queueTrdAnswers queue answer results
    /// \param stStore partal threadsafe class for work with database
    ///
    static void workerDataBaseWork(BlockFreeQueue<dataFinLoadTask>              & queueFinQuotesLoad,
                       BlockFreeQueue<dataBuckgroundThreadAnswer>               &queueTrdAnswers,
                       Storage &stStore);


    ////////////////////
    /// \brief workerAmiClient Client for pipes with trade data sources (fast time work).
    /// Process has !infinite! lifetime. Works until this_thread_flagInterrup is set
    /// !Warning! Only one instanse of process must be run, or you must divide serviced pipes between instances.
    /// \param queueFinQuotesLoad queue for database work tasks (need to give orders for optimisation and etc.)
    /// \param queueTrdAnswers  queue for answers
    /// \param queuePipeTasks queue for receive main task or process
    /// \param queuePipeAnswers queue for answers
    /// \param queueFastTasks queue for setting tasks to process received packets with data
    /// \param pipesHolder partal threadsafe class for work with connections (pipes)
    ///
    static void workerAmiClient(BlockFreeQueue<dataFinLoadTask>                 &queueFinQuotesLoad,
                                BlockFreeQueue<dataBuckgroundThreadAnswer>      &queueTrdAnswers,
                                BlockFreeQueue<dataAmiPipeTask>                 &queuePipeTasks,
                                BlockFreeQueue<dataAmiPipeAnswer>               &queuePipeAnswers,
                                BlockFreeQueue<dataFastLoadTask>                &queueFastTasks,
                                AmiPipeHolder& pipesHolder);


    //////////////////
    /// \brief workerFastDataWork process for parallel processing receiving data packets (long time work).
    /// Inserting into LSM-tree database, forming data chunk to didplay by GUI and etc.
    /// Process has !infinite! lifetime. Works until this_thread_flagInterrup is set.
    /// You can safely run several instances of process - the received chunks of data are correctly ordered and processed.
    /// \param queueFastTasks queue for main task
    /// \param queuePipeAnswers queue for answer
    /// \param fastHolder partal threadsafe class for work with data chunks
    /// \param stStore stStore partal threadsafe class for work with database
    /// \param Holders main in-memory storage of trade data
    ///
    static void workerFastDataWork( BlockFreeQueue<dataFastLoadTask>            &queueFastTasks,
                                    //BlockFreeQueue<dataBuckgroundThreadAnswer>  &queueTrdAnswers,
                                    BlockFreeQueue<dataAmiPipeAnswer>           &queuePipeAnswers,
                                    FastTasksHolder &fastHolder,
                                    Storage &stStore,
                                    std::map<int,std::shared_ptr<GraphHolder>>& Holders);




private:

    //////////////////////
    /// \brief workerFinQuotesLoad process to load history data from file. Used by workerDataBaseWork
    ///
    static void workerFinQuotesLoad(BlockFreeQueue<dataFinLoadTask>             &queueFinQuotesLoad,
                       BlockFreeQueue<dataBuckgroundThreadAnswer>               &queueTrdAnswers,
                       Storage &stStore,
                       dataFinLoadTask & data);

    /////////////////////
    /// \brief workerFinQuotesCheck  process to check history data from file whith database. Used by workerDataBaseWork
    ///
    static void workerFinQuotesCheck(BlockFreeQueue<dataFinLoadTask>            &queueFinQuotesLoad,
                       BlockFreeQueue<dataBuckgroundThreadAnswer>               &queueTrdAnswers,
                       Storage &stStore,
                       dataFinLoadTask & data);

    ////////////////////
    /// \brief workerLoadFromStorage  process to load data from SS-table part of database. Used by workerDataBaseWork
    ///
    static void workerLoadFromStorage(BlockFreeQueue<dataFinLoadTask>           &queueTasks,
                                    BlockFreeQueue<dataBuckgroundThreadAnswer>  &queueTrdAnswers,
                                    Storage & stStore,
                                    dataFinLoadTask & data);

    ////////////////////
    /// \brief workerLoadIntoGraph process the data read from the SS-table by the workerLoadFromStorage and form correct LSM-tree.  Used by workerDataBaseWork
    ///
    static void workerLoadIntoGraph(BlockFreeQueue<dataFinLoadTask>             &queueTasks,
                                    BlockFreeQueue<dataBuckgroundThreadAnswer>  &queueTrdAnswers,
                                    Storage & stStore,
                                    dataFinLoadTask & data);

    ///////////////////
    /// \brief workerOptimizeStorage optimise phisical structure of SS-tables (merge and seal process). Used by workerDataBaseWork
    ///
    static void workerOptimizeStorage(BlockFreeQueue<dataFinLoadTask>           &queueTasks,
                                    BlockFreeQueue<dataBuckgroundThreadAnswer>  &queueTrdAnswers,
                                    Storage & stStore,
                                    dataFinLoadTask & data);


    //////////////////
    /// \brief compareBars utility function to compare data storage with data set. Used by workerFinQuotesCheck
    ///
    static bool compareBars(GraphHolder &hl,dataFinLoadTask & data,
                            std::vector<Bar> &vBars,
                            BlockFreeQueue<dataBuckgroundThreadAnswer>          &queueTrdAnswers,
                            std::chrono::time_point<std::chrono::steady_clock> dtActivity);
    template<typename T>
    static bool compareBarsT(GraphHolder &hl,dataFinLoadTask & data,
                             std::vector<Bar> &vBars,
                             BlockFreeQueue<dataBuckgroundThreadAnswer>         &queueTrdAnswers,
                             std::chrono::time_point<std::chrono::steady_clock> dtActivity);
    //////////////////


    //////////////////
    /// \brief createCleanUpHeader Utility to form control title packet in SS-table. Used by workerFinQuotesLoad
    ///
    static int createCleanUpHeader(std::time_t tMonth, char* cBuff,std::time_t tBegin, std::time_t tEnd);

    /////////////////
    /// \brief workerEtalon typical structure of process function (for develop purpouse)
    ///
    static void workerEtalon(BlockFreeQueue<dataFinLoadTask>                    &queueFinQuotesLoad,
                       BlockFreeQueue<dataBuckgroundThreadAnswer>               &queueTrdAnswers);

};



#endif // WORKERLOADERFINAM_H
