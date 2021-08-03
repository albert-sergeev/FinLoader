#ifndef WORKERLOADERFINAM_H
#define WORKERLOADERFINAM_H

#include"blockfreequeue.h"
#include"datafinamloadtask.h"


class workerLoaderFinam
{
public:
    workerLoaderFinam();

    static void worker(BlockFreeQueue<dataFinamLoadTask> & queueFilamLoad);
};



#endif // WORKERLOADERFINAM_H
