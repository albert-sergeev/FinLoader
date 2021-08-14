#ifndef BAR_H
#define BAR_H

#include <stdexcept>
#include <sstream>
#include <chrono>
#include "threadfreelocaltime.h"


class BarMemcopier;

////
/// \brief Main class for store trade operations data for decent time period
///
class Bar
{
    double dOpen;
    double dHigh;
    double dLow;
    double dClose;
    unsigned long iVolume;
    std::time_t tmPeriod;

    int iInterval;

    friend class BarMemcopier;

public:

    inline double Open()                    const   {return dOpen;};
    inline double High()                    const   {return dHigh;};
    inline double Low()                     const   {return dLow;};
    inline double Close()                   const   {return dClose;};
    inline unsigned long Volume()           const   {return iVolume;};
    inline int Interval()                   const   {return iInterval;};
    inline std::time_t Period()             const   {return tmPeriod;};

    inline void setOpen     (const double d)                   {dOpen   = d;};
    inline void setHigh     (const double d)                   {dHigh   = d;};
    inline void setLow      (const double d)                   {dLow    = d;};
    inline void setClose    (const double d)                   {dClose  = d;};
    inline void setVolume   (const unsigned long v)            {iVolume = v;};
    inline void initInterval(const int iv)                     {iInterval    = iv;};
    inline void setPeriod   (const std::time_t tm)             {tmPeriod = DateAccommodate(tm,this->iInterval);};

public:
    //--------------------------------------------------------------------------------------------------------
    enum eInterval:int {pTick=(0),p1=1,p5=5,p10=10,p15=15,p30=30,p60=60,p120=120,p180=180, pDay=1440, pWeek=10080, pMonth=302400};
    //--------------------------------------------------------------------------------------------------------

