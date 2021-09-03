#ifndef WORKERLOADERFINAM_H
#define WORKERLOADERFINAM_H


#include<chrono>


#include "blockfreequeue.h"
#include "datafinloadtask.h"
#include "databuckgroundthreadanswer.h"
#include "datafinquotesparse.h"



class workerLoader
{
public:
    workerLoader();


    static void workerDataBaseWork(BlockFreeQueue<dataFinLoadTask> & queueFinQuotesLoad,
                       BlockFreeQueue<dataBuckgroundThreadAnswer> &queueTrdAnswers,
                       Storage &stStore);

private:
    static void workerFinQuotesLoad(BlockFreeQueue<dataFinLoadTask> & queueFinQuotesLoad,
                       BlockFreeQueue<dataBuckgroundThreadAnswer> &queueTrdAnswers,
                       Storage &stStore,
                       dataFinLoadTask & data);

    static void workerFinQuotesCheck(BlockFreeQueue<dataFinLoadTask> & queueFinQuotesLoad,
                       BlockFreeQueue<dataBuckgroundThreadAnswer> &queueTrdAnswers,
                       Storage &stStore,
                       dataFinLoadTask & data);

    static void workerLoadFromStorage(BlockFreeQueue<dataFinLoadTask> & queueTasks,
                                    BlockFreeQueue<dataBuckgroundThreadAnswer> &queueTrdAnswers,
                                    Storage & stStore,
                                    dataFinLoadTask & data);

    static void workerLoadIntoGraph(BlockFreeQueue<dataFinLoadTask> & queueTasks,
                                    BlockFreeQueue<dataBuckgroundThreadAnswer> &queueTrdAnswers,
                                    Storage & stStore,
                                    dataFinLoadTask & data);

    static void workerOptimizeStorage(BlockFreeQueue<dataFinLoadTask> & queueTasks,
                                    BlockFreeQueue<dataBuckgroundThreadAnswer> &queueTrdAnswers,
                                    Storage & stStore,
                                    dataFinLoadTask & data);


    static bool compareBars(GraphHolder &hl,dataFinLoadTask & data,std::vector<Bar> &vBars,BlockFreeQueue<dataBuckgroundThreadAnswer> &queueTrdAnswers,
                            std::chrono::time_point<std::chrono::steady_clock> dtActivity);
    template<typename T>
    static bool compareBarsT(GraphHolder &hl,dataFinLoadTask & data,std::vector<Bar> &vBars,BlockFreeQueue<dataBuckgroundThreadAnswer> &queueTrdAnswers,
                             std::chrono::time_point<std::chrono::steady_clock> dtActivity);


    static int createCleanUpHeader(std::time_t tMonth, char* cBuff,std::time_t tBegin, std::time_t tEnd);

    static void workerEtalon(BlockFreeQueue<dataFinLoadTask> & queueFinQuotesLoad,
                       BlockFreeQueue<dataBuckgroundThreadAnswer> &queueTrdAnswers);

};



#endif // WORKERLOADERFINAM_H
