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
    int             TickerID;
    std::string     sSign;
    int             iInterval;
    bool            bLoad;
    std::time_t     dtBegin;
    std::time_t     dtEnd;
    std::filesystem::path pathFileName;



    //--------------------------------------------------------------
    dataFinLoadTask():stStore{nullptr},parentWnd{nullptr}{;};
    //--------------------------------------------------------------
    dataFinLoadTask(const dataFinLoadTask & o){

        parentWnd       = o.parentWnd;
        stStore         = o.stStore;

        TickerID        = o.TickerID;
        sSign           = o.sSign;
        iInterval       = o.iInterval;
        bLoad           = o.bLoad;
        dtBegin         = o.dtBegin;
        dtEnd           = o.dtEnd;
        pathFileName    = o.pathFileName;
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
