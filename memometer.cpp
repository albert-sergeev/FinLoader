#include "memometer.h"

#include<QPainter>

Memometer::Memometer(QWidget *parent) : QWidget(parent),dPercent{0}
{
    font.setPixelSize(12);
    font.setBold(false);
    font.setFamily("Times New Roman");

    setMinimumWidth(10);
    setMinimumHeight(40);

    setAttribute(Qt::WA_TranslucentBackground);

    setToolTipText("Memometer");
    setValue(0.5);
}


void Memometer::paintEvent(QPaintEvent */*event*/)
{
    QPainter painter(this);
    painter.save();

    painter.setRenderHint(QPainter::Antialiasing, true);

    //QPen blackSolidPen(Qt::black,1,Qt::SolidLine);
    //QPen circlePen(Qt::gray,0.5,Qt::SolidLine);
    QPen blackPen(Qt::black,0.5,Qt::SolidLine);
    //QPen circlePen(Qt::black,1,Qt::SolidLine);
    //QPen circlePen(Qt::gray,1,Qt::SolidLine);


    qreal iHeight = this->height();
    qreal iWidth = this->width();

    qreal iScaleHeight = iHeight - 5 - iWidth;


    painter.setPen(blackPen);

    painter.drawLine(iWidth / 4.0          , 5,
                     iWidth / 4.0          , iScaleHeight);
    painter.drawLine(iWidth * 3.0 / 4.0    , 5,
                     iWidth * 3.0 / 4.0    , iScaleHeight);

    painter.setBrush(Qt::blue);

    QRectF rec (iWidth / 4.0 + 1, iScaleHeight * (1 - dPercent), (iWidth / 2.0) - 3, iScaleHeight * dPercent);
    painter.drawRect(rec);


    /*
    const static int iR{3};
    const static int iC{7};


    {
        {
           // painter.setPen(blackSolidPen);

            painter.drawLine(iC     , iC - iR, iC       , iC + iR);
            painter.drawLine(iC - iR, iC     , iC + iR  , iC);

            painter.setPen(circlePen);

            painter.drawEllipse(iC - iR - 3 , iC - iR - 3,iR*4,iR*4);
        }
    }*/
    painter.restore();

}
