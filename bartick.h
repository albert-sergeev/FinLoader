#ifndef BarTICK_H
#define BarTICK_H

#include <stdexcept>
#include <sstream>
#include <chrono>
#include "threadfreelocaltime.h"


class BarTickMemcopier;


class BarTick
{
protected:
    double dClose;
    unsigned long iVolume;
    std::time_t tmPeriod;

    friend class BarTickMemcopier;

public:
    //--------------------------------------------------------------------------------------------------------
    enum eInterval:int {pTick=(0),p1=1,p5=5,p10=10,p15=15,p30=30,p60=60,p120=120,p180=180, pDay=1440, pWeek=10080, pMonth=302400};
    //--------------------------------------------------------------------------------------------------------

    virtual double Open()                   const   {return dClose;};
    virtual double High()                   const   {return dClose;};
    virtual double Low()                    const   {return dClose;};
    virtual int Interval()                  const   {return eInterval::pTick;};

    inline double Close()                   const   {return dClose;};
    inline unsigned long Volume()           const   {return iVolume;};
    inline std::time_t Period()             const   {return tmPeriod;};

    virtual void setOpen     (const double d)                  {dClose   = d;};
    virtual void setHigh     (const double d)                  {dClose   = d;};
    virtual void setLow      (const double d)                  {dClose    = d;};
    virtual void initInterval(const int /*iv*/)                {;};
    virtual void setPeriod   (const std::time_t tm)            {tmPeriod = DateAccommodate(tm,eInterval::pTick);};

    inline void setClose    (const double d)                   {dClose  = d;};
    inline void setVolume   (const unsigned long v)            {iVolume = v;};


public:


    //--------------------------------------------------------------------------------------------------------
    BarTick(){};
    //--------------------------------------------------------------------------------------------------------
    BarTick (double Close,unsigned long Value, std::time_t Period, int /*Interval*/ = eInterval::pTick):
        dClose{Close},iVolume{Value},tmPeriod{Period}
    {
        // accomodate time to discret intervals (to up)
        tmPeriod = DateAccommodate(tmPeriod,eInterval::pTick);

    };
    //--------------------------------------------------------------------------------------------------------
    BarTick (const BarTick &b):
        dClose{b.dClose},iVolume{b.iVolume}, tmPeriod{b.tmPeriod}
    {
    };
    //--------------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------------
    BarTick & reinit (const BarTick &b)
    {
        dClose      =   b.dClose;
        iVolume     =   b.iVolume;
        tmPeriod    =   b.tmPeriod;
        return  *this;
    }
    //--------------------------------------------------------------------------------------------------------
    BarTick & operator= (const BarTick &b)
    {
        dClose      =   b.dClose;
        iVolume     =   b.iVolume;
        tmPeriod    =   b.tmPeriod;

        return  *this;
    }
    //
    BarTick & operator= (BarTick &&b)
    {
        return operator=(b);
    }
    //--------------------------------------------------------------------------------------------------------
//    bool operator==(const BarTick &b) const
//    {
//        return equal(b);
//    }
//    bool operator==(const BarTick &&b) const
//    {
//        return equal(b);
//    }
    //--------------------------------------------------------------------------------------------------------
    bool equal (const BarTick &b) const
    {
        if(
            dClose      ==   b.dClose   &&
            iVolume     ==   b.iVolume   &&
            tmPeriod    ==   b.tmPeriod
                )
            return  true;
        else
            return  false;
    }
    //
    bool equal (BarTick &&b) const
    {
        return equal(b);
    }
    //--------------------------------------------------------------------------------------------------------
    // do compare only by time
    bool operator< (const BarTick &b) const
    {
        return tmPeriod < b.tmPeriod;
    }
    //--------------------------------------------------------------------------------------------------------
    BarTick & Append (const BarTick &b)
    {
        //dOpen       =   b.dOpen;                      //  first event is in left element
        dClose      =   b.dClose;                       //  last  event is in right element
        iVolume      +=   b.iVolume;                      //  accumulate
        //iInterval   =   b.iInterval;                  //  const
        //
        return  *this;
    }
    //
    BarTick & Append (BarTick &&b)
    {
        return Append(b);
    }
    //--------------------------------------------------------------------------------------------------------
    // align dates to discret marks
    static time_t DateAccommodate(time_t t, int iInterval)
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

class BarTickMemcopier{

    BarTick &bb;

public:

    BarTickMemcopier(BarTick &b):bb{b}{;}

    inline double& Close()           {return bb.dClose;};
    inline unsigned long& Volume()   {return bb.iVolume;};
    inline std::time_t& Period()     {return bb.tmPeriod;};

    BarTickMemcopier() = delete;
    BarTickMemcopier(BarTickMemcopier&) = delete;
    BarTickMemcopier& operator= (BarTickMemcopier&) = delete;
};


#endif // BarTICK_H
