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

#ifndef MARKET_H
#define MARKET_H


#include<string>
#include<memory>
#include<QVariant>

#include "bar.h"

// Counter for primary index of market (iMarketID)
// !Warning! depend on area of visibility type declaration may demand redezign
// carefully use namspaces and multythread environment or you will have strange effectes
// and consistency of your data will be wasted!
static int iMarketCounter {1};

// call once flag for init constants
inline std::once_flag market_call_once_flag;

//////////////////////////////////////////////
/// \brief The Market class. Storing data for trading floor of the securities market
/// Store data for table of markets
/// provides an interface for get/set properties and for use in container classes
/// uses iMarketID as an uniquie primary index
/// sMarketSign & sMarketName better never be null, but not necessarily
/// Ticker class refer to iMarketID by its member
///
/// provides an interface to check trade data for hitting in the trading periods or repo periods
/// for use in incoming quotes processing algorithms
///
class Market
{
public:
    typedef  std::vector<std::pair<std::time_t,std::pair<std::time_t,std::vector<std::pair<std::time_t,std::time_t>>>>> SessionTable_type;

private:

    // main data

    std::string sMarketName;
    std::string sMarketSign;
    int         iMarketID;

    // utility data
    bool        bAutoLoad;
    bool        bUpToSys;
    std::time_t tStartTime;
    std::time_t tEndTime;

    SessionTable_type vSessionTable;
    SessionTable_type vRepoTable;


    static std::time_t t1971_01_01_00_00_00;

public:
    //--------------------------------------------------------------------------------------------------------
    // get/set property-like interface

    inline std::string MarketName() const {return sMarketName;};
    inline std::string MarketSign() const {return sMarketSign;};
    inline int         MarketID()   const {return iMarketID;};
    inline bool        AutoLoad()   const {return bAutoLoad;};
    inline bool        UpToSys()    const {return bUpToSys;};
    inline std::time_t StartTime()  const {return tStartTime;};
    inline std::time_t EndTime()    const {return tEndTime;};

    inline void SetMarketName   (const std::string  MarketName) {sMarketName    = MarketName;};
    inline void SetMarketSign   (const std::string  MarketSign) {sMarketSign    = MarketSign;};
    inline void SetAutoLoad     (const bool         AutoLoad)   {bAutoLoad      = AutoLoad;};
    inline void SetUpToSys      (const bool         UpToSys)    {bUpToSys       = UpToSys;};
    inline void SetStartTime    (const std::time_t  StartTime)  {tStartTime     = StartTime;};
    inline void SetEndTime      (const std::time_t  EndTime)    {tEndTime       = EndTime;};

    const SessionTable_type& SessionTable() const {return vSessionTable;};
    const SessionTable_type& RepoTable()    const {return vRepoTable;};

    inline void setSessionTable(const SessionTable_type &table) {vSessionTable = table;};
    inline void setRepoTable(const SessionTable_type &table)    {vRepoTable = table;};



public:
    //--------------------------------------------------------------------------------------------------------
    // interface for use in container classes

