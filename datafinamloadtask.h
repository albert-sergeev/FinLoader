#ifndef DATAFINAMLOADTASK_H
#define DATAFINAMLOADTASK_H

#include <string>
#include <chrono>
#include<filesystem>

#include "storage.h"
#include "blockfreequeue.h"


class dataFinamLoadTask
{
private:
    Storage * stStore;

public:
    int             TickerID;
    std::string     sSign;
    int             iInterval;
    bool            bLoad;
    std::time_t     dtBegin;
    std::time_t     dtEnd;
    std::filesystem::path pathFileName;

    //--------------------------------------------------------------
    dataFinamLoadTask(){;};
    //--------------------------------------------------------------
    dataFinamLoadTask(const dataFinamLoadTask & o){

        TickerID        = o.TickerID;
        sSign           = o.sSign;
        iInterval       = o.iInterval;
        bLoad           = o.bLoad;
        dtBegin         = o.dtBegin;
        dtEnd           = o.dtEnd;
        stStore         = o.stStore;
        pathFileName    = o.pathFileName;
    };
    //--------------------------------------------------------------
    inline void SetStore(Storage * const stSt)      { stStore = stSt;};
    inline  Storage *  GetStorage() const           {return stStore;}
    //--------------------------------------------------------------
    dataFinamLoadTask& operator=(dataFinamLoadTask&) = delete;
};

#endif // DATAFINAMLOADTASK_H
