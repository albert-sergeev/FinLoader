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

#ifndef Ticker_H
#define Ticker_H

#include<string>
#include<memory>

#include "bar.h"
#include "market.h"


// Counter for primary index of ticker (iTickerID)
// !Warning! depend on area of visibility type declaration may demand redezign
// carefully use namspaces and multythread environment or you will have strange effectes
// and consistency of your data will be wasted!
static int iTickerCounter {1};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Class for storing security paper(i.e. ticker's) data
/// Store data for table of tickers
/// provides an interface for get/set properties and for use in container classes
/// uses iTickerID as uniquie primary index
/// refers to Marker::MarketID by MarketID member
/// sTickerSign must never be null
///
class Ticker
{
private:

    // main data

    std::string             sTickerName;
    std::string             sTickerSign;

    std::string             sTickerSignFinam;
    std::string             sTickerSignQuik;

    int                     iTickerID;
    int                     iMarketID;

    // utility parameters

    bool                    bAutoLoad;
    bool                    bUpToSys;

    bool                    bBulbululator;
    bool                    bBuble;

    // viewport cast parameters
    Bar::eInterval                  iStoredSelectedInterval;
    bool                            bOHLC;
    std::time_t                     tStoredTimePosition;
    int                             iStoredRightAggregate;
    double                          dStoredVValue;
    double                          dHScale;

    std::time_t                     tViewBegin;
    std::time_t                     tViewEnd;
    std::map<int,double>            mVScale;
    std::map<int,double>            mVVolumeScale;


public:
    //--------------------------------------------------------------------------------------------------------
    /// property-like get/set interface:

    inline std::string              TickerName()        const   {return sTickerName;};
    inline std::string              TickerSign()        const   {return sTickerSign;};
    inline std::string              TickerSignFinam()   const   {return sTickerSignFinam;};
    inline std::string              TickerSignQuik()    const   {return sTickerSignQuik;};
    inline int                      TickerID()          const   {return iTickerID;};
    inline int                      MarketID()          const   {return iMarketID;};
    inline bool                     AutoLoad()          const   {return bAutoLoad;};
    inline bool                     UpToSys()           const   {return bUpToSys;};
    inline bool                     Bulbululator()      const   {return bBulbululator;};
    inline bool                     Buble()             const   {return bBuble;};

    inline Bar::eInterval                  StoredSelectedInterval()         const   {return iStoredSelectedInterval;};
    inline bool                            OHLC()                           const   {return bOHLC;};
    inline std::time_t                     StoredTimePosition()             const   {return tStoredTimePosition;};
    inline int                             StoredRightAggregate()           const   {return iStoredRightAggregate;}
    inline double                          StoredVValue()                   const   {return dStoredVValue;};
    inline double                          StoredHScale()                   const   {return dHScale;};
    inline std::time_t                     StoredViewBeginDate()            const   {return tViewBegin;};
    inline std::time_t                     StoredViewEndDate()              const   {return tViewEnd;};
    inline double                          StoredVScale(int Interval)       const   {return mVScale.find(Interval) == mVScale.end() ? 0 : mVScale.at(Interval); };
    inline double                          StoredVVolumeScale(int Interval) const   {return mVVolumeScale.find(Interval) == mVVolumeScale.end() ? 0 : mVVolumeScale.at(Interval); };

    inline void SetTickerName           (const std::string  TickerName)     {sTickerName            = TickerName;};
    inline void SetTickerSign           (const std::string  TickerSign)     {sTickerSign            = TickerSign;};
    inline void SetTickerSignFinam      (const std::string  TickerSign)     {sTickerSignFinam       = TickerSign;};
    inline void SetTickerSignQuik       (const std::string  TickerSign)     {sTickerSignQuik        = TickerSign;};
    inline void SetAutoLoad             (const bool         AutoLoad)       {bAutoLoad              = AutoLoad;};
    inline void SetUpToSys              (const bool         UpToSys)        {bUpToSys               = UpToSys;};
    inline void SetBulbululator         (const bool         Bulbululator)   {bBulbululator          = Bulbululator;};
    inline void SetBuble                (const bool         Buble)          {bBuble                 = Buble;};


    inline void SetStoredSelectedInterval   (const Bar::eInterval Interval)           {iStoredSelectedInterval = Interval;};
    inline void SetOHLC                     (const bool OHLC)                         {bOHLC = OHLC;};
    inline void SetStoredTimePosition       (const std::time_t t)                     {tStoredTimePosition = t;};
    inline void SetStoredRightAggregate     (const int RightAggregate)                {iStoredRightAggregate = RightAggregate;};
    inline void SetStoredHValue             (const double HValue)                     {dStoredVValue = HValue;};
    inline void SetHScale                   (const double Scale)                      {dHScale = Scale;};
    inline void SetViewBegin                (const std::time_t Begin)                 {tViewBegin = Begin;};
    inline void SetViewEnd                  (const std::time_t End)                   {tViewEnd = End;};
    inline void SetVScale                   (const std::map<int,double>&m)            {mVScale = m; };
    inline void SetVVolumeScale             (const std::map<int,double>&m)            {mVVolumeScale = m; };

    //--------------------------------------------------------------------------------------------------------

public:
    //--------------------------------------------------------------------------------------------------------
    /// constructors and procedures for use in container classes:

    //--------------------------------------------------------------------------------------------------------
    // use only explicit constructors.
    Ticker() = delete ;
    //--------------------------------------------------------------------------------------------------------
    // base used constructor
    Ticker(int TickerID, std::string TickerName,std::string TickerSign, int MarketID)
    {
        if(MarketID <= 0){
            throw std::invalid_argument("Invalid null pointer to market in Ticker()");
        }
        //
        sTickerName = TickerName;
        sTickerSign = TickerSign;
        iTickerID   = TickerID;
        iMarketID = MarketID;

        sTickerSignFinam = TickerSign;
        sTickerSignQuik = TickerSign;

        bAutoLoad       = true;
        bUpToSys        = false;

        bBulbululator   = true;
        bBuble          = true;

        //iStoredSelectedInterval = Bar::p60;
        iStoredSelectedInterval = Bar::p1;
        bOHLC                   = false;
        tStoredTimePosition     = 0;
        iStoredRightAggregate   = 0;
        dStoredVValue           = 0;
        dHScale                 = 0;
        tViewBegin              = 0;
        tViewEnd                = 0;

        mVScale[Bar::eInterval::pTick] = 0;
        mVScale[Bar::eInterval::p1]    = 0;
        mVScale[Bar::eInterval::p5]    = 0;
        mVScale[Bar::eInterval::p10]   = 0;
        mVScale[Bar::eInterval::p15]   = 0;
        mVScale[Bar::eInterval::p30]   = 0;
        mVScale[Bar::eInterval::p60]   = 0;
        mVScale[Bar::eInterval::p120]  = 0;
        mVScale[Bar::eInterval::p180]  = 0;
        mVScale[Bar::eInterval::pDay]  = 0;
        mVScale[Bar::eInterval::pWeek] = 0;
        mVScale[Bar::eInterval::pMonth]= 0;

        mVVolumeScale[Bar::eInterval::pTick] = 0;
        mVVolumeScale[Bar::eInterval::p1]    = 0;
        mVVolumeScale[Bar::eInterval::p5]    = 0;
        mVVolumeScale[Bar::eInterval::p10]   = 0;
        mVVolumeScale[Bar::eInterval::p15]   = 0;
        mVVolumeScale[Bar::eInterval::p30]   = 0;
        mVVolumeScale[Bar::eInterval::p60]   = 0;
        mVVolumeScale[Bar::eInterval::p120]  = 0;
        mVVolumeScale[Bar::eInterval::p180]  = 0;
        mVVolumeScale[Bar::eInterval::pDay]  = 0;
        mVVolumeScale[Bar::eInterval::pWeek] = 0;
        mVVolumeScale[Bar::eInterval::pMonth]= 0;


        //
        if (TickerID >= iTickerCounter) iTickerCounter = TickerID + 1;
    }
    //
    Ticker(std::string TickerName,std::string TickerSign, int MarketID):Ticker(iTickerCounter++,TickerName,TickerSign, MarketID){};
    //--------------------------------------------------------------------------------------------------------
    // copy constructor
    Ticker (const Ticker & t){
        sTickerName = t.sTickerName;
        sTickerSign = t.sTickerSign;
        iTickerID   = t.iTickerID;
        iMarketID = t.iMarketID;

        sTickerSignFinam = t.sTickerSignFinam;
        sTickerSignQuik = t.sTickerSignQuik;

        bAutoLoad = t.bAutoLoad;
        bUpToSys = t.bUpToSys;

        bBulbululator   = t.bBulbululator;
        bBuble          = t.bBuble;

        iStoredSelectedInterval = t.iStoredSelectedInterval;
        bOHLC                   = t.bOHLC;
        tStoredTimePosition     = t.tStoredTimePosition;
        iStoredRightAggregate   = t.iStoredRightAggregate;
        dStoredVValue           = t.dStoredVValue;
        dHScale                 = t.dHScale;
        tViewBegin              = t.tViewBegin;
        tViewEnd                = t.tViewEnd;
        mVScale                 = t.mVScale;
        mVVolumeScale           = t.mVVolumeScale;
    }
    //--------------------------------------------------------------------------------------------------------
    // copy constructor
    Ticker & operator= (const Ticker & t){
        sTickerName = t.sTickerName;
        sTickerSign = t.sTickerSign;
        iTickerID   = t.iTickerID;
        iMarketID = t.iMarketID;

        sTickerSignFinam = t.sTickerSignFinam;
        sTickerSignQuik = t.sTickerSignQuik;

        bAutoLoad = t.bAutoLoad;
        bUpToSys = t.bUpToSys;

        bBulbululator   = t.bBulbululator;
        bBuble          = t.bBuble;

        iStoredSelectedInterval = t.iStoredSelectedInterval;
        bOHLC                   = t.bOHLC;
        tStoredTimePosition     = t.tStoredTimePosition;
        iStoredRightAggregate   = t.iStoredRightAggregate;
        dStoredVValue           = t.dStoredVValue;
        dHScale                 = t.dHScale;
        tViewBegin              = t.tViewBegin;
        tViewEnd                = t.tViewEnd;
        mVScale                 = t.mVScale;
        mVVolumeScale           = t.mVVolumeScale;

        return  *this;
    }
    //--------------------------------------------------------------------------------------------------------
    // comparison used to store ticker changes to database. That is in order not to waste resources for not impotant updates
    bool equal (const Ticker & t){
        if (sTickerName == t.sTickerName &&
            sTickerSign == t.sTickerSign &&
            iTickerID   == t.iTickerID &&
            iMarketID   == t.iMarketID &&
            sTickerSignFinam == t.sTickerSignFinam &&
            sTickerSignQuik == t.sTickerSignQuik &&
            bAutoLoad == t.bAutoLoad &&
            bUpToSys == t.bUpToSys &&
            bBulbululator   == t.bBulbululator &&
            bBuble          == t.bBuble &&
            iStoredSelectedInterval == t.iStoredSelectedInterval &&
            bOHLC                   == t.bOHLC //&&
            //tStoredTimePosition     == t.tStoredTimePosition &&
            //dStoredHValue           == t.dStoredHValue &&
            //dHScale                 == t.dHScale &&
            //dVVolumeScale           == t.dVVolumeScale &&
            //tViewBegin              == t.tViewBegin &&
            //tViewEnd                == t.tViewEnd
                ){
//                auto It (mVScalse.begin());
//                auto ItT (t.mVScalse.begin());
//                while(It != mVScalse.end() && ItT != t.mVScalse.end()){
//                    if (*It != *ItT){
//                        return  false;
//                    }
//                    It++;
//                    ItT++;
//                }
//                if(It != mVScalse.end() || ItT != t.mVScalse.end()){
//                    return false;
//                }
            return true;
        }
        else{
            return false;
        }

    }
    //--------------------------------------------------------------------------------------------------------
    // full data comparison
    bool equalFull (const Ticker & t){
        if (sTickerName == t.sTickerName &&
            sTickerSign == t.sTickerSign &&
            iTickerID   == t.iTickerID &&
            iMarketID   == t.iMarketID &&
            sTickerSignFinam == t.sTickerSignFinam &&
            sTickerSignQuik == t.sTickerSignQuik &&
            bAutoLoad == t.bAutoLoad &&
            bUpToSys == t.bUpToSys &&
            bBulbululator   == t.bBulbululator &&
            bBuble          == t.bBuble &&
            iStoredSelectedInterval == t.iStoredSelectedInterval &&
            bOHLC                   == t.bOHLC &&
            tStoredTimePosition     == t.tStoredTimePosition &&
            iStoredRightAggregate   == t.iStoredRightAggregate &&
            dStoredVValue           == t.dStoredVValue &&
            dHScale                 == t.dHScale //&&
            //tViewBegin              == t.tViewBegin &&
            //tViewEnd                == t.tViewEnd
                ){

            {
                auto It (mVScale.begin());
                auto ItT (t.mVScale.begin());
                while(It != mVScale.end() && ItT != t.mVScale.end()){
                    if (*It != *ItT){
                        return  false;
                    }
                    It++;
                    ItT++;
                }
                if(It != mVScale.end() || ItT != t.mVScale.end()){
                    return false;
                }
            }
            {
                auto It (mVVolumeScale.begin());
                auto ItT (t.mVVolumeScale.begin());
                while(It != mVVolumeScale.end() && ItT != t.mVVolumeScale.end()){
                    if (*It != *ItT){
                        return  false;
                    }
                    It++;
                    ItT++;
                }
                if(It != mVVolumeScale.end() || ItT != t.mVVolumeScale.end()){
                    return false;
                }
            }
            return true;
        }
        else{
            return false;
        }

    }
    //--------------------------------------------------------------------------------------------------------
};

//std::ostream & operator<<(std::ostream &os, Ticker &t)
//{

//    os << t.MarketID() << ",";
//    os << t.TickerID() << ",";
//    os << t.TickerName() << ",";
//    os << t.TickerSign() << ",";

//    return  os;
//}





#endif // Ticker_H