    //--------------------------------------------------------------------------------------------------------
    // use only explicit constructor.
    Market() = delete ;
    //--------------------------------------------------------------------------------------------------------
    Market(std::string MarketName,std::string MarketSign, int MarketID)
    {
        sMarketName = MarketName;
        sMarketSign = MarketSign;
        iMarketID   = MarketID;

        std::tm tmPer;
        {
            tmPer.tm_year   = 1971 - 1900;
            tmPer.tm_mon    = 1 - 1;
            tmPer.tm_mday   = 1;
            tmPer.tm_hour   = 0;
            tmPer.tm_min    = 0;
            tmPer.tm_sec    = 0;
            tmPer.tm_isdst  = 0;
        }

        bAutoLoad = true;
        bUpToSys = false;
        //
        {
            tmPer.tm_hour   = 9;
            tmPer.tm_min    = 30;
        }
        tStartTime = mktime_gm(&tmPer);
        //
        {
            tmPer.tm_hour   = 19;
            tmPer.tm_min    = 0;
        }
        tEndTime = mktime_gm(&tmPer);
        //
        vSessionTable = buildDefaultSessionsTable();
        vRepoTable = buildDefaultRepoTable();


        //
        if (MarketID >= iMarketCounter) iMarketCounter = MarketID + 1;
    }
    //
    Market(std::string MarketName,std::string MarketSign):Market(MarketName,MarketSign, iMarketCounter++){};
    //--------------------------------------------------------------------------------------------------------
    Market(const Market &o)
    {
        sMarketName = o.sMarketName;
        sMarketSign = o.sMarketSign;
        iMarketID   = o.iMarketID;

        // utility data
        bAutoLoad   = o.bAutoLoad;
        bUpToSys    = o.bUpToSys;
        tStartTime  = o.tStartTime;
        tEndTime    = o.tEndTime;

        vSessionTable   = o.vSessionTable;
        vRepoTable      = o.vRepoTable;
    }
    //--------------------------------------------------------------------------------------------------------
    bool equal(const Market &o)
    {
        if (
            sMarketName == o.sMarketName &&
            sMarketSign == o.sMarketSign &&
            iMarketID   == o.iMarketID &&
            bAutoLoad   == o.bAutoLoad &&
            bUpToSys    == o.bUpToSys &&
            tStartTime  == o.tStartTime &&
            tEndTime    == o.tEndTime){
            // SessionTable comparison not needed:
            // vSessionTable   = o.vSessionTable;
            // vRepoTable      = o.vRepoTable;
            return true;
        }
        else{
            return false;
        }
    }
    // interface for use in container classes end
    //--------------------------------------------------------------------------------------------------------

public:
    //--------------------------------------------------------------------------------------------------------
    // interface to check trade data for hitting in the trading periods or repo periods

    //--------------------------------------------------------------------------------------------------------
    /// \brief buildDefaultSessionsTable build standart trade session time table for MOEX
    /// \return session table in usable format
    ///
    static SessionTable_type buildDefaultSessionsTable()
    {

        std::tm tmPer;
        {
            tmPer.tm_year   = 1971 - 1900;
            tmPer.tm_mon    = 1 - 1;
            tmPer.tm_mday   = 1;
            tmPer.tm_hour   = 0;
            tmPer.tm_min    = 0;
            tmPer.tm_sec    = 0;
            tmPer.tm_isdst  = 0;
        }


        std::time_t t   = mktime_gm(&tmPer);

        tmPer.tm_hour   = 10;
        tmPer.tm_min    = 0;
        tmPer.tm_sec    = 0;
        std::time_t t1_1   = mktime_gm(&tmPer);

//        tmPer.tm_hour   = 18;
//        tmPer.tm_min    = 39;
        tmPer.tm_hour   = 23;
        tmPer.tm_min    = 49;
        tmPer.tm_sec    = 59;
        std::time_t t1_2   = mktime_gm(&tmPer);

//        tmPer.tm_hour   = 19;
//        tmPer.tm_min    = 05;
//        tmPer.tm_sec    = 0;
//        std::time_t t2_1   = mktime_gm(&tmPer);

//        tmPer.tm_hour   = 23;
//        tmPer.tm_min    = 49;
//        tmPer.tm_sec    = 59;
//        std::time_t t2_2   = mktime_gm(&tmPer);


        tmPer.tm_year   = 2100 - 1900;
        tmPer.tm_hour   = 0;
        tmPer.tm_min    = 0;
        tmPer.tm_sec    = 0;
        tmPer.tm_isdst  = 0;

        std::time_t tE   = mktime_gm(&tmPer);


        //SessionTable_type v{{t,{tE,{{t1_1,t1_2},{t2_1,t2_2}}}}};
        SessionTable_type v{{t,{tE,{{t1_1,t1_2}/*,{t2_1,t2_2}*/}}}};

        return v;
    };
    //--------------------------------------------------------------------------------------------------------
    /// \brief buildDefaultSessionsTable build standart repo session time table for MOEX
    /// \return session table in usable format
    static SessionTable_type buildDefaultRepoTable()
    {

        std::tm tmPer;
        {
            tmPer.tm_year   = 1971 - 1900;
            tmPer.tm_mon    = 1 - 1;
            tmPer.tm_mday   = 1;
            tmPer.tm_hour   = 9;
            tmPer.tm_min    = 59;
            tmPer.tm_sec    = 0;
            tmPer.tm_isdst  = 0;
        }


        std::time_t t   = mktime_gm(&tmPer);

        tmPer.tm_hour   = 9;
        tmPer.tm_min    = 59;
        tmPer.tm_sec    = 0;
        std::time_t t1_1   = mktime_gm(&tmPer);

        tmPer.tm_hour   = 9;
        tmPer.tm_min    = 59;
        tmPer.tm_sec    = 59;
        std::time_t t1_2   = mktime_gm(&tmPer);

        tmPer.tm_hour   = 18;
        tmPer.tm_min    = 40;
        tmPer.tm_sec    = 0;
        std::time_t t2_1   = mktime_gm(&tmPer);

        tmPer.tm_hour   = 19;
        tmPer.tm_min    = 04;
        tmPer.tm_sec    = 59;
        std::time_t t2_2   = mktime_gm(&tmPer);


        tmPer.tm_year   = 2100 - 1900;
        tmPer.tm_hour   = 0;
        tmPer.tm_min    = 0;
        tmPer.tm_sec    = 0;
        tmPer.tm_isdst  = 0;

        std::time_t tE   = mktime_gm(&tmPer);

        SessionTable_type v{{t,{tE,{{t1_1,t1_2},{t2_1,t2_2}}}}};
        //SessionTable_type v{{t,{tE,{{t1_1,t1_2}}}}};
        //SessionTable_type v{{t,{tE,{{t2_1,t2_2}}}}};
        return v;
    };

