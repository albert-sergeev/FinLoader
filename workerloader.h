#ifndef WORKERLOADERFINAM_H
#define WORKERLOADERFINAM_H

#include"blockfreequeue.h"
#include"datafinloadtask.h"
#include "databuckgroundthreadanswer.h"
#include "datafinquotesparse.h"


class workerLoader
{
public:
    workerLoader();


    static void workerFinQuotesLoad(BlockFreeQueue<dataFinLoadTask> & queueFinQuotesLoad,
                       BlockFreeQueue<dataBuckgroundThreadAnswer> &queueTrdAnswers,
                       Storage &stStore);

    static void workerEtalon(BlockFreeQueue<dataFinLoadTask> & queueFinQuotesLoad,
                       BlockFreeQueue<dataBuckgroundThreadAnswer> &queueTrdAnswers);
};



#endif // WORKERLOADERFINAM_H
