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

#include "transparentbutton.h"
#include "threadfreecout.h"

#include<QPainter>

//----------------------------------------------------------------------------------------------------------------------------
/// \brief TransparentButton::TransparentButton constuctor for button-like mode
/// \param Text
/// \param parent
///
TransparentButton::TransparentButton(QString Text, QWidget *parent) : QWidget(parent),bPushed{false}
{
    strText     = Text;
    modeMode    = eMode::Button;

    font.setPixelSize(12);
    font.setBold(false);
    font.setFamily("Times New Roman");

    iWidth  = 20;
    iHeight = 20;
    setFixedSize(iWidth, iHeight);

    setAttribute(Qt::WA_TranslucentBackground);
}
//----------------------------------------------------------------------------------------------------------------------------
/// \brief TransparentButton::TransparentButton <constuctor for checkbox-like mode
/// \param Text Text for first state
/// \param TextAlternate Text for second state
/// \param State init state
/// \param parent
///
TransparentButton::TransparentButton(QString Text, QString TextAlternate, bool State, QWidget *parent) : QWidget(parent),bPushed{false}
{
    strText             = Text;
    strTextAlternate    = TextAlternate;
    bState              = State;
    modeMode            = eMode::CheckBox;

    font.setPixelSize(12);
    font.setBold(false);
    font.setFamily("Times New Roman");

    iWidth  = 20;
    iHeight = 20;
    setFixedSize(iWidth, iHeight);

    setAttribute(Qt::WA_TranslucentBackground);
}
//----------------------------------------------------------------------------------------------------------------------------
///
/// \brief TransparentButton::paintEvent general paint procedure
///
void TransparentButton::paintEvent(QPaintEvent */*event*/)
{
    // algorithm
    // 1. init variables
    // 2. choose text to paint depend on state and mode
    // 3. calculate draw rect depend on state and mode
    // 4. determine the new size
    // 5. resize and draw

    /////////////////////////////////////////
    // 1. init variables

    QPainter painter(this);

    painter.save();

    painter.setRenderHint(QPainter::Antialiasing, true);

    QPen blackSolidPen(Qt::black,1,Qt::SolidLine);

    QPen grayPen(Qt::gray,0.5,Qt::SolidLine);
    //QPen circlePen(Qt::black,0.5,Qt::SolidLine);
    //QPen circlePen(Qt::black,1,Qt::SolidLine);
    QPen circlePen(Qt::gray,1,Qt::SolidLine);

    painter.setFont(font);

    QRectF rectangle{0,0,1,1};
    QRectF boundRect;
    QString sTxt;

    /////////////////////////////////////////
    // 2. choose text to paint depend on state and mode

    if (modeMode != eMode::CheckBox || bState ){
        sTxt = strText;
    }
    else{
        sTxt = strTextAlternate;
    }

    /////////////////////////////////////////
    // 3 calculate draw rect depend on state and mode

    boundRect = painter.boundingRect(rectangle, Qt::AlignmentFlag::AlignLeft | Qt::AlignmentFlag::AlignTop, sTxt);

    QRectF drawRect;
    QRectF drawTextRect;

    if ((modeMode != eMode::CheckBox && !bPushed) ||
        (modeMode == eMode::CheckBox && !bState)
            ){
        drawRect     = QRectF {1,1,boundRect.width() + 4,boundRect.height() + 1};
        drawTextRect = QRectF {3,2,boundRect.width(),boundRect.height()};
    }
    else{
        drawRect     = QRectF {2,2,boundRect.width() + 4,boundRect.height() + 1};
        drawTextRect = QRectF {4,3,boundRect.width(),boundRect.height()};
    }

    /////////////////////////////////////////
    // 4. determine the new size
    iWidth  = (((int)boundRect.width()) > 0  ? (int)boundRect.width()  + 1 : int (boundRect.width()))  + 8;
    iHeight = (((int)boundRect.height()) > 0 ? (int)boundRect.height() + 1 : int (boundRect.height())) + 4;

    QRectF resizeRect   {0,0,(qreal)iWidth,(qreal)iHeight};


    /////////////////////////////////////////
    // 5. resize and draw

    setFixedSize(resizeRect.width(), resizeRect.height());

    painter.setPen(blackSolidPen);
    painter.drawText(drawTextRect, Qt::AlignmentFlag::AlignLeft | Qt::AlignmentFlag::AlignTop, sTxt);
    painter.setPen(grayPen);
    painter.drawRect(drawRect);

    painter.restore();

}
//----------------------------------------------------------------------------------------------------------------------------
/// \brief TransparentButton::mousePressEvent process mouse press handling
///
void TransparentButton::mousePressEvent(QMouseEvent */*event*/){
    bPushed = true;
    this->repaint(0,0,iWidth,iHeight);
}
//----------------------------------------------------------------------------------------------------------------------------
/// \brief TransparentButton::mouseReleaseEvent mouse release handling
///
void TransparentButton::mouseReleaseEvent(QMouseEvent */*event*/){
    bPushed = false;

    // if in checkmode - remember the next state
    if (modeMode == eMode::CheckBox){
        bState = !bState;
    }
    this->repaint(0,0,iWidth,iHeight);

    emit clicked();
    emit stateChanged(bState);
}
//----------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------


