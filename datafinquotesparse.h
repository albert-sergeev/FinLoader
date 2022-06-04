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

#ifndef DATAFINQUOTESPARSE_H
#define DATAFINQUOTESPARSE_H

#include<sstream>
#include<vector>
#include<chrono>


#include"bar.h"


class dataFinQuotesParse{

    std::istringstream * const issT;
    std::ostringstream * const ossE;
    int iColMax;
    int iDefaultInterval;
    std::string sSign;
    char cDelimiter;
    bool bHasHeader;

    std::vector<int>  vFieldsType;

public:

    enum fieldType:int {TICKER,PER,DATE,TIME,OPEN,HIGH,LOW,CLOSE,VOL,LAST};


    dataFinQuotesParse() = delete;
    dataFinQuotesParse(dataFinQuotesParse&) = delete;

    dataFinQuotesParse(std::istringstream * issTmp,std::ostringstream * ossErr):
        issT{issTmp},ossE{ossErr},iColMax{9},iDefaultInterval{-1},sSign{""}, bHasHeader{false}
    {
        vFieldsType.resize(9);
        t_tp.tm_isdst = 0;

    };

    dataFinQuotesParse& operator=(const dataFinQuotesParse &o){

        // issT and ossE don't copy purposely

        iColMax                 = o.iColMax;
        iDefaultInterval        = o.iDefaultInterval;
        sSign                   = o.sSign;
        cDelimiter              = o.cDelimiter;
        bHasHeader              = o.bHasHeader;

        vFieldsType.clear();
        std::copy(o.vFieldsType.begin(),o.vFieldsType.end(),std::back_inserter(vFieldsType));

        return *this;
    }



public:

    std::string     t_sWordBuff;
    std::string     t_sSign;
    int             t_iInterval;
    double          t_dTmp;
    std::string     t_sYear{"1990"};
    std::string     t_sMonth{"01"};
    std::string     t_sDay{"01"};
    std::string     t_sHour{"00"};
    std::string     t_sMin{"00"};
    std::string     t_sSec{"00"};
    std::tm         t_tp;
    int             t_iCurrN{0};


public:

    inline std::ostringstream & ossErr()       {return *ossE;};
    inline std::istringstream & issTmp()       {return *issT;};
    inline int ColMax()           const        {return  iColMax;};
    inline int DefaultInterval()  const        {return  iDefaultInterval;};
    inline std::string Sign()     const        {return  sSign;};
    inline std::vector<int> & fields()         { return vFieldsType;};
    inline char Delimiter()       const        {return  cDelimiter;};
    inline bool HasHeader()       const        {return  bHasHeader;}


    inline void setDefaultInterval(const int DefaultInterval)   {iDefaultInterval = DefaultInterval;};
    inline void setDefaultSign(const std::string Sign)          {sSign = Sign;};
    inline void setDelimiter(const char c)                      {cDelimiter = c;};
    inline void SetHeaderPresence(const bool b)                 {bHasHeader = b;}

    //----------------------
    bool initDefaultFieldsValues(int iCol){
        vFieldsType[0] = fieldType::TICKER;
        vFieldsType[1] = fieldType::PER;
        vFieldsType[2] = fieldType::DATE;
        vFieldsType[3] = fieldType::TIME;
        vFieldsType[4] = fieldType::OPEN;
        vFieldsType[5] = fieldType::HIGH;
        vFieldsType[6] = fieldType::LOW;
        vFieldsType[7] = fieldType::CLOSE;
        vFieldsType[8] = fieldType::VOL;

        if(iCol == 9 ){
            iColMax = iCol;
        }
        else if(iCol == 6 ){
            iColMax = iCol;
            vFieldsType[4] = fieldType::LAST;
            vFieldsType[5] = fieldType::VOL;
            iDefaultInterval = Bar::eInterval::pTick;
        }
        else{
            return false;
        }
        return  true;
    }
    //----------------------



};

#endif // DATAFINQUOTESPARSE_H
