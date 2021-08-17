#ifndef BARGRAPHICSITEM_H
#define BARGRAPHICSITEM_H

#include <QGraphicsItem>
#include "bar.h"
#include "threadfreelocaltime.h"

class BarGraphicsItem : public QGraphicsItem
{
    Bar b;
    size_t iRealIndex;
    bool bOHLC;
    int iState;
    double dHScale;

    bool IsTick{false};
public:

    static const int nPenWidth{1};
    static const int BarWidth{nPenWidth*7};
    static const int nTickHalfHeight{3};

public:
    BarGraphicsItem(Bar bb,size_t idx, int State):b{bb},iRealIndex{idx},bOHLC{true},iState{State},IsTick{false}{
        dHScale = 2.1;

        std::time_t t = b.Period();
        std::stringstream ss;
        ss <<threadfree_localtime_to_str(&t)<<"\r\n";
        ss << "open: "  << b.Open()<<"\r\n";
        ss << "high: "  << b.High()<<"\r\n";
        ss << "low: "   << b.Low()<<"\r\n";
        ss << "close: " << b.Close()<<"\r\n";
        ss << "value: " << b.Volume()<<"\r\n";
        this->setToolTip(QString::fromStdString(ss.str()));
    };
    BarGraphicsItem(BarTick bb,size_t idx, int State):b{bb},iRealIndex{idx},bOHLC{true},iState{State},IsTick{true}{
        dHScale = 2.1;

        std::time_t t = b.Period();
        std::stringstream ss;
        ss <<threadfree_localtime_to_str(&t)<<"\r\n";
        ss << "close: " << b.Close()<<"\r\n";
        ss << "value: " << b.Volume()<<"\r\n";
        this->setToolTip(QString::fromStdString(ss.str()));
    };

    virtual QRectF boundingRect() const;

    virtual void paint(QPainter* ppainter, const QStyleOptionGraphicsItem*, QWidget*);


    inline std::time_t Period() const {return b.Period();}
    inline size_t  realPosition() const {return iRealIndex;}

    //virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *) ;

private:
    bool IsOHLC()   const  {return  bOHLC;}
    double HScale() const {return dHScale;}

    // QGraphicsItem interface
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *) ;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
};

#endif // BARGRAPHICSITEM_H