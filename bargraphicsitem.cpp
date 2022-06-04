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
                             -(/*HScale() **/ nTickHalfHeight)-nPenWidth);
        size = QSizeF(BarWidth , (/*HScale()**/nTickHalfHeight*2)+2*nPenWidth);
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

    //painter->setRenderHint(QPainter::Antialiasing, true);

    if (IsTick){
        painter->setPen(QPen(qtColor,nPenWidth));
        painter->drawLine(0                ,-(/*HScale() **/ nTickHalfHeight),0                  ,+(/*HScale() **/ nTickHalfHeight) ); // HLine
        painter->drawLine(0                ,0                            ,0 + nPenWidth * 2  ,0                             ); // CloseLine
    }
    else{
        if (IsOHLC())
        {
            if (b.Open() > b.Close())   qtColor = Qt::red;
            else                        qtColor = Qt::darkGreen;

            painter->setPen(QPen(qtColor,nPenWidth));

            painter->setPen(QPen(qtColor,nPenWidth));

            painter->drawLine(0                ,-(HScale() * (b.High()-b.Close()))  ,0                  ,-(HScale() * (b.Low()-b.Close()))  ); // HLine
            painter->drawLine(-nPenWidth * 2   ,-(HScale() * (b.Open()-b.Close()))  ,0                  ,-(HScale() * (b.Open()-b.Close())) ); // OpenLine
            painter->drawLine(0                ,0                                   ,0 + nPenWidth * 2  ,0                                  ); // CloseLine

        }
        else
        {
            if (b.Open() > b.Close())   qtColor = Qt::red;
            else                        qtColor = Qt::darkGreen;

            painter->setPen(QPen(qtColor,nPenWidth));
            painter->setBrush(qtColor);

            QRectF rec (-nPenWidth * 2, -(HScale() * (b.Open()-b.Close())), nPenWidth * 4, (HScale() * (b.Open()-b.Close())));

            painter->drawRect(rec);
            painter->drawLine(0                ,-(HScale() * (b.High()-b.Close()))  ,0                  ,-(HScale() * (b.Low()-b.Close()))  ); // HLine
        }
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
