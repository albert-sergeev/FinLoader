#ifndef BAR_H
#define BAR_H

#include <stdexcept>
#include <sstream>
#include <chrono>

#include "bartick.h"

#include "threadfreelocaltime.h"


class BarMemcopier;

////
/// \brief Main class for store trade operations data for decent time period
///
class Bar:public BarTick
{
protected:

    double dOpen;
    double dHigh;
    double dLow;

    friend class BarMemcopier;

public:

    virtual double Open()                    const   {return dOpen;};
    virtual double High()                    const   {return dHigh;};
    virtual double Low()                     const   {return dLow;};

    using BarTick::Close;

    using BarTick::Volume;
    using BarTick::Interval;
    using BarTick::Period;

    virtual void setOpen     (const double d)                   {dOpen   = d;};
    virtual void setHigh     (const double d)                   {dHigh   = d;};
    virtual void setLow      (const double d)                   {dLow    = d;};

    using BarTick::setClose;
    using BarTick::setVolume;
    using BarTick::initInterval;
    using BarTick::setPeriod;

public:
    //--------------------------------------------------------------------------------------------------------
    //enum eInterval:int {pTick=(0),p1=1,p5=5,p10=10,p15=15,p30=30,p60=60,p120=120,p180=180, pDay=1440, pWeek=10080, pMonth=302400};
    using BarTick::eInterval;
    //--------------------------------------------------------------------------------------------------------

    //--------------------------------------------------------------------------------------------------------
    Bar():BarTick{}{};
    //--------------------------------------------------------------------------------------------------------
    Bar (double Open, double High,double Low,double Close,unsigned long Value, std::time_t Period, int Interval = eInterval::pTick):
        BarTick {Close,Value,Period,Interval},
        dOpen{Open},dHigh{High},dLow{Low}

    {};
    //--------------------------------------------------------------------------------------------------------
    Bar (const Bar &b):BarTick {b},
        dOpen{b.dOpen},dHigh{b.dHigh},dLow{b.dLow}
    {
    };
    //--------------------------------------------------------------------------------------------------------
    Bar (const BarTick &b):BarTick {b},
        dOpen{b.Close()},dHigh{b.Close()},dLow{b.Close()}
    {
    };
    //--------------------------------------------------------------------------------------------------------
    Bar & reinit (const Bar &b)
    {
        BarTick::reinit(b);
        dOpen       =   b.dOpen;
        dHigh       =   b.dHigh;
        dLow        =   b.dLow;

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
    // align dates to discret marks
    using  BarTick::DateAccommodate;
};

//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
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
