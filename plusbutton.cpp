#include "plusbutton.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QPainter>

//PlusButton::PlusButton(bool Plus, QWidget *parent) : QLabel(parent),bPlus{Plus}
PlusButton::PlusButton(bool Plus, QWidget *parent) : QWidget(parent),bPlus{Plus}
{
    //this->setText("+BOBOMHOIM:UGHMLIJMYIMGOLHLKGUM");
    setFixedSize(15,15);
    setAttribute(Qt::WA_TranslucentBackground);
//    QLabel *lbl =new QLabel("ZZZ");
//    QHBoxLayout *lt = new QHBoxLayout();
//    lt->addWidget(lbl);
//    this->setLayout(lt);

}

void PlusButton::paintEvent(QPaintEvent */*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    //QPen blackSolidPen(Qt::black,1,Qt::SolidLine);

    //QPen circlePen(Qt::gray,0.5,Qt::SolidLine);
    //QPen circlePen(Qt::black,0.5,Qt::SolidLine);
    //QPen circlePen(Qt::black,1,Qt::SolidLine);
    QPen circlePen(Qt::gray,1,Qt::SolidLine);

    const static int iR{3};
    const static int iC{7};

    if (bPlus){
        if (!bPushed){
           // painter.setPen(blackSolidPen);

            painter.drawLine(iC     , iC - iR, iC       , iC + iR);
            painter.drawLine(iC - iR, iC     , iC + iR  , iC);

            painter.setPen(circlePen);

            painter.drawEllipse(iC - iR - 3 , iC - iR - 3,iR*4,iR*4);
        }
        else{
            //painter.setPen(blackSolidPen);

            painter.drawLine(iC     + 1, iC - iR+ 1, iC       + 1, iC + iR + 1);
            painter.drawLine(iC - iR+ 1, iC     + 1, iC + iR  + 1, iC + 1);

            painter.setPen(circlePen);

            painter.drawEllipse(iC - iR - 2 , iC - iR - 2,iR*4,iR*4);
        }
    }
    else{
        if (!bPushed){
            //painter.setPen(blackSolidPen);
            painter.drawLine(iC - iR, iC     , iC + iR  , iC);
            painter.setPen(circlePen);
            painter.drawEllipse(iC - iR - 3 , iC - iR - 3,iR*4,iR*4);
        }
        else{
            //painter.setPen(blackSolidPen);
            painter.drawLine(iC - iR+ 1, iC     + 1, iC + iR  + 1, iC + 1);
            painter.setPen(circlePen);
            painter.drawEllipse(iC - iR - 2 , iC - iR - 2,iR*4,iR*4);
        }
    }

}

void PlusButton::mousePressEvent(QMouseEvent */*event*/){
    bPushed = true;
    this->repaint(0,0,15,15);
}
void PlusButton::mouseReleaseEvent(QMouseEvent */*event*/){
    bPushed = false;
    this->repaint(0,0,15,15);
    emit clicked();
    emit clicked(bPlus);
}
