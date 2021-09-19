#include "transparentbutton.h"
#include "threadfreecout.h"

#include<QPainter>

//----------------------------------------------------------------------------------------------------------------------------
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
TransparentButton::TransparentButton(QString Text, QString TextAlternate, bool State, QWidget *parent) : QWidget(parent),bPushed{false}
{
    strText             = Text;
    strTextAlternate    = TextAlternate;
    bState              = State;
    modeMode            = eMode::CheckBox;

    iWidth  = 20;
    iHeight = 20;
    setFixedSize(iWidth, iHeight);

    setAttribute(Qt::WA_TranslucentBackground);
}
//----------------------------------------------------------------------------------------------------------------------------
void TransparentButton::showEvent(QShowEvent */*event*/)
{
        ;
}
//----------------------------------------------------------------------------------------------------------------------------
void TransparentButton::paintEvent(QPaintEvent */*event*/)
{
    QPainter painter(this);

    painter.save();

    painter.setRenderHint(QPainter::Antialiasing, true);

    QPen blackSolidPen(Qt::black,1,Qt::SolidLine);

    QPen grayPen(Qt::gray,0.5,Qt::SolidLine);
    //QPen circlePen(Qt::black,0.5,Qt::SolidLine);
    //QPen circlePen(Qt::black,1,Qt::SolidLine);
    QPen circlePen(Qt::gray,1,Qt::SolidLine);

    QFont font;
    font.setPixelSize(12);
    font.setBold(false);
    font.setFamily("Times New Roman");
    painter.setFont(font);


    QRectF rectangle{0,0,1,1};
    QRectF boundRect;


    QString sTxt;


    if (modeMode != eMode::CheckBox || bState ){
        sTxt = strText;
    }
    else{
        sTxt = strTextAlternate;
    }

    boundRect = painter.boundingRect(rectangle, Qt::AlignmentFlag::AlignLeft | Qt::AlignmentFlag::AlignTop, sTxt);

    QRectF drawRect;
    QRectF drawTextRect;
    if (!bPushed){
        drawRect     = QRectF {1,1,boundRect.width() + 4,boundRect.height() + 1};
        drawTextRect = QRectF {3,2,boundRect.width(),boundRect.height()};
    }
    else{
        drawRect     = QRectF {2,2,boundRect.width() + 4,boundRect.height() + 1};
        drawTextRect = QRectF {4,3,boundRect.width(),boundRect.height()};
    }

    iWidth  = (((int)boundRect.width()) > 0  ? (int)boundRect.width()  + 1 : int (boundRect.width()))  + 8;
    iHeight = (((int)boundRect.height()) > 0 ? (int)boundRect.height() + 1 : int (boundRect.height())) + 4;

    QRectF resizeRect   {0,0,(qreal)iWidth,(qreal)iHeight};


    setFixedSize(resizeRect.width(), resizeRect.height());

    painter.setPen(blackSolidPen);
    painter.drawText(drawTextRect, Qt::AlignmentFlag::AlignLeft | Qt::AlignmentFlag::AlignTop, sTxt);
    painter.setPen(grayPen);
    painter.drawRect(drawRect);

    painter.restore();

}
//----------------------------------------------------------------------------------------------------------------------------
void TransparentButton::mousePressEvent(QMouseEvent */*event*/){
    bPushed = true;
    this->repaint(0,0,iWidth,iHeight);
}
//----------------------------------------------------------------------------------------------------------------------------
void TransparentButton::mouseReleaseEvent(QMouseEvent */*event*/){
    bPushed = false;

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


