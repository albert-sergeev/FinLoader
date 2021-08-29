#ifndef Ticker_H
#define Ticker_H

#include<string>
#include<memory>
#include<QVariant>

#include "bar.h"
#include "market.h"


//REDO: warning. in multithread redo to atomic tipe.
static int iTickerCounter {1};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Class for store paper data
///
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
    double                          dHScale;
    double                          dVVolumeScale;
    std::time_t                     tViewBegin;
    std::time_t                     tViewEnd;
    std::map<int,double>            mVScalse;


public:

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

    inline Bar::eInterval                  StoredSelectedInterval()     const   {return iStoredSelectedInterval;};
    inline bool                            OHLC()                       const   {return bOHLC;};
    inline std::time_t                     StoredTimePosition()         const   {return tStoredTimePosition;};
    inline double                          HScale()                     const   {return dHScale;};
    inline double                          VVolumeScale()               const   {return dVVolumeScale;};
    inline std::time_t                     ViewBeginDate()              const   {return tViewBegin;};
    inline std::time_t                     ViewEndDate()                const   {return tViewEnd;};
    inline double                          VScalse(int Interval)        const   {return ( mVScalse.at(Interval)); };

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
    inline void SetHScale                   (const double Scale)                      {dHScale = Scale;};
    inline void SetVVolumeScale             (const double VolumeScale)                {dVVolumeScale = VolumeScale;};
    inline void SetViewBegin                (const std::time_t Begin)                 {tViewBegin = Begin;};
    inline void SetViewEnd                  (const std::time_t End)                   {tViewEnd = End;};
    inline void SetVScalse                  (const std::map<int,double>&m)            {mVScalse = m; };


public:
    //--------------------------------------------------------------------------------------------------------
    // use only explicit constructor. Copy constructor by default is acceptable;
    Ticker() = delete ;
    //--------------------------------------------------------------------------------------------------------
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

        iStoredSelectedInterval = Bar::p60;
        bOHLC                   = false;
        tStoredTimePosition     = 0;
        dHScale                 = 0;
        dVVolumeScale           = 0;
        tViewBegin              = 0;
        tViewEnd                = 0;

        mVScalse[Bar::eInterval::pTick] = 0;
        mVScalse[Bar::eInterval::p1]    = 0;
        mVScalse[Bar::eInterval::p5]    = 0;
        mVScalse[Bar::eInterval::p10]   = 0;
        mVScalse[Bar::eInterval::p15]   = 0;
        mVScalse[Bar::eInterval::p30]   = 0;
        mVScalse[Bar::eInterval::p60]   = 0;
        mVScalse[Bar::eInterval::p120]  = 0;
        mVScalse[Bar::eInterval::p180]  = 0;
        mVScalse[Bar::eInterval::pDay]  = 0;
        mVScalse[Bar::eInterval::pWeek] = 0;
        mVScalse[Bar::eInterval::pMonth]= 0;


        //
        if (TickerID >= iTickerCounter) iTickerCounter = TickerID + 1;
    }
    //
    Ticker(std::string TickerName,std::string TickerSign, int MarketID):Ticker(iTickerCounter++,TickerName,TickerSign, MarketID){};
    //--------------------------------------------------------------------------------------------------------
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
        dHScale                 = t.dHScale;
        dVVolumeScale           = t.dVVolumeScale;
        tViewBegin              = t.tViewBegin;
        tViewEnd                = t.tViewEnd;
        mVScalse                = t.mVScalse;
    }
    //--------------------------------------------------------------------------------------------------------
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
        dHScale                 = t.dHScale;
        dVVolumeScale           = t.dVVolumeScale;
        tViewBegin              = t.tViewBegin;
        tViewEnd                = t.tViewEnd;
        mVScalse                = t.mVScalse;

        return  *this;
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
