#ifndef Ticker_H
#define Ticker_H

#include<string>
#include<memory>
#include<QVariant>


//REDO: warning. in multithread redo to atomic tipe.
static int iTickerCounter {1};
static int iMarketCounter {1};



///////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Class for store info about Markets, worktime, auto its load parameters
///
/// ///////////////////////////////////////////////////////////////////////////////////////////////////////////

class Market
{
private:

    std::string sMarketName;
    std::string sMarketSign;
    int         iMarketID;

    bool        bAutoLoad;
    bool        bUpToSys;
    std::time_t tStartTime;
    std::time_t tEndTime;

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


public:
    //--------------------------------------------------------------------------------------------------------
    // use only explicit constructor. Copy constructor by default is acceptable;
    Market() = delete ;
    //--------------------------------------------------------------------------------------------------------
    Market(std::string MarketName,std::string MarketSign, int MarketID)
    {
        sMarketName = MarketName;
        sMarketSign = MarketSign;
        iMarketID   = MarketID;

        std::tm tmPer;
        {
            tmPer.tm_year   = 2000 - 1900;
            tmPer.tm_mon    = 1;
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
        tStartTime = std::mktime(&tmPer);
        //
        {
            tmPer.tm_hour   = 19;
            tmPer.tm_min    = 0;
        }
        tEndTime = std::mktime(&tmPer);
        //
        if (MarketID >= iMarketCounter) iMarketCounter = MarketID + 1;
    }
    //
    Market(std::string MarketName,std::string MarketSign):Market(MarketName,MarketSign, iMarketCounter++){};
    //--------------------------------------------------------------------------------------------------------
//    Market(const Market &m)
//    {
//        sMarketName = m.sMarketName;
//        sMarketSign = m.sMarketSign;
//        iMarketID   = m.iMarketID;
//    }
    //--------------------------------------------------------------------------------------------------------

};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Class for store paper data
///
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Ticker
{
private:

    std::string             sTickerName;
    std::string             sTickerSign;

    std::string             sTickerSignFinam;
    std::string             sTickerSignQuik;

    int                     iTickerID;
    int                     iMarketID;

    bool                    bAutoLoad;
    bool                    bUpToSys;

    bool                    bBulbululator;
    bool                    bBuble;


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

    inline void SetTickerName           (const std::string  TickerName)     {sTickerName            = TickerName;};
    inline void SetTickerSign           (const std::string  TickerSign)     {sTickerSign            = TickerSign;};
    inline void SetTickerSignFinam      (const std::string  TickerSign)     {sTickerSignFinam       = TickerSign;};
    inline void SetTickerSignQuik       (const std::string  TickerSign)     {sTickerSignQuik        = TickerSign;};
    inline void SetAutoLoad             (const bool         AutoLoad)       {bAutoLoad              = AutoLoad;};
    inline void SetUpToSys              (const bool         UpToSys)        {bUpToSys               = UpToSys;};
    inline void SetBulbululator         (const bool         Bulbululator)   {bBulbululator          = Bulbululator;};
    inline void SetBuble                (const bool         Buble)          {bBuble                 = Buble;};

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
