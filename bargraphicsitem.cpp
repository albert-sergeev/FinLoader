#include<QApplication>
#include<QPainter>

#include "bargraphicsitem.h"
#include "threadfreecout.h"

//BarGraphicsItem::BarGraphicsItem()
//{

//}

//----------------------------------------------------------------------------------------------------
QRectF BarGraphicsItem::boundingRect() const
{
    QPointF ptPosition(-(BarWidth/2+1),-(HScale() * (b.Low()-b.Close())));
    QSizeF size;
    if (IsTick){
        size = QSizeF(BarWidth, 20+2);
    }
    else{
        size = QSizeF(BarWidth, (HScale() * (b.High()-b.Low())));
    }

    return QRectF(ptPosition, size);
}
//----------------------------------------------------------------------------------------------------
void BarGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{


    QColor qtColor;

    switch (iState) {
    case 0: qtColor = Qt::green; break;
    case 1: qtColor = Qt::blue; break;
    case 2: qtColor = Qt::red; break;
    default:qtColor = Qt::magenta; break;
    }
    painter->save();
    painter->setPen(QPen(qtColor,nPenWidth));

    if (IsOHLC()){
        if (IsTick){
            painter->drawLine(0                ,-(HScale() * 10),0                  ,+(HScale() * 10)); // HLine
            //painter->drawLine(-nPenWidth * 2   ,0               ,0                  ,0               ); // OpenLine
            painter->drawLine(0                ,0               ,0 + nPenWidth * 2  ,0               ); // CloseLine
        }
        else{
            painter->drawLine(0                ,-(HScale() * (b.High()-b.Close()))  ,0                  ,-(HScale() * (b.Low()-b.Close()))  ); // HLine
            painter->drawLine(-nPenWidth * 2   ,-(HScale() * (b.Open()-b.Close()))  ,0                  ,-(HScale() * (b.Open()-b.Close())) ); // OpenLine
            painter->drawLine(0                ,0                                   ,0 + nPenWidth * 2  ,0                                  ); // CloseLine
        }
    }
    else{
        ;
    }
    painter->restore();
}
//----------------------------------------------------------------------------------------------------
void BarGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent */*event*/)
{
    QApplication::setOverrideCursor(Qt::PointingHandCursor);
    //QGraphicsItem::mousePressEvent(event);
}
//----------------------------------------------------------------------------------------------------
void BarGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent */*event*/)
{
    QApplication::restoreOverrideCursor();
    //QGraphicsItem::mouseReleaseEvent(event);
}
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
