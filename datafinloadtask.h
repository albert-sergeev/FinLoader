#ifndef DATAFINAMLOADTASK_H
#define DATAFINAMLOADTASK_H

#include <string>
#include <chrono>
#include<filesystem>
#include<QWidget>

#include "storage.h"
#include "blockfreequeue.h"


class dataFinLoadTask
{
private:
    Storage * stStore;
    QWidget * parentWnd;
    //void*  parentWnd;

public:

    enum TaskType:int { finQuotesImport = 1, finQuotesLoadFromStorage = 2, storageOptimisation = 3, LoadIntoGraph };

    int             TickerID;
    std::string     sSign;
    int             iInterval;
    bool            bLoad;
    std::time_t     dtBegin;
    std::time_t     dtEnd;
    std::filesystem::path pathFileName;

    dataFinQuotesParse parseData;

    TaskType taskType;

    //--------------------------------------------------------------
    dataFinLoadTask():stStore{nullptr},parentWnd{nullptr},parseData{nullptr,nullptr},taskType{TaskType::finQuotesImport}{;};
    //--------------------------------------------------------------
    dataFinLoadTask(const dataFinLoadTask & o):parseData{nullptr,nullptr}{

        parentWnd       = o.parentWnd;
        stStore         = o.stStore;

        TickerID        = o.TickerID;
        sSign           = o.sSign;
        iInterval       = o.iInterval;
        bLoad           = o.bLoad;
        dtBegin         = o.dtBegin;
        dtEnd           = o.dtEnd;
        pathFileName    = o.pathFileName;

        parseData       = o.parseData;

        taskType        = o.taskType;
    };
    //--------------------------------------------------------------
    inline void SetStore(Storage * const stSt)      { stStore = stSt;};
    inline  Storage *  GetStorage() const           {return stStore;}

    inline void SetParentWnd(QWidget * const wt)    { parentWnd = wt;};
    inline  QWidget*  GetParentWnd() const             {return parentWnd;}
    //--------------------------------------------------------------
    dataFinLoadTask& operator=(dataFinLoadTask&) = delete;
};

#endif // DATAFINAMLOADTASK_H
