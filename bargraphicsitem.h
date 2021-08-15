#ifndef BARGRAPHICSITEM_H
#define BARGRAPHICSITEM_H

#include <QGraphicsItem>
#include "bar.h"

class BarGraphicsItem : public QGraphicsItem
{
    Bar b;
    bool bOHLC;
    int iState;
    double dHScale;

    bool IsTick{false};
public:

    static const int nPenWidth{1};
    static const int BarWidth{nPenWidth*7};

public:
    BarGraphicsItem(Bar bb, int State):b{bb},bOHLC{true},iState{State},IsTick{false}{
        dHScale = 2.1;
    };
    BarGraphicsItem(BarTick bb, int State):b{bb},bOHLC{true},iState{State},IsTick{true}{
        dHScale = 2.1;
    };

    virtual QRectF boundingRect() const;

    virtual void paint(QPainter* ppainter, const QStyleOptionGraphicsItem*, QWidget*);



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
