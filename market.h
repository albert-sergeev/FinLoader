#ifndef MARKET_H
#define MARKET_H


#include<string>
#include<memory>
#include<QVariant>

#include "bar.h"


//REDO: warning. in multithread redo to atomic tipe.
static int iMarketCounter {1};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Class for store info about Markets, worktime, auto its load parameters
///
/// ///////////////////////////////////////////////////////////////////////////////////////////////////////////
///
inline std::once_flag market_call_once_flag;

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


    static std::time_t t1990_01_01_00_00_00;

public:



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

    SessionTable_type SessionTable()  {return vSessionTable;};
    SessionTable_type RepoTable()     {return vRepoTable;};

public:
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
            tmPer.tm_year   = 1990 - 1900;
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
// TODO: SessionTable comparison
//            vSessionTable   = o.vSessionTable;
//            vRepoTable      = o.vRepoTable;
            return true;
        }
        else{
            return false;
        }
    }
    //--------------------------------------------------------------------------------------------------------
    static SessionTable_type buildDefaultSessionsTable()
    {

        std::tm tmPer;
        {
            tmPer.tm_year   = 1990 - 1900;
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
    static SessionTable_type buildDefaultRepoTable()
    {

        std::tm tmPer;
        {
            tmPer.tm_year   = 1990 - 1900;
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
protected:

    static void initStartConst(){

        std::tm tmPer;
        {
            tmPer.tm_year   = 1990 - 1900;
            tmPer.tm_mon    = 1 - 1;
            tmPer.tm_mday   = 1;
            tmPer.tm_hour   = 0;
            tmPer.tm_min    = 0;
            tmPer.tm_sec    = 0;
            tmPer.tm_isdst  = 0;
        }
        t1990_01_01_00_00_00 = mktime_gm(&tmPer);
    };
    //--------------------------------------------------------------------------------------------------------
    static std::time_t AccomodateToTime(std::time_t t){

        std::call_once(market_call_once_flag,initStartConst);

        t = ((t - t1990_01_01_00_00_00) % 86400) + t1990_01_01_00_00_00;
        return t;
    }
    //--------------------------------------------------------------------------------------------------------

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

inline std::time_t Market::t1990_01_01_00_00_00;

#endif // MARKET_H
