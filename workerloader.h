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



class workerLoader
{
public:
    workerLoader();


    static void workerDataBaseWork(BlockFreeQueue<dataFinLoadTask>              & queueFinQuotesLoad,
                       BlockFreeQueue<dataBuckgroundThreadAnswer>               &queueTrdAnswers,
                       Storage &stStore);


    static void workerAmiClient(BlockFreeQueue<dataFinLoadTask>                 &queueFinQuotesLoad,
                                BlockFreeQueue<dataBuckgroundThreadAnswer>      &queueTrdAnswers,
                                BlockFreeQueue<dataAmiPipeTask>                 &queuePipeTasks,
                                BlockFreeQueue<dataAmiPipeAnswer>               &queuePipeAnswers,
                                BlockFreeQueue<dataFastLoadTask>                &queueFastTasks,
                                AmiPipeHolder& pipesHolder);


    static void workerFastDataWork( BlockFreeQueue<dataFastLoadTask>            &queueFastTasks,
                                    BlockFreeQueue<dataBuckgroundThreadAnswer>  &queueTrdAnswers,
                                    BlockFreeQueue<dataAmiPipeAnswer>           &queuePipeAnswers,
                                    FastTasksHolder &fastHolder,
                                    Storage &stStore,
                                    std::map<int,std::shared_ptr<GraphHolder>>& Holders);




private:
    static void workerFinQuotesLoad(BlockFreeQueue<dataFinLoadTask>             &queueFinQuotesLoad,
                       BlockFreeQueue<dataBuckgroundThreadAnswer>               &queueTrdAnswers,
                       Storage &stStore,
                       dataFinLoadTask & data);

    static void workerFinQuotesCheck(BlockFreeQueue<dataFinLoadTask>            &queueFinQuotesLoad,
                       BlockFreeQueue<dataBuckgroundThreadAnswer>               &queueTrdAnswers,
                       Storage &stStore,
                       dataFinLoadTask & data);

    static void workerLoadFromStorage(BlockFreeQueue<dataFinLoadTask>           &queueTasks,
                                    BlockFreeQueue<dataBuckgroundThreadAnswer>  &queueTrdAnswers,
                                    Storage & stStore,
                                    dataFinLoadTask & data);

    static void workerLoadIntoGraph(BlockFreeQueue<dataFinLoadTask>             &queueTasks,
                                    BlockFreeQueue<dataBuckgroundThreadAnswer>  &queueTrdAnswers,
                                    Storage & stStore,
                                    dataFinLoadTask & data);

    static void workerOptimizeStorage(BlockFreeQueue<dataFinLoadTask>           &queueTasks,
                                    BlockFreeQueue<dataBuckgroundThreadAnswer>  &queueTrdAnswers,
                                    Storage & stStore,
                                    dataFinLoadTask & data);


    static bool compareBars(GraphHolder &hl,dataFinLoadTask & data,
                            std::vector<Bar> &vBars,
                            BlockFreeQueue<dataBuckgroundThreadAnswer>          &queueTrdAnswers,
                            std::chrono::time_point<std::chrono::steady_clock> dtActivity);
    template<typename T>
    static bool compareBarsT(GraphHolder &hl,dataFinLoadTask & data,
                             std::vector<Bar> &vBars,
                             BlockFreeQueue<dataBuckgroundThreadAnswer>         &queueTrdAnswers,
                             std::chrono::time_point<std::chrono::steady_clock> dtActivity);


    static int createCleanUpHeader(std::time_t tMonth, char* cBuff,std::time_t tBegin, std::time_t tEnd);

    static void workerEtalon(BlockFreeQueue<dataFinLoadTask>                    &queueFinQuotesLoad,
                       BlockFreeQueue<dataBuckgroundThreadAnswer>               &queueTrdAnswers);

};



#endif // WORKERLOADERFINAM_H
