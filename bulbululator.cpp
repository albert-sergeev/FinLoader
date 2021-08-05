#include "bulbululator.h"
#include<QGridLayout>

Bulbululator::Bulbululator(QWidget *parent) : QWidget(parent)
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
