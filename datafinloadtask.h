/****************************************************************************
*  This is part of FinLoader
*  Copyright (C) 2021  Albert Sergeyev
*  Contact: albert.s.sergeev@mail.ru
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <https://www.gnu.org/licenses/>.
****************************************************************************/

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

    enum TaskType:int { finQuotesImport = 1, finQuotesLoadFromStorage = 2, storageOptimisation = 3, LoadIntoGraph = 4, finQuotesCheck = 5};

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

    Market::SessionTable_type vSessionTable;
    Market::SessionTable_type  vRepoTable;

    bool bCheckInMemory;

    //--------------------------------------------------------------
    dataFinLoadTask():
        stStore{nullptr},
        parentWnd{nullptr},
        parseData{nullptr,nullptr},
        taskType{TaskType::finQuotesImport},
        holder{nullptr},
        bCheckInMemory{false}
    {;};
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

        bCheckInMemory  = o.bCheckInMemory;
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
