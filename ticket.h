#ifndef TICKET_H
#define TICKET_H

#include<string>
#include<memory>
#include<QVariant>


//REDO: warning. in multithread redo to atomic tipe.
static int iTicketCounter {1};
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
            tmPer.tm_mon    = 0;
            tmPer.tm_mday   = 0;
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

class Ticket
{
private:

    std::string             sTicketName;
    std::string             sTicketSign;
    int                     iTicketID;
    std::shared_ptr<Market> ptrMarket;

public:

    inline std::string              TicketName()    const   {return sTicketName;};
    inline std::string              TicketSign()    const   {return sTicketSign;};
    inline int                      TicketID()      const   {return iTicketID;};
    inline int                      MarketID()      const   {return ptrMarket.get()->MarketID();};
    inline std::shared_ptr<Market>  MarketShPtr()   const   {return ptrMarket;};

public:
    //--------------------------------------------------------------------------------------------------------
    // use only explicit constructor. Copy constructor by default is acceptable;
    Ticket() = delete ;
    //--------------------------------------------------------------------------------------------------------
    Ticket(std::string TicketName,std::string TicketSign,std::shared_ptr<Market> Market, int TicketID)
    {
        if(!Market){
            throw std::invalid_argument("Invalid null pointer to market in Ticket()");
        }
        //
        ptrMarket = Market;
        //
        sTicketName = TicketName;
        sTicketSign = TicketSign;
        //iMarketID = ptrMarket.get()->MarketID();
        iTicketID   = TicketID;
        //
        if (TicketID >= iTicketCounter) iTicketCounter = TicketID + 1;
    }
    //
    Ticket(std::string TicketName,std::string TicketSign, std::shared_ptr<Market> Market):Ticket(TicketName,TicketSign, Market,iTicketCounter++){};
    //--------------------------------------------------------------------------------------------------------
};




#endif // TICKET_H
