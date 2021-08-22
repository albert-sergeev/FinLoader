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
    QPointF ptPosition;
    QSizeF size;
    if (IsTick){
        ptPosition = QPointF(-((BarWidth+1)/2),
                             -(HScale() * nTickHalfHeight)-nPenWidth);
        size = QSizeF(BarWidth , (HScale()*nTickHalfHeight*2)+2*nPenWidth);
    }
    else{
        ptPosition = QPointF(-((BarWidth+1)/2),
                             -(HScale() * (b.High()-b.Close()))
                             );
        size = QSizeF(BarWidth, ((b.High()-b.Low())*HScale()));
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
    painter->setRenderHint(QPainter::Antialiasing, true);

    if (IsOHLC()){
        if (IsTick){
            painter->drawLine(0                ,-(HScale() * nTickHalfHeight),0                  ,+(HScale() * nTickHalfHeight) ); // HLine
          //painter->drawLine(-nPenWidth * 2   ,0                            ,0                  ,0                             ); // OpenLine
            painter->drawLine(0                ,0                            ,0 + nPenWidth * 2  ,0                             ); // CloseLine
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
