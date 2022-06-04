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
