#include "styledswitcher.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPropertyAnimation>
#include <QSignalTransition>
#include <QDebug>

#include "threadfreecout.h"


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

//    QSignalTransition * ptrans1 = stateOff->addTransition(btnP,SIGNAL(clicked()),stateOn);
//    QSignalTransition * ptrans2 = stateOn->addTransition(btnP,SIGNAL(clicked()),stateOff);

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
        //bChecked = true;
        if (!bChecked){//redone because init problems
            emit DoChangeStateOff();
        }
    }
}
void StyledSwitcher::slotStateOffActivated(bool bActive)
{
    if (bActive){
//        bChecked = false;
        if (bChecked){//redone because init problems
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
    //p.setColor(QPalette::Window,q);
    //p.setColor(QPalette::WindowText,q); // text
    p.setColor(role,q); // text
    lblL->setAutoFillBackground(true);
    lblL->setPalette(p);
}
//------------------------------------------------------
void StyledSwitcher::SetOffColor(const QPalette::ColorRole role,  const QColor q)
{
    QPalette p = lblR->palette();
    //p.setColor(QPalette::Window,q);
    //p.setColor(QPalette::WindowText,q); // text
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
