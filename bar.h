#ifndef BAR_H
#define BAR_H

#include <stdexcept>
#include <sstream>
#include <chrono>



////
/// \brief Main class for store trade operations data for decent time period
///
class Bar
{
private:

    double dOpen;
    double dHigh;
    double dLow;
    double dClose;
    int iValue;
    const int iInterval;
    std::time_t tmPeriod;

public:

    inline double Open()                    const   {return dOpen;};
    inline double High()                    const   {return dHigh;};
    inline double Low()                     const   {return dLow;};
    inline double Close()                   const   {return dClose;};
    inline int Value()                      const   {return iValue;};
    inline int Interval()                   const   {return iInterval;};
    inline std::time_t Period()             const   {return tmPeriod;};

public:
    //--------------------------------------------------------------------------------------------------------
    enum eInterval:int {pTick=0,p1=1,p5=5,p10=10,p15=15,p30=30,p60=60,p120=120,p180=180, pDay=1440, pWeek=10080, pMonth=302400};
    //--------------------------------------------------------------------------------------------------------

    //--------------------------------------------------------------------------------------------------------
    Bar() = delete ;
    //--------------------------------------------------------------------------------------------------------
    Bar (double Open, double High,double Low,double Close,int Value, std::time_t Period, int Interval = eInterval::pTick):
        dOpen{Open},dHigh{High},dLow{Low},dClose{Close},iValue{Value},iInterval{Interval},
        tmPeriod{Period}
    {
        // accomodate time to discret intervals (to up)
        tmPeriod = DateAccomodate(tmPeriod,this->iInterval);

    };
    //--------------------------------------------------------------------------------------------------------
    Bar (Bar &b):
        dOpen{b.dOpen},dHigh{b.dHigh},dLow{b.dLow},dClose{b.dClose},iValue{b.iValue}, iInterval{b.iInterval}, tmPeriod{b.tmPeriod}
    {
    };
    //--------------------------------------------------------------------------------------------------------
    Bar & operator= (const Bar &b)
    {
        if(b.iInterval != iInterval){
            std::stringstream ss;
            ss<<"Invalid interval value [Bar& Bar::operator=(Bar &)] {" << iInterval << "!=" << b.iInterval << "}";
            throw std::invalid_argument(ss.str());
        }

        dOpen       =   b.dOpen;
        dHigh       =   b.dHigh;
        dLow        =   b.dLow;
        dClose      =   b.dClose;
        iValue      =   b.iValue;
        //iInterval   =   b.iInterval;
        tmPeriod    = b.tmPeriod;

        return  *this;
    }
    //
    Bar & operator= (Bar &&b)
    {
        return operator=(b);
    }
    //--------------------------------------------------------------------------------------------------------
    bool operator== (const Bar &b) const
    {
        if(b.iInterval != iInterval){
            std::stringstream ss;
            ss<<"Invalid interval value [Bar& Bar::operator==(Bar &)] {" << iInterval << "!=" << b.iInterval << "}";
            throw std::invalid_argument(ss.str());
        }

        if(
            dOpen       ==   b.dOpen    &&
            dHigh       ==   b.dHigh    &&
            dLow        ==   b.dLow     &&
            dClose      ==   b.dClose   &&
            iValue      ==   b.iValue   &&
            iInterval   ==   b.iInterval &&
            tmPeriod    ==   b.tmPeriod
                )
            return  true;
        else
            return  false;
    }
    //
    bool operator== (Bar &&b) const
    {
        return operator==(b);
    }
    //--------------------------------------------------------------------------------------------------------
    // do compare only by time
    bool operator< (const Bar &b) const
    {
        if(b.iInterval != iInterval){
            std::stringstream ss;
            ss<<"Invalid interval value [Bar& Bar::operator<(Bar &)] {" << iInterval << "!=" << b.iInterval << "}";
            throw std::invalid_argument(ss.str());
        }

        return tmPeriod < b.tmPeriod;
    }
    //--------------------------------------------------------------------------------------------------------
    Bar & Append (const Bar &b)
    {
        if(b.iInterval > iInterval){// must be less or equal
            std::stringstream ss;
            ss<<"Invalid interval value [Bar& Bar::Append(Bar &)] {" << iInterval << "!=" << b.iInterval << "}";
            throw std::invalid_argument(ss.str());
        }
        //dOpen       =   b.dOpen;                      //  first event is in left element
        if(dHigh    <   b.dHigh)    dHigh   = b.dHigh;  //  expand up
        if(dLow     >   b.dLow)     dLow    = b.dLow;   //  expand down
        dClose      =   b.dClose;                       //  last  event is in right element
        iValue      +=   b.iValue;                      //  accumulate
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
    time_t DateAccomodate(time_t &t, int iInterval)
    {
        time_t tRet = t;

        if(iInterval != eInterval::pTick){
            if(iInterval < pDay){
                int iSec = t % (iInterval*60);
                if(iSec>0){
                    tRet = t  + ((iInterval*60) - iSec);
                }
            }
            else{
                std::tm tp  =  *std::localtime(&t);
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

#endif // BAR_H
