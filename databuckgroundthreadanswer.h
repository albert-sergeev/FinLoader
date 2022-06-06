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

#ifndef DATABUCKGROUNDTHREADANSWER_H
#define DATABUCKGROUNDTHREADANSWER_H

#include<QWidget>

#include<memory>

#include "bar.h"
#include "bartick.h"

//////////////////////////////////////////////////////////////////////////////////////////////////
/// data container for activities of main type background processes


class dataBuckgroundThreadAnswer
{

public:
    enum eAnswerType:int {nop,
                          famImportBegin,famImportEnd,famImportCurrent,LoadActivity,TextInfoMessage
                          ,storagLoadFromStorageGraphBegin,storagLoadFromStorageGraphEnd, logText
                          ,storagLoadToGraphBegin,storagLoadToGraphEnd, logCriticalError, testPvBars
                          ,storageOptimisationBegin, storageOptimisationEnd,
                          cloneThread
                         };

    //-------------------------------------------------------------------------------
    inline eAnswerType  AnswerType() const          {return iAnswerType;};
    //
    inline bool Successfull() const                 {return bSuccessfull;}
    inline void SetSuccessfull(const bool bS)       {bSuccessfull = bS;};
    //
    inline std::string GetErrString() const         {return strErr;}
    inline void SetErrString(const std::string s)   {strErr = s;};
    //
    inline std::string GetTextInfo() const          {return strTextInfo;}
    inline void SetTextInfo(const std::string s)   {strTextInfo = s;};

    //
    inline void SetPercent(const int i)             {iLoadPercent = i;};
    inline int  Percent()   const                   {return iLoadPercent;};
    //
    //inline void SetTickerID(const int i)            {iTickerID = i;};
    inline int  TickerID()   const                  {return iTickerID;};
    //
    inline void SetParentWnd(QWidget * const wt)    { parentWnd = wt;};
    inline  QWidget *  GetParentWnd() const         {return parentWnd;}

    //
    inline void SetBeginDate(const std::time_t  t)  {dtBegin = t;};
    inline std::time_t  BeginDate()   const         {return dtBegin;};
    //
    inline void SetEndDate(const std::time_t  t)  {dtEnd = t;};
    inline std::time_t  EndDate()   const         {return dtEnd;};
    //

    //-------------------------------------------------------------------------------
    dataBuckgroundThreadAnswer(int TickerID = 0, eAnswerType iType = eAnswerType::nop, QWidget * parent = nullptr):iTickerID{TickerID},parentWnd{parent}{
        iAnswerType = iType;
        bSuccessfull = false;
    };
    //-------------------------------------------------------------------------------
    dataBuckgroundThreadAnswer(const dataBuckgroundThreadAnswer &o){

        iLoadPercent    = o.iLoadPercent;
        iAnswerType     = o.iAnswerType;
        bSuccessfull    = o.bSuccessfull;
        strErr          = o.strErr;
        strTextInfo     = o.strTextInfo;

        iTickerID       = o.iTickerID;

        parentWnd       = o.parentWnd;

        dtBegin         = o.dtBegin;
        dtEnd           = o.dtEnd;

        pvBars          = o.pvBars;
    };


private:
    int iLoadPercent{0};
    eAnswerType iAnswerType;
    bool bSuccessfull;
    std::string strErr;
    std::string strTextInfo;

    std::time_t dtBegin;
    std::time_t dtEnd;

    int iTickerID;

    QWidget * parentWnd;
public:
    std::shared_ptr<std::vector<std::vector<BarTick>>> pvBars;
};

#endif // DATABUCKGROUNDTHREADANSWER_H
