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
    //QPen blackSolidPen(Qt::black,1,Qt::SolidLine);
    //QPen circlePen(Qt::gray,0.5,Qt::SolidLine);
    QPen blackPen(Qt::black,0.5,Qt::SolidLine);
    //QPen circlePen(Qt::black,1,Qt::SolidLine);
    //QPen circlePen(Qt::gray,1,Qt::SolidLine);

    qreal iHeight = this->height();
    qreal iWidth = this->width();
    qreal iScaleHeight = iHeight - 5 - iWidth;

    ////-------------------------------------------------
    QPainter painter(this);
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(blackPen);

    painter.drawLine(iWidth * 2.0/ 4.0 , 5,
                     iWidth * 2.0/ 4.0 , iScaleHeight + 5);
    painter.drawLine(iWidth            , 5,
                     iWidth            , iScaleHeight + 5);

    painter.restore();
    painter.end();
    /////////////////////////////////////////////////////
    QRect rectargetHat (iWidth / 2.0, 5 - iWidth / 4.0, iWidth / 2.0, iWidth / 2.0);
    QRect rectHat      (0,            0, iWidth / 2.0, iWidth / 2.0);
    QRect rectHatRazor (0, iWidth / 4.0 + 1, iWidth / 2.0, iWidth / 2.0);
    QImage sourceHatImage(rectHat.size(),QImage::Format_ARGB32_Premultiplied);
    QImage resultHatImage(rectHat.size(),QImage::Format_ARGB32_Premultiplied);
    sourceHatImage.fill(QColor(0,0,0,0));


    QRect rectargetEl (0, iHeight - 5 - iWidth, iWidth, iWidth);
    QRect rectargetElInt (1, iHeight - 5 - iWidth + 1, iWidth - 2, iWidth - 2);
    QRect recEl (0, 0, iWidth, iWidth);
    QImage sourceElRectImage(recEl.size(),QImage::Format_ARGB32_Premultiplied);
    QImage resultElRectImage(recEl.size(),QImage::Format_ARGB32_Premultiplied);
    sourceElRectImage.fill(QColor(0,0,0,0));
    QRectF recMometr (iWidth * 2.0 / 4.0 + 1, iScaleHeight * (1 - dPercent) + 5, (iWidth / 2.0) - 3, iScaleHeight * dPercent);
    QRectF recM0 (iWidth * 2.0 / 4.0 + 1, iHeight - 5 - iWidth, (iWidth / 2.0) - 3, iWidth/2.0);

    QRect recRect (iWidth * 2.0/ 4.0, 0,
                   iWidth * 2.0/ 4.0, iWidth - 5);
    ////-------------------------------------------------
    painter.begin(&sourceHatImage);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(blackPen);
    painter.drawEllipse(rectHat);
    painter.end();
    ////-------------------------------------------------
    painter.begin(&resultHatImage);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.setPen(blackPen);
    painter.setBrush(Qt::green);
    painter.drawRect(rectHatRazor);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOut);
    painter.drawImage(rectHat,sourceHatImage);
    painter.end();
    ////-------------------------------------------------
    painter.begin(&sourceElRectImage);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(blackPen);
    painter.drawEllipse(recEl);
    painter.end();
    ////-------------------------------------------------
    painter.begin(&resultElRectImage);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.setPen(blackPen);
    painter.setBrush(Qt::green);
    painter.drawRect(recRect);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOut);
    painter.drawImage(recEl,sourceElRectImage);
    painter.end();
    ////-------------------------------------------------
    painter.begin(this);
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(blackPen);
    painter.drawImage(rectargetHat,resultHatImage);
    painter.drawImage(rectargetEl,resultElRectImage);

    painter.setBrush(Qt::blue);
    painter.drawEllipse(rectargetElInt);
    painter.drawRect(recM0);

    painter.drawRect(recMometr);

    painter.restore();
    painter.end();
    /////////////////////////////////////////////////////

}
