#ifndef WORKERLOADERFINAM_H
#define WORKERLOADERFINAM_H

#include"blockfreequeue.h"
#include"datafinamloadtask.h"
#include "databuckgroundthreadanswer.h"


class workerLoaderFinam
{
public:
    workerLoaderFinam();

    static void worker(BlockFreeQueue<dataFinamLoadTask> & queueFilamLoad,
                       BlockFreeQueue<dataBuckgroundThreadAnswer> &queueTrdAnswers);
};



#endif // WORKERLOADERFINAM_H