    //--------------------------------------------------------------------------------------------------------
    // check if desired time hits session table
    // works fast enough to use in trade quotes processing algoritms (but would like to faster of course:))
    static bool IsInSessionTabe(SessionTable_type &v, std::time_t t)
    {
        size_t iFirst = LeftIndex(v,t);

        if(iFirst < v.size()){
            if(v[iFirst].second.first >= t){

                std::time_t timeAcc = AccomodateToTime(t);

                size_t iSecond = LeftIndex(v[iFirst].second.second,timeAcc);

                if(iSecond < v[iFirst].second.second.size()){
                    if(/*v[iFirst].second.second[iSecond].first  <= timeAcc &&*/
                       v[iFirst].second.second[iSecond].second >= timeAcc ){
                        return true;
                    }
                }
            }
        }
        return false;
    };
    //--------------------------------------------------------------------------------------------------------
    // flattens the time to one day 1971_01_01, so we can simply compare times, without call heavy functions.
    // requires time to be GMT
    static std::time_t AccomodateToTime(std::time_t t){

        std::call_once(market_call_once_flag,initStartConst);

        t = ((t - t1971_01_01_00_00_00) % 86400) + t1971_01_01_00_00_00;
        return t;
    }
    //--------------------------------------------------------------------------------------------------------
    // utility to print session table content to standart output
    static void coutSessionTable(const SessionTable_type &tbl){
        ThreadFreeCout pcout;
        pcout <<"SessionTable:\n";

        for(const auto &e:tbl){
            pcout <<"{"<<threadfree_gmtime_to_str(&e.first)<<" : "<<threadfree_gmtime_to_str(&e.second.first)<<"}\n";
            for(const auto &v:e.second.second){
                pcout <<"\t{"<<threadfree_gmtime_to_str(&v.first)<<" : "<<threadfree_gmtime_to_str(&v.second)<<"}\n";
            }
        }
    }
protected:
    //--------------------------------------------------------------------------------------------------------
    /// initialize static constant, that use for fast time convertion
    static void initStartConst(){

        std::tm tmPer;
        {
            tmPer.tm_year   = 1971 - 1900;
            tmPer.tm_mon    = 1 - 1;
            tmPer.tm_mday   = 1;
            tmPer.tm_hour   = 0;
            tmPer.tm_min    = 0;
            tmPer.tm_sec    = 0;
            tmPer.tm_isdst  = 0;
        }
        t1971_01_01_00_00_00 = mktime_gm(&tmPer);
    };

    //--------------------------------------------------------------------------------------------------------
    /// utility func for fast find index of the range(i.e.period) in which the desired time falls
    /// requires T to be an increment sorted container of std::vector<std::pair<std::time_t,some_type>>
    template<typename T>
    static size_t LeftIndex(T v,std::time_t t){

        size_t left = 0;
        size_t right = v.size();

        size_t middle = left + (right-left)/2;
        while(left < right){
            middle = left + (right-left)/2;

            if(v[middle].first > t){
                right = middle;
            }
            else if(v[middle].first < t){
                left = middle + 1;
            }
            else{
                return middle;
            }
        }
        return right > 0 ? right - 1 : v.size();
    };

    //--------------------------------------------------------------------------------------------------------
};

inline std::time_t Market::t1971_01_01_00_00_00;

#endif // MARKET_H
