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

#include "bulbululator.h"
#include<QGridLayout>
#include<QPropertyAnimation>
#include<QEvent>
#include<QPainter>
#include<QGraphicsView>




#include "threadfreecout.h"

//-----------------------------------------------------------------------------------------
Bulbululator::Bulbululator(QWidget *parent) : QWidget(parent)
{


    /////////////////////////////////////////
    QGridLayout *lt=new QGridLayout();
    lblMain = new QLabel("ABBA");
    lt->addWidget(lblMain);
    lblMain->setMargin(0);
    lt->setMargin(0);
    this->setLayout(lt);
    lblMain->setFrameShape(QFrame::StyledPanel);

    iProcessCount = 1;
    stState = eTickerState::Connected;

    /////////////////////////////////
    dtStartBlink = std::chrono::time_point<std::chrono::steady_clock>{};
    bBlink = false;
    defPallete = lblMain->palette();
    /////////////////////////////////

    this->startTimer(100);
}
//-----------------------------------------------------------------------------------------
Bulbululator::~Bulbululator()
{
//    if (buble != nullptr){
//        buble->close();
//        delete  buble;
//    }
}
//-----------------------------------------------------------------------------------------
void Bulbululator::timerEvent(QTimerEvent * /*event*/)
{
    if (bBlink){
        std::chrono::time_point dtNow(std::chrono::steady_clock::now());
        milliseconds tCount = dtNow - dtStartBlink;

        if (tCount.count()>100){
            bBlink = false;
           // lblMain->setPalette(defPallete);
            QPalette p = defPallete;
            switch (stState) {
            case eTickerState::Informant:
                p.setColor(QPalette::Window,QColor(Qt::lightGray));
                lblMain->setAutoFillBackground(true);
                lblMain->setPalette(p);
                break;
            case eTickerState::Connected:
                lblMain->setPalette(defPallete);
                break;
            case eTickerState::NeededPipe:
                p.setColor(QPalette::Window,QColor(Qt::magenta));
                lblMain->setAutoFillBackground(true);
                lblMain->setPalette(p);
                break;
            case eTickerState::Halted:
                p.setColor(QPalette::Window,QColor(Qt::red));
                lblMain->setAutoFillBackground(true);
                lblMain->setPalette(p);
                break;
            }
            dtStartBlink = std::chrono::steady_clock::now();
        }
    }
}
//-----------------------------------------------------------------------------------------
void Bulbululator::setState(eTickerState State)
{
    stState = State;
    if (!bBlink){
        QPalette p = defPallete;
        switch (stState) {
        case eTickerState::Informant:
            p.setColor(QPalette::Window,QColor(Qt::lightGray));
            lblMain->setAutoFillBackground(true);
            lblMain->setPalette(p);
            break;
        case eTickerState::Connected:
            lblMain->setPalette(defPallete);
            break;
        case eTickerState::NeededPipe:
            p.setColor(QPalette::Window,QColor(Qt::magenta));
            lblMain->setAutoFillBackground(true);
            lblMain->setPalette(p);
            break;
        case eTickerState::Halted:
            p.setColor(QPalette::Window,QColor(Qt::red));
            lblMain->setAutoFillBackground(true);
            lblMain->setPalette(p);
            break;
        }
    }
}
//-----------------------------------------------------------------------------------------
void Bulbululator::Bubble()
{

    std::chrono::time_point dtNow(std::chrono::steady_clock::now());
    milliseconds tCount = dtNow - dtStartBlink;
    dtStartBlink = std::chrono::steady_clock::now();

    if (!bBlink && tCount.count()>100){
        dtStartBlink = std::chrono::steady_clock::now();
        bBlink = true;

        //QColor colorGreen(0, 255, 0, 128);
        //QColor colorDarkGreen(0, 59, 56, 128);
        QColor colorDarkGreen(0, 100, 52,200);

        QPalette p = this->palette();
        //p.setColor(QPalette::WindowText,Qt::green); // text
        //p.setColor(QPalette::Window,Qt::green);
        p.setColor(QPalette::Window,colorDarkGreen);
        lblMain->setAutoFillBackground(true);
        lblMain->setPalette(p);
    }

//    if (!bStartBuble){
//        bStartBuble = true;
////        //buble = new QLabel();
////        buble = new QWidget();

//        QPoint pG = this->mapToGlobal(this->pos());

////        ///////////////////////////////////////////
//        QPropertyAnimation *pan1=new QPropertyAnimation(&bubleB,"geometry");
//        pan1->setDuration(3000);
//        pan1->setStartValue(QRect(0,0,3,3));
//        //pan1->setStartValue(QRect(480,380,200,200));
//        pan1->setEndValue(QRect(0,0,5,5));
//        pan1->setEasingCurve(QEasingCurve::InOutBounce);

//        QPropertyAnimation *pan2=new QPropertyAnimation(&bubleB,"pos");
//        pan2->setDuration(3000);
////        pan2->setStartValue(QPoint(bubleB.pos().x() + bubleB.width()/2,this->height()));
////        pan2->setEndValue(QPoint(bubleB.pos().x()+ bubleB.width()/2,0));
////        auto p (this->pos());
////        QPoint ptStart (200,this->pos().y());
//        //QGraphicsView::mapFromScene()
//        //QPoint ppp = this->mapFromParent(this->pos());


//        pan2->setStartValue(QPoint(pG.x() + this->width()/2,pG.y()));
//        pan2->setEndValue(QPoint(pG.x()+ this->width()/2,pG.y()/2));
//        //pan2->setEasingCurve(QEasingCurve::InOutExpo);
//        //pan2->setEasingCurve(QEasingCurve::OutExpo);
//        pan2->setEasingCurve(QEasingCurve::OutCubic);

//        QPropertyAnimation *pan3=new QPropertyAnimation(&bubleB,"pos");
//        pan3->setStartValue(QPoint(pG.x(),pG.y()));
//        pan3->setEndValue(QPoint(pG.x()+ this->width(),pG.y()/2));
//        pan3->setEasingCurve(QEasingCurve::InOutExpo);
//        ;



//        pag.addAnimation(pan1);
//        pag.addAnimation(pan2);
//        pag.addAnimation(pan3);
//        pag.setLoopCount(5);
//        pag.start();

//        bubleB.show();
//    }
}
//-----------------------------------------------------------------------------------------
bool Bulbululator::event(QEvent *event)
{
    if(event->type() == QEvent::Close){
        //bubleB.close();
//        ThreadFreeCout pcout;
//        pcout <<"do close buble\n";
    }
    else if(event->type() == QEvent::MouseButtonDblClick){
        emit DoubleClicked(iTickerID);
    }
    else if(event->type() == QEvent::ContextMenu){
        //emit DoubleClicked(iTickerID);
//        ThreadFreeCout pcout;
//        pcout <<"context requested {"<<this->Text().toStdString() <<"}\n";

        QPoint point;

        emit ContextMenuRequested(iTickerID, point);

    }

    return QWidget::event(event);
}

//-----------------------------------------------------------------------------------------
Buble::Buble(QWidget *parent ): QWidget{parent,Qt::Window |Qt::FramelessWindowHint}
{
    setFocusPolicy(Qt::NoFocus);

//    QLabel * lbl= new QLabel();
//    QGridLayout *lt=new  QGridLayout();
//    lt->addWidget(lbl);
//    lt->setMargin(0);
//    setLayout(lt);
//    lbl->setMargin(0);

    //setFixedSize(15,15);
    setAttribute(Qt::WA_TranslucentBackground  );

    //| Qt::FramelessWindowHint| Qt::FramelessWindowHint
    //Qt::Window | Qt::FramelessWindowHint
    //lbl->setPixmap(QPixmap(":/store/images/sc_pie"));
}
//-----------------------------------------------------------------------------------------
Buble::~Buble()
{
}
//-----------------------------------------------------------------------------------------
void Buble::paintEvent(QPaintEvent */*event*/)
{
    QPainter pntr(this);

    pntr.setPen(QPen(Qt::black,1,Qt::SolidLine)); //Qt::DashLine
    pntr.setRenderHint(QPainter::Antialiasing,true);
    pntr.drawEllipse(0,0,this->width(),this->height());
}
