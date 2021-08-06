#include "bulbululator.h"
#include<QGridLayout>
#include<QPropertyAnimation>




#include "threadfreecout.h"

//-----------------------------------------------------------------------------------------
Bulbululator::Bulbululator(QWidget *parent) : QWidget(parent),buble{nullptr}
{


    /////////////////////////////////////////
    QGridLayout *lt=new QGridLayout();
    lbl = new QLabel("ABBA");
    lt->addWidget(lbl);
    lbl->setMargin(0);
    lt->setMargin(0);
    this->setLayout(lt);

    //lbl->setFrameShape(QFrame::Panel);
    lbl->setFrameShape(QFrame::StyledPanel);

//    QFont fnt ( lbl->font());

//    //fnt.setWeight(15);

//    lbl->setFont(fnt);

    /////////////////////////////////
    dtStartBlink = std::chrono::steady_clock::now();
    bBlink = false;
    defPallete = lbl->palette();
    /////////////////////////////////

    this->startTimer(100);
}
//-----------------------------------------------------------------------------------------
Bulbululator::~Bulbululator()
{
    if (buble != nullptr){
        buble->close();
        delete  buble;
    }
}
//-----------------------------------------------------------------------------------------
void Bulbululator::timerEvent(QTimerEvent * /*event*/)
{

    if (bBlink){
        std::chrono::time_point dtNow(std::chrono::steady_clock::now());
        milliseconds tCount = dtNow - dtStartBlink;

        if (tCount.count()>100){
            bBlink = false;
            lbl->setPalette(defPallete);
            dtStartBlink = std::chrono::steady_clock::now();
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
        QColor colorDarkGreen(0, 59, 56, 128);

        QPalette p = this->palette();
        //p.setColor(QPalette::WindowText,Qt::green); // text
        //p.setColor(QPalette::Window,Qt::green);
        p.setColor(QPalette::Window,colorDarkGreen);
        lbl->setAutoFillBackground(true);
        lbl->setPalette(p);
    }


    if (buble == nullptr){
//        //buble = new QLabel();
//        buble = new QWidget();
//        QLabel * lbl= new QLabel();
//        QGridLayout *lt=new  QGridLayout();
//        lt->addWidget(lbl);
//        lt->setMargin(0);
//        buble->setLayout(lt);
//        lbl->setMargin(0);

//        buble->setAttribute(Qt::WA_TranslucentBackground);

//        lbl->setPixmap(QPixmap(":/store/images/sc_pie"));

//        ///////////////////////////////////////////
//        QPropertyAnimation *pan1=new QPropertyAnimation(buble,"geometry");
//        pan1->setDuration(3000);
//        pan1->setStartValue(QRect(120,0,100,100));
//        //pan1->setStartValue(QRect(480,380,200,200));
//        pan1->setEndValue(QRect(480,380,200,200));
//        pan1->setEasingCurve(QEasingCurve::InOutBounce);

//        QPropertyAnimation *pan2=new QPropertyAnimation(buble,"pos");
//        pan2->setDuration(3000);
//        pan2->setStartValue(QPoint(240,480));
//        pan2->setEndValue(QPoint(240,0));
//        //pan2->setEasingCurve(QEasingCurve::InOutExpo);
//        pan2->setEasingCurve(QEasingCurve::OutBounce);


//        pag.addAnimation(pan1);
//        pag.addAnimation(pan2);
//        pag.setLoopCount(3);
//        pag.start();

//        buble->show();
    }
}
//-----------------------------------------------------------------------------------------
