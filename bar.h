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
/// expands expands BarTick class for candle data
///
class Bar:public BarTick
{
protected:

    // core data
    double dOpen;
    double dHigh;
    double dLow;
    int iInterval;

    // friend class for direct memory access
    friend class BarMemcopier;

public:

    // get/set methods

    virtual double Open()                    const   {return dOpen;};
    virtual double High()                    const   {return dHigh;};
    virtual double Low()                     const   {return dLow;};
    virtual int Interval()                   const   {return iInterval;};

    using BarTick::Close;

    using BarTick::Volume;
    using BarTick::Period;

    virtual void setOpen     (const double d)                   {dOpen   = d;};
    virtual void setHigh     (const double d)                   {dHigh   = d;};
    virtual void setLow      (const double d)                   {dLow    = d;};

    using BarTick::setClose;
    using BarTick::setVolume;
    virtual void initInterval(const int iv)                     {iInterval = iv;};
    virtual void setPeriod   (const std::time_t tm)             {tmPeriod = DateAccommodate(tm,iInterval);};

public:
    //--------------------------------------------------------------------------------------------------------
    //enum eInterval:int {pTick=(0),p1=1,p5=5,p10=10,p15=15,p30=30,p60=60,p120=120,p180=180, pDay=1440, pWeek=10080, pMonth=302400};
    using BarTick::eInterval;
    using BarTick::castInterval;
    //--------------------------------------------------------------------------------------------------------

    //--------------------------------------------------------------------------------------------------------
    Bar():BarTick{},iInterval{eInterval::pUndefined}{};
    //--------------------------------------------------------------------------------------------------------
    Bar (double Open, double High,double Low,double Close,unsigned long Value, std::time_t Period, int Interval = eInterval::p1):
        BarTick {Close,Value,Period,Interval},
        dOpen{Open},dHigh{High},dLow{Low},iInterval{Interval}

    {};
    //--------------------------------------------------------------------------------------------------------
    Bar (const Bar &b):BarTick {b},
        dOpen{b.dOpen},dHigh{b.dHigh},dLow{b.dLow},iInterval{b.iInterval}
    {
    };
    //--------------------------------------------------------------------------------------------------------
    Bar (const Bar &b, eInterval Interval):BarTick {b},
        dOpen{b.dOpen},dHigh{b.dHigh},dLow{b.dLow},iInterval{Interval}
    {
        tmPeriod = DateAccommodate(tmPeriod,Interval);
    };
    //--------------------------------------------------------------------------------------------------------
    Bar (const BarTick &b):BarTick {b},
        dOpen{b.Close()},dHigh{b.Close()},dLow{b.Close()},iInterval{eInterval::pTick}
    {
    };
    //--------------------------------------------------------------------------------------------------------
    Bar (const BarTick &b, eInterval Interval):BarTick {b},
        dOpen{b.Close()},dHigh{b.Close()},dLow{b.Close()},iInterval{Interval}
    {
        tmPeriod = DateAccommodate(tmPeriod,Interval);
    };
    //--------------------------------------------------------------------------------------------------------
    Bar & reinit (const Bar &b)
    {
        BarTick::reinit(b);
        dOpen       =   b.dOpen;
        dHigh       =   b.dHigh;
        dLow        =   b.dLow;
        iInterval   =   b.iInterval;

        return  *this;
    }
    //--------------------------------------------------------------------------------------------------------
    Bar & operator= (const Bar &b)
    {
        if(b.iInterval != iInterval){
            if(pUndefined != iInterval){
                std::stringstream ss;
                ss<<"Bar::operator=() Invalid interval value [Bar& Bar::operator=(Bar &)] {" << iInterval << "!=" << b.iInterval << "}";
                throw std::invalid_argument(ss.str());
            }
            iInterval = b.iInterval;
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
    // comparison (not used [==] because safety)
    bool equal (const Bar &b) const
    {
        if(b.Interval() != Interval()){
            std::stringstream ss;
            ss<<"Bar::equal() Invalid interval value [Bar& Bar::operator==(Bar &)] {" << iInterval << "!=" << b.iInterval << "}";
            throw std::invalid_argument(ss.str());
        }

        if(
            dOpen       ==   b.dOpen        &&
            dHigh       ==   b.dHigh        &&
            dLow        ==   b.dLow         &&
            dClose      ==   b.dClose       &&
            iVolume     ==   b.iVolume      &&
            Interval()  ==   b.Interval()   &&
            tmPeriod    ==   b.tmPeriod
                )
            return  true;
        else
            return  false;
    }
    // rvalue variant
    bool equal (Bar &&b) const
    {
        return equal(b);
    }
    //--------------------------------------------------------------------------------------------------------
    // do compare only by time
    bool operator< (const Bar &b) const
    {
        if(b.Interval() != Interval()){
            std::stringstream ss;
            ss<<"Bar::operator<() Invalid interval value [Bar& Bar::operator<(Bar &)] {" << iInterval << "!=" << b.iInterval << "}";
            throw std::invalid_argument(ss.str());
        }

        return tmPeriod < b.tmPeriod;
    }
    bool operator< (const std::time_t &t) const
    {
        return tmPeriod < t;
    }
    //--------------------------------------------------------------------------------------------------------
    // for use in LSM tree building
    Bar & Append (const Bar &b)
    {
        if(b.Interval() > Interval()){// must be less or equal
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
