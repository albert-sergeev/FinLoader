#include "combindicator.h"

#include <QPainter>
#include "threadfreecout.h"


CombIndicator::CombIndicator(int MaxLevel, QWidget *parent) :
    QWidget(parent),
    iCurrentLevel{0},
    iMaxLevel{MaxLevel}
{
    this->setCurrentLevel(0);

    setMinimumWidth(iMaxLevel * 4 + 4 + 5 );

    setAttribute(Qt::WA_TranslucentBackground);
}

void CombIndicator::paintEvent(QPaintEvent */*event*/)
{
    QPainter painter(this);
    //painter.setRenderHint(QPainter::Antialiasing, true);

    QPen blackSolidPen(Qt::black,0.5,Qt::SolidLine);
    QPen bluePen(Qt::blue,1,Qt::SolidLine);
    //QPen circlePen(Qt::black,0.5,Qt::SolidLine);
    //QPen circlePen(Qt::black,1,Qt::SolidLine);
    QPen grayPen(Qt::gray,1,Qt::SolidLine);

//    qreal xRadius = 1;
//    qreal yRadius = 1;

    painter.setPen(blackSolidPen);
    //painter.drawRoundedRect(1,1,iMaxLevel * 4 + 5, this->height() - 2,xRadius,yRadius);
    //painter.drawRoundedRect(1,0,iMaxLevel * 4 + 5, this->height() - 1,xRadius,yRadius);
    painter.drawRect(1,0,iMaxLevel * 4 + 5, this->height() - 1);


    for(int i = iMaxLevel * 4 + 2 ; i > 2; i-=4 ){
        if ( (iMaxLevel * 4 + 2 - i) <  iCurrentLevel * 4){
            painter.setPen(bluePen);
        }
        else{
            painter.setPen(grayPen);
        }
//        painter.drawLine(i - 1, 3 ,i - 1, this->height() - 4);
//        painter.drawLine(i, 3 ,i, this->height() - 4);

        painter.drawLine(i - 1, 3 ,i - 1, this->height() - 5);
        painter.drawLine(i    , 3 ,i, this->height() - 5);

//        painter.drawLine(i - 1, 1 ,i - 1, this->height() - 2);
//        painter.drawLine(i    , 1 ,i, this->height() - 2);
    }
}
