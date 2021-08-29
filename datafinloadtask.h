#ifndef DATAFINAMLOADTASK_H
#define DATAFINAMLOADTASK_H

#include <string>
#include <chrono>
#include<filesystem>
#include<QWidget>

#include "storage.h"
#include "graphholder.h"
#include "blockfreequeue.h"


class dataFinLoadTask
{
private:
    Storage * stStore;
    QWidget * parentWnd;

public:

    enum TaskType:int { finQuotesImport = 1, finQuotesLoadFromStorage = 2, storageOptimisation = 3, LoadIntoGraph = 4, finQuotesCheck = 5 };

    int             TickerID;
    std::string     sSign;
    int             iInterval;
    bool            bLoad;
    std::time_t     dtBegin;
    std::time_t     dtEnd;
    std::filesystem::path pathFileName;

    std::shared_ptr<std::vector<std::vector<BarTick>>> pvBars;
    dataFinQuotesParse parseData;

    TaskType taskType;

    std::shared_ptr<GraphHolder> holder;

    std::vector<std::pair<std::time_t,std::pair<std::time_t,std::vector<std::pair<std::time_t,std::time_t>>>>> vSessionTable;
    std::vector<std::pair<std::time_t,std::pair<std::time_t,std::vector<std::pair<std::time_t,std::time_t>>>>> vRepoTable;

    //--------------------------------------------------------------
    dataFinLoadTask():stStore{nullptr},parentWnd{nullptr},parseData{nullptr,nullptr},taskType{TaskType::finQuotesImport},holder{nullptr}{;};
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
        pvBars          = o.pvBars;

        taskType        = o.taskType;

        holder          = o.holder;

        vSessionTable   = o.vSessionTable;
        vRepoTable      = o.vRepoTable;
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
