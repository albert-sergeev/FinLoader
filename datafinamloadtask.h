#ifndef DATAFINAMLOADTASK_H
#define DATAFINAMLOADTASK_H

#include <string>
#include <chrono>
#include<filesystem>
#include<QWidget>

#include "storage.h"
#include "blockfreequeue.h"


class dataFinamLoadTask
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
    dataFinamLoadTask():stStore{nullptr},parentWnd{nullptr}{;};
    //--------------------------------------------------------------
    dataFinamLoadTask(const dataFinamLoadTask & o){

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
    dataFinamLoadTask& operator=(dataFinamLoadTask&) = delete;
};

#endif // DATAFINAMLOADTASK_H