    //--------------------------------------------------------------------------------------------------------
    Bar():iInterval{eInterval::pTick}{};
    //--------------------------------------------------------------------------------------------------------
    Bar (double Open, double High,double Low,double Close,unsigned long Value, std::time_t Period, int Interval = eInterval::pTick):
        dOpen{Open},dHigh{High},dLow{Low},dClose{Close},iVolume{Value},tmPeriod{Period}
        ,iInterval{Interval}
    {
        // accomodate time to discret intervals (to up)
        tmPeriod = DateAccommodate(tmPeriod,this->iInterval);

    };
    //--------------------------------------------------------------------------------------------------------
    Bar (const Bar &b):
        dOpen{b.dOpen},dHigh{b.dHigh},dLow{b.dLow},dClose{b.dClose},iVolume{b.iVolume}, tmPeriod{b.tmPeriod}, iInterval{b.iInterval}
    {
    };
    //--------------------------------------------------------------------------------------------------------
    Bar & reinit (const Bar &b)
    {
        dOpen       =   b.dOpen;
        dHigh       =   b.dHigh;
        dLow        =   b.dLow;
        dClose      =   b.dClose;
        iVolume     =   b.iVolume;
        tmPeriod    =   b.tmPeriod;

        iInterval   =   b.iInterval;
        return  *this;
    }
    //--------------------------------------------------------------------------------------------------------
    Bar & operator= (const Bar &b)
    {
        if(b.iInterval != iInterval){
            std::stringstream ss;
            ss<<"Bar::operator=() Invalid interval value [Bar& Bar::operator=(Bar &)] {" << iInterval << "!=" << b.iInterval << "}";
            throw std::invalid_argument(ss.str());
        }

        dOpen       =   b.dOpen;
        dHigh       =   b.dHigh;
        dLow        =   b.dLow;
        dClose      =   b.dClose;
        iVolume     =   b.iVolume;
        tmPeriod    =   b.tmPeriod;

        //iInterval   =   b.iInterval;

        return  *this;
    }
    //
    Bar & operator= (Bar &&b)
    {
        return operator=(b);
    }
    //--------------------------------------------------------------------------------------------------------
//    bool operator==(const Bar &b) const
//    {
//        return equal(b);
//    }
//    bool operator==(const Bar &&b) const
//    {
//        return equal(b);
//    }
    //--------------------------------------------------------------------------------------------------------
    bool equal (const Bar &b) const
    {
        if(b.iInterval != iInterval){
            std::stringstream ss;
            ss<<"Bar::equal() Invalid interval value [Bar& Bar::operator==(Bar &)] {" << iInterval << "!=" << b.iInterval << "}";
            throw std::invalid_argument(ss.str());
        }

        if(
            dOpen       ==   b.dOpen    &&
            dHigh       ==   b.dHigh    &&
            dLow        ==   b.dLow     &&
            dClose      ==   b.dClose   &&
            iVolume     ==   b.iVolume   &&
            iInterval   ==   b.iInterval &&
            tmPeriod    ==   b.tmPeriod
                )
            return  true;
        else
            return  false;
    }
    //
    bool equal (Bar &&b) const
    {
        return equal(b);
    }
    //--------------------------------------------------------------------------------------------------------
    // do compare only by time
    bool operator< (const Bar &b) const
    {
        if(b.iInterval != iInterval){
            std::stringstream ss;
            ss<<"Bar::operator<() Invalid interval value [Bar& Bar::operator<(Bar &)] {" << iInterval << "!=" << b.iInterval << "}";
            throw std::invalid_argument(ss.str());
        }

        return tmPeriod < b.tmPeriod;
    }
    //--------------------------------------------------------------------------------------------------------
    Bar & Append (const Bar &b)
    {
        if(b.iInterval > iInterval){// must be less or equal
            std::stringstream ss;
            ss<<"Bar::Append() Invalid interval value [Bar& Bar::Append(Bar &)] {" << iInterval << "!=" << b.iInterval << "}";
            throw std::invalid_argument(ss.str());
        }
        //dOpen       =   b.dOpen;                      //  first event is in left element
        if(dHigh    <   b.dHigh)    dHigh   = b.dHigh;  //  expand up
        if(dLow     >   b.dLow)     dLow    = b.dLow;   //  expand down
        dClose      =   b.dClose;                       //  last  event is in right element
        iVolume      +=   b.iVolume;                      //  accumulate
        //iInterval   =   b.iInterval;                  //  const
        //
        return  *this;
    }
    //
    Bar & Append (Bar &&b)
    {
        return Append(b);
    }
    //--------------------------------------------------------------------------------------------------------
private:
    //--------------------------------------------------------------------------------------------------------
    // align dates to discret marks
    time_t DateAccommodate(time_t t, int iInterval)
    {
        time_t tRet = t;

        //1. ticks dont align - they use seconds
        //2. interdays align by math: append addition to remainder of divide by interval
        //3. days and more by manipulate days and substract hours and mins


        if(iInterval != eInterval::pTick){
            if(iInterval < pDay){
                int iSec = t % (iInterval*60);
                if(iSec>0){
                    tRet = t  + ((iInterval*60) - iSec);
                }
            }
            else{
                std::tm tp =  *threadfree_localtime(&t);

                if(iInterval == eInterval::pWeek){//weeks align to mondeys
                    t= t -  ((tp.tm_wday)*86400);
                    tp  =  *threadfree_localtime(&t);
                }
                else if(iInterval == eInterval::pMonth){// month to first day
                    tp.tm_mday = 1;
                }

                tp.tm_hour = 0;
                tp.tm_min = 0;
                tp.tm_sec = 0;
                tp.tm_isdst = 0;

                tRet = std::mktime(&tp);
            }
        }

        return tRet;
    }
    //--------------------------------------------------------------------------------------------------------

};

class BarMemcopier{

    Bar &bb;

public:

    BarMemcopier(Bar &b):bb{b}{;}

    inline double& Open()            {return bb.dOpen;};
    inline double& High()            {return bb.dHigh;};
    inline double& Low()             {return bb.dLow;};
    inline double& Close()           {return bb.dClose;};
    inline unsigned long& Volume()   {return bb.iVolume;};
    inline std::time_t& Period()     {return bb.tmPeriod;};

    BarMemcopier() = delete;
    BarMemcopier(BarMemcopier&) = delete;
    BarMemcopier& operator= (BarMemcopier&) = delete;
};


#endif // BAR_H
