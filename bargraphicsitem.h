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

#ifndef BARGRAPHICSITEM_H
#define BARGRAPHICSITEM_H

#include <QGraphicsItem>
#include "bar.h"
#include "threadfreelocaltime.h"



///////////////////////////////////////////////
/// \brief Graphic Item for using in main view as candle
///
class BarGraphicsItem : public QGraphicsItem
{
    ///////////////////////////////////////////////////////
    Bar b;              // core data of element
    size_t iRealIndex;  // index in LSA Tree to position
    bool bOHLC;         // view type: candle or OHLC
    int iState;         // view type: color depends on state
    double dHScale;     // scale
    bool IsTick{false}; // view type: simple (tick) or complex (candle/OHLC)
public:

    // constants for using in graphviews
    static const int nPenWidth{1};
    static const int BarWidth{nPenWidth*7};
    static const int nTickHalfHeight{3};

public:

    ////
    /// \brief BarGraphicsItem main constructor specialized for Bar type
    /// \param bb       - Bar type core data
    /// \param idx      - index in LSA tree
    /// \param State    - state of bar
    /// \param dS       - scale
    ///
    BarGraphicsItem(Bar bb,size_t idx, int State, double dS):
        b{bb},iRealIndex{idx},bOHLC{true},iState{State},
        dHScale{dS},
        IsTick{false}{
        //dHScale = 2.1;

        std::time_t t = b.Period();
        std::stringstream ss;
        if (b.Interval() >= Bar::eInterval::pDay){
            ss <<threadfree_gmtime_date_to_str(&t)<<"\r\n";
        }
        else{
            ss <<threadfree_gmtime_to_str(&t)<<"\r\n";
        }
        ss << "open: "  << b.Open()<<"\r\n";
        ss << "high: "  << b.High()<<"\r\n";
        ss << "low: "   << b.Low()<<"\r\n";
        ss << "close: " << b.Close()<<"\r\n";
        ss << "volume: " << b.Volume();
        ss <<"\n"<< "index: " << iRealIndex<<"";

        this->setToolTip(QString::fromStdString(ss.str()));
    };
    ////
    /// \brief BarGraphicsItem main constructor specialized for _BarTick_ type
    /// \param bb       - Bar type core data
    /// \param idx      - index in LSA tree
    /// \param State    - state of bar
    /// \param dS       - scale
    ///
    BarGraphicsItem(BarTick bb,size_t idx, int State, double dS):
        b{bb},iRealIndex{idx},bOHLC{true},iState{State},
        dHScale{dS},
        IsTick{true}{
        //dHScale = 2.1;

        std::time_t t = b.Period();
        std::stringstream ss;
        if (b.Interval() >= Bar::eInterval::pDay){
            ss <<threadfree_gmtime_date_to_str(&t)<<"\r\n";
        }
        else{
            ss <<threadfree_gmtime_to_str(&t)<<"\r\n";
        }
        ss << "close: " << b.Close()<<"\r\n";
        ss << "volume: " << b.Volume();
        ss <<"\n"<< "index: " << iRealIndex<<"";
        this->setToolTip(QString::fromStdString(ss.str()));
    };

    // methods for using when draw
    virtual QRectF boundingRect() const;
    virtual void paint(QPainter* ppainter, const QStyleOptionGraphicsItem*, QWidget*);


    // utilities
    inline std::time_t Period() const {return b.Period();}
    inline size_t  realPosition() const {return iRealIndex;}

    //virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *) ;

    // modification methods
    void setBar(const Bar &bb)         {b = bb;};
    void setBar(const BarTick &bb)     {b = bb;};

    // delegate method for dinamic change view type of item
    template <class F, class... Args>
    void SetOHLC(F&& f, Args&&... args){funcOHLC = std::bind (f,args...);}

private:

    std::function<bool()> funcOHLC;
    bool IsOHLC()   const  {return  funcOHLC();}

    double HScale() const {return dHScale;}


    // QGraphicsItem interface
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *) ;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
};

#endif // BARGRAPHICSITEM_H
