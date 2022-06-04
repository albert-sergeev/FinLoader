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

#include "styledswitcher.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPropertyAnimation>
#include <QSignalTransition>
#include <QDebug>

#include "threadfreecout.h"

//////////////////////////////////////////////////////////////
/// \brief Constructor
/// \param Text for left part
/// \param Text for right part
/// \param Init State
/// \param minumum width of the widget
/// \param parent
///
StyledSwitcher::StyledSwitcher(QString Left,QString Right,bool InitState, int btnWidth, QWidget *parent) : QWidget(parent)
{

    sLeft       = Left;
    sRight      = Right;

    bChecked  = InitState;
    ibtnWidth     = btnWidth;
    //////////////////////////////
    lblL = new QLabel(Left,this);
    lblR = new QLabel(Right,this);
    lblL->installEventFilter(this);
    lblR->installEventFilter(this);

    int iWL = lblL->fontMetrics().boundingRect(Left).width();
    int iWR = lblR->fontMetrics().boundingRect(Right).width();
    int iLblMax = iWL > iWR ? iWL : iWR;

    this->setFixedSize(iLblMax + btnWidth,lblL->fontMetrics().boundingRect(Left).height()+2);
    this->setMinimumWidth(iLblMax + btnWidth);

    btnP=new QPushButton(this);
    btnP->setAutoFillBackground(true);
    btnP->show();

    QStateMachine *psm;

    psm = new QStateMachine(this);

    QState *stateOn = new QState(psm);

    QRect recBtnOn (iLblMax             ,0,btnWidth,this->height());
    QRect recLblLOn (iLblMax - iWL      ,0,iWL,this->height());
    QRect recLblROn (iLblMax + btnWidth ,0,iWR,this->height());
    stateOn->assignProperty(btnP,"geometry",recBtnOn);
    stateOn->assignProperty(lblL,"geometry",recLblLOn);
    stateOn->assignProperty(lblR,"geometry",recLblROn);

    QState *stateOff = new QState(psm);

    QRect recBtnOff  (0       ,0,btnWidth,this->height());
    QRect recLblLOff (0 - iWL ,0,iWL,this->height());
    QRect recLblROff (btnWidth,0,iWR,this->height());
    stateOff->assignProperty(btnP,"geometry",recBtnOff);
    stateOff->assignProperty(lblL,"geometry",recLblLOff);
    stateOff->assignProperty(lblR,"geometry",recLblROff);

    connect(stateOn,SIGNAL(activeChanged(bool)),this,SLOT(slotStateOnActivated(bool)));
    connect(stateOff,SIGNAL(activeChanged(bool)),this,SLOT(slotStateOffActivated(bool)));



    if (bChecked)
        psm->setInitialState(stateOn);
    else
        psm->setInitialState(stateOff);

    QSignalTransition * ptrans1 = stateOff->addTransition(this,SIGNAL(DoChangeStateOn()),stateOn);
    QSignalTransition * ptrans2 = stateOn->addTransition(this,SIGNAL(DoChangeStateOff()),stateOff);


    QPropertyAnimation* anim1_1 =new QPropertyAnimation (btnP,"geometry",this);
    QPropertyAnimation* anim1_2 =new QPropertyAnimation (lblL,"geometry",this);
    QPropertyAnimation* anim1_3 =new QPropertyAnimation (lblR,"geometry",this);
    ptrans1->addAnimation(anim1_1);
    ptrans1->addAnimation(anim1_2);
    ptrans1->addAnimation(anim1_3);

    QPropertyAnimation* anim2_1 =new QPropertyAnimation (btnP,"geometry",this);
    QPropertyAnimation* anim2_2 =new QPropertyAnimation (lblL,"geometry",this);
    QPropertyAnimation* anim2_3 =new QPropertyAnimation (lblR,"geometry",this);
    ptrans2->addAnimation(anim2_1);
    ptrans2->addAnimation(anim2_2);
    ptrans2->addAnimation(anim2_3);

    psm->start();




    connect(btnP,SIGNAL(clicked()),this,SLOT(slotButtonClicked()));

    QPalette p = btnP->palette();
    p.setColor(QPalette::Window,Qt::black);
    //p.setColor(QPalette::WindowText,q); // text
    btnP->setAutoFillBackground(true);
    btnP->setPalette(p);

};
//------------------------------------------------------
void StyledSwitcher::slotButtonClicked()
{
    bChecked = !bChecked;
    if (bChecked)
        emit DoChangeStateOn();
    else
        emit DoChangeStateOff();
    emit stateChanged(bChecked);

}
//------------------------------------------------------
void StyledSwitcher::slotStateOnActivated(bool bActive)
{
    if (bActive){
        if (!bChecked){
            emit DoChangeStateOff();
        }
    }
}
void StyledSwitcher::slotStateOffActivated(bool bActive)
{
    if (bActive){
        if (bChecked){
            emit DoChangeStateOn();
        }
    }
}
//------------------------------------------------------
StyledSwitcher::~StyledSwitcher()
{

}
//------------------------------------------------------
bool StyledSwitcher::eventFilter(QObject *watched, QEvent *event){

    if (watched == lblL || watched == lblR){
        if(event->type() == QEvent::MouseButtonPress){
            btnP->click();
        }
    }
    return QObject::eventFilter(watched, event);
}
//------------------------------------------------------
void StyledSwitcher::SetOnColor(const QPalette::ColorRole role, const QColor q)
{

    QPalette p = lblL->palette();
    p.setColor(role,q); // text
    lblL->setAutoFillBackground(true);
    lblL->setPalette(p);
}
//------------------------------------------------------
void StyledSwitcher::SetOffColor(const QPalette::ColorRole role,  const QColor q)
{
    QPalette p = lblR->palette();
    p.setColor(role,q); // text
    lblR->setAutoFillBackground(true);
    lblR->setPalette(p);
}
//------------------------------------------------------

void StyledSwitcher::setChecked(bool bState)
{
    if (bState){
        setCheckState(Qt::CheckState::Checked);
    }
    else{
        setCheckState(Qt::CheckState::Unchecked);
    }
}

//------------------------------------------------------
void StyledSwitcher::setCheckState(Qt::CheckState state)
{
    if (     (state == Qt::CheckState::Checked && !bChecked)
          || (state != Qt::CheckState::Checked && bChecked)
            ){
        bChecked = !bChecked;
        if (bChecked){
            emit DoChangeStateOn();
        }
        else{
            emit DoChangeStateOff();
        }
    }
}
