#include "bulbululator.h"
#include<QGridLayout>
#include<QPropertyAnimation>

//-----------------------------------------------------------------------------------------
Bulbululator::Bulbululator(QWidget *parent) : QWidget(parent),buble{nullptr}
{
    QGridLayout *lt=new QGridLayout();
    lbl = new QLabel("ABBA");
    lt->addWidget(lbl);
    lbl->setMargin(0);
    lt->setMargin(0);
    this->setLayout(lt);

    //lbl->setFrameShape(QFrame::Panel);
    lbl->setFrameShape(QFrame::StyledPanel);

    QFont fnt ( lbl->font());

    //fnt.setWeight(15);

    lbl->setFont(fnt);
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
void Bulbululator::Bubble()
{
    if (buble == nullptr){
        buble = new QWidget();

        QPropertyAnimation *pan1=new QPropertyAnimation(buble,"geometry");
        pan1->setDuration(3000);
        pan1->setStartValue(QRect(120,0,100,100));
        //pan1->setStartValue(QRect(480,380,200,200));
        pan1->setEndValue(QRect(480,380,200,200));
        pan1->setEasingCurve(QEasingCurve::InOutBounce);

        QPropertyAnimation *pan2=new QPropertyAnimation(buble,"pos");
        pan2->setDuration(3000);
        pan2->setStartValue(QPoint(240,480));
        pan2->setEndValue(QPoint(240,0));
        //pan2->setEasingCurve(QEasingCurve::InOutExpo);
        pan2->setEasingCurve(QEasingCurve::OutBounce);


        pag.addAnimation(pan1);
        pag.addAnimation(pan2);
        pag.setLoopCount(3);
        pag.start();

        buble->show();
    }
}
//-----------------------------------------------------------------------------------------
