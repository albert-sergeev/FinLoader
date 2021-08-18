#include "graphviewform.h"
#include "ui_graphviewform.h"
#include "storage.h"

#include<QScrollBar>

#include<sstream>
#include<iomanip>

#include "threadfreelocaltime.h"
#include "threadfreecout.h"

//---------------------------------------------------------------------------------------------------------------
GraphViewForm::GraphViewForm(const int TickerID, std::vector<Ticker> &v, std::shared_ptr<GraphHolder> hldr, QWidget *parent) :
    QWidget(parent),    
    iTickerID{TickerID},
    tTicker{0,"","",1},
    vTickersLst{v},
    tStoredMiddlePointPosition{0},
    tStoredMinDate{0},
    tStoredMaxDate{0},
    iStoredMaxSize{0},
    dStoredLowMin{0},
    dStoredHighMax{0},
    ui(new Ui::GraphViewForm)
{
    ui->setupUi(this);

    QColor colorDarkGreen(0, 100, 52,50);
    QColor colorDarkRed(31, 53, 200,40);
    //-------------------------------------------------------------
    // for ticker
    //-------------------------------------------------------------
    QHBoxLayout *lt1 = new QHBoxLayout();
    lt1->setMargin(0);
    ui->wtCandle->setLayout(lt1);
    swtCandle = new StyledSwitcher(tr("OHLC "),tr(" Candle"),true,10,this);
    lt1->addWidget(swtCandle);
    swtCandle->SetOnColor(QPalette::Window,colorDarkGreen);
    swtCandle->SetOffColor(QPalette::Window,colorDarkRed);
    //-------------------------------------------------------------

//    btnScaleHViewPlus = new QPushButton("+",this);
//    btnScaleHViewPlus->setAttribute(Qt::WA_TranslucentBackground);

    btnScaleHViewPlus = new QLabel("+ -",this);
    btnScaleHViewPlus->setAttribute(Qt::WA_TranslucentBackground);


    //-------------------------------------------------------------
    holder = hldr;
    //
    auto It (std::find_if(vTickersLst.begin(),vTickersLst.end(),[&](const Ticker &t){
                return t.TickerID() == iTickerID;
                }));
    if(It == vTickersLst.end()){
        std::stringstream ss;
        ss<<"no ticker ID = "<<iTickerID<<"";
        throw std::invalid_argument(ss.str());
    }
    tTicker = (*It);
    this->setWindowTitle(QString::fromStdString(tTicker.TickerSign()));
    //------------------------------

    iSelectedInterval   = Bar::eInterval::pTick;
    iMaxGraphViewSize   = 0;
    tStoredMiddlePointPosition  = 0;
    dHScale             = 1;
    bOHLC               = true;

    mVScale[Bar::eInterval::pTick]      = 0;
    mVScale[Bar::eInterval::p1]         = 0;
    mVScale[Bar::eInterval::p5]         = 0;
    mVScale[Bar::eInterval::p10]        = 0;
    mVScale[Bar::eInterval::p15]        = 0;
    mVScale[Bar::eInterval::p30]        = 0;
    mVScale[Bar::eInterval::p60]        = 0;
    mVScale[Bar::eInterval::p120]       = 0;
    mVScale[Bar::eInterval::p180]       = 0;
    mVScale[Bar::eInterval::pDay]       = 0;
    mVScale[Bar::eInterval::pWeek]      = 0;
    mVScale[Bar::eInterval::pMonth]     = 0;




    connect(ui->btnTestLoad,SIGNAL(clicked()),this,SLOT(slotLoadGraphButton()));
    //------------------------------

    grScene = new QGraphicsScene();
    ui->grViewQuotes->setScene(grScene);

    ui->grViewQuotes->setContentsMargins(0,0,0,0);
    ui->grViewQuotes->setFrameStyle(QFrame::NoFrame);

    grSceneScaleUpper   = new QGraphicsScene();
    grSceneVolume       = new QGraphicsScene();
    ui->grViewScaleUpper->setScene(grSceneScaleUpper);
    ui->grViewScaleLower->setScene(grSceneScaleUpper);
    ui->grViewVolume->setScene(grSceneVolume);

    grSceneViewR1       = new QGraphicsScene();
    grSceneViewL1       = new QGraphicsScene();
    grSceneVertScroll   = new QGraphicsScene();

    ui->grViewR1->setScene(grSceneViewR1);
    ui->grViewL1->setScene(grSceneViewL1);
    ui->grVertScroll->setScene(grSceneVertScroll);



    connect(ui->grViewQuotes->scene(),SIGNAL(sceneRectChanged( const QRectF &)),this,SLOT(slotSceneRectChanged(const QRectF &)));
    connect(ui->grVertScroll->verticalScrollBar(),SIGNAL(valueChanged(int)),this,SLOT(slotVerticalScrollBarValueChanged(int)));
    connect(ui->grViewQuotes->verticalScrollBar(),SIGNAL(valueChanged(int)),this,SLOT(slotVerticalScrollBarValueChanged(int)));


    //connect(ui->grHorizontalScroll->horizontalScrollBar(),SIGNAL(valueChanged(int)),this,SLOT(slotHorisontalScrollBarValueChanged(int)));

    //----------------------------------------------------------------------------
    connect(ui->spbVScale,SIGNAL(valueChanged(double)),this,SLOT(slotVScaleValueChanged(double)));
    ui->spbHScale->setValue(dHScale);
    connect(ui->spbHScale,SIGNAL(valueChanged(double)),this,SLOT(slotHScaleValueChanged(double)));
    //----------------------------------------------------------------------------

    ui->horizontalScrollBar->setMinimum( BarGraphicsItem::BarWidth / dHScale);
    ui->horizontalScrollBar->setMaximum( BarGraphicsItem::BarWidth / dHScale);
    connect(ui->horizontalScrollBar,SIGNAL(valueChanged(int)),this,SLOT(slotSliderValueChanged(int)));

    //------------------------------
    std::time_t dtBegin{0};
    std::time_t dtEnd{0};
    bool bSuccess{false};

    {   // lock holder
        auto ItDefender(holder->beginIteratorByDate<BarTick>(iSelectedInterval,0,bSuccess));
        if (bSuccess){
            dtBegin = holder->getViewGraphDateMin(Bar::eInterval::pTick);
            dtEnd   = holder->getViewGraphDateMax(Bar::eInterval::pTick);
            ItDefender.ulock();
            if (dtBegin < dtEnd){
                slotInvalidateGraph(dtBegin, dtEnd);
            }
        }
    }
}
//---------------------------------------------------------------------------------------------------------------
GraphViewForm::~GraphViewForm()
{
    delete ui;
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::slotLoadGraphButton()
{
    std::time_t tNow =std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::time_t tBegin = Storage::dateAddMonth(tNow,-12);
    std::time_t tEnd = tNow;

    emit NeedLoadGraph(iTickerID, tBegin, tEnd);
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::slotInvalidateGraph(std::time_t dtBegin, std::time_t dtEnd, bool bNeedToRescale)
{
    bool bSuccess{false};
    queueRepaint.Push({dtBegin,dtEnd,bNeedToRescale});
    auto pData (queueRepaint.Pop(bSuccess));
    while(bSuccess)
    {
        auto data(*pData.get());
        //////
        if (iSelectedInterval == Bar::eInterval::pTick) {
            bSuccess = RepainInvalidRange<BarTick>(data);
        }
        else{
            bSuccess = RepainInvalidRange<Bar>(data);
        }
        ///////
        if(!bSuccess){ // if not, postpone for the future
            queueRepaint.Push({data.dtStart,data.dtEnd,data.bNeedToRescale});
        }
        else{   // if success, do next
            pData = queueRepaint.Pop(bSuccess);
        }
    }
}
//---------------------------------------------------------------------------------------------------------------
template<typename T>
bool GraphViewForm::RepainInvalidRange(RepainTask & data)
{
    bool bSuccess;
    auto It (holder->beginIteratorByDate<T>(iSelectedInterval,data.dtStart,bSuccess));
    if (bSuccess){
        auto ItEnd (holder->beginIteratorByDate<T>(iSelectedInterval,data.dtEnd,bSuccess));
        if (bSuccess){
            {   ThreadFreeCout pcout; pcout <<"do invalidate\n";}
            if (It == ItEnd)    return true;
            //////

            auto ItTotalEnd (holder->end<T>());
            size_t iSize =  holder->getViewGraphSize(iSelectedInterval);
            std::time_t tMinDate = holder->getViewGraphDateMin(Bar::eInterval::pTick);
            std::time_t tMaxDate =  holder->getViewGraphDateMax(Bar::eInterval::pTick);

            if (tStoredMinDate == 0 || tStoredMinDate > tMinDate)  tStoredMinDate = tMinDate;
            if (tStoredMaxDate == 0 || tStoredMaxDate < tMaxDate)  tStoredMaxDate = tMaxDate;

            auto p = holder->getMinMax(tStoredMinDate,tStoredMaxDate);
            double dLowMin  = p.first;
            double dHighMax = p.second;
            bool bNeedRescale {false};
            if (dStoredLowMin == 0  || dStoredLowMin  > dLowMin)    { dStoredLowMin = dLowMin;      }
            if (dStoredHighMax == 0 || dStoredHighMax < dHighMax)   { dStoredHighMax = dHighMax;    }
            if (mVScale.at(iSelectedInterval) == 0){
                bNeedRescale = true;
                if ((dStoredHighMax - dStoredLowMin) <= 0){
                    mVScale[iSelectedInterval] = 1.0;
                }
                else{
                    mVScale[iSelectedInterval] = (iViewPortHeight - iViewPortHighStrip - iViewPortLowStrip)/(dStoredHighMax - dStoredLowMin);
                }

                disconnect(ui->spbVScale,SIGNAL(valueChanged(double)),this,SLOT(slotVScaleValueChanged(double)));
                ui->spbVScale->setValue(mVScale[iSelectedInterval]);
                connect(ui->spbVScale,SIGNAL(valueChanged(double)),this,SLOT(slotVScaleValueChanged(double)));
            }

//            {
//                ThreadFreeCout pcout;
//                pcout <<"dLowMin: "<< dLowMin<<"\n";
//                pcout <<"dHighMax:"<< dHighMax<<"\n";
//                pcout <<"dStoredLowMin: "<< dStoredLowMin<<"\n";
//                pcout <<"dStoredHighMax: "<< dStoredHighMax<<"\n";
//                pcout <<"tMinDate: "<< threadfree_localtime_to_str(&tMinDate)<<"\n";
//                pcout <<"tMaxDate: "<< threadfree_localtime_to_str(&tMaxDate)<<"\n";

//            }


            // if neсessary clean all
            if (    tStoredMinDate == 0
                || tMinDate != tStoredMinDate
                || (iStoredMaxSize !=iSize && tMinDate >= tStoredMinDate && tMaxDate <= tStoredMaxDate)
                || bNeedRescale
                    ){

                size_t j=0;
                while (j < vShowedGraphicsBars.size()){
                       ui->grViewQuotes->scene()->removeItem(vShowedGraphicsBars[j]);
                       delete (vShowedGraphicsBars[j]);
                       vShowedGraphicsBars.erase(std::next(vShowedGraphicsBars.begin(),j));
                }
            }
            // clean invalid range
            size_t j=0;
            std::time_t tBeg = (It != ItTotalEnd) ? It->Period() : 0;
            //
            while (j < vShowedGraphicsBars.size()){
                if (vShowedGraphicsBars[j]->Period() >= tBeg &&
                    (ItEnd != ItTotalEnd && vShowedGraphicsBars[j]->Period() <= ItEnd->Period())
                        ){
                   ui->grViewQuotes->scene()->removeItem(vShowedGraphicsBars[j]);
                   delete (vShowedGraphicsBars[j]);
                   vShowedGraphicsBars.erase(std::next(vShowedGraphicsBars.begin(),j));
                }
                else{
                    ++j;
                }
            }
            // resize Scene rect
            if (iStoredMaxSize !=iSize || bNeedRescale || data.bNeedToRescale){

                int iNewViewPortH = (mVScale.at(iSelectedInterval) * (dStoredHighMax - dStoredLowMin)  + iViewPortLowStrip + iViewPortHighStrip );
                if ( iViewPortHeight > iNewViewPortH){
                    iNewViewPortH = iViewPortHeight;
                }
                QRectF newRec(0,-iNewViewPortH,
                              (iSize + iLeftShift + iRightShift) * BarGraphicsItem::BarWidth * dHScale  ,
                              iNewViewPortH
                              );

                grScene->setSceneRect(newRec);
                //ui->grViewQuotes->setSceneRect(newRec);
//                if (iStoredMaxSize)
//                    ui->grViewQuotes->setSceneRect(newRec);
//                else
//                    ui->grViewQuotes->scene()->addRect(newRec);


                int iNewSize = iSize + iLeftShift + iRightShift - ui->grViewQuotes->width()/(dHScale * BarGraphicsItem::BarWidth);
                iNewSize = iNewSize < 0? 0 : iNewSize;
                ui->horizontalScrollBar->setMaximum(iNewSize);
//                {
//                    ThreadFreeCout pcout;
//                    pcout <<"init scrollsize to:"<< iNewSize<<"\n";
//                }
            }

            iStoredMaxSize  = iSize;
            tStoredMinDate  = tMinDate;
            tStoredMaxDate  = tMaxDate;

//            //---------------------------
            bool bS{false};
            if (tStoredMiddlePointPosition != 0){
                if (iSelectedInterval == Bar::eInterval::pTick)
                    bS = RollSliderToMidTime<BarTick>(tStoredMiddlePointPosition);
                else
                    bS = RollSliderToMidTime<Bar>(tStoredMiddlePointPosition);
            }
            if (!bS){
                ui->horizontalScrollBar->setValue(iSize + iLeftShift + iRightShift - ui->grViewQuotes->width()/(dHScale * BarGraphicsItem::BarWidth));
            }
        }
    }
    return bSuccess;
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::slotLoadGraphButton2()
{

}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::slotSliderValueChanged(int iBeg)
{

    int iMiddle = iBeg + (ui->grViewQuotes->width()/(2 * dHScale * BarGraphicsItem::BarWidth));
    if (iSelectedInterval == Bar::eInterval::pTick){
        SliderValueChanged<BarTick>(iMiddle - iLeftShift);
    }
    else{
        SliderValueChanged<Bar>(iMiddle - iLeftShift);
    }
    SetSliderToPos(iMiddle);
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::SetSliderToPos(int iMiddlePos)
{
    ui->grViewQuotes->centerOn((iMiddlePos /*- iLeftShift*/)*BarGraphicsItem::BarWidth * dHScale,
                             ui->grViewQuotes->mapToScene(0,ui->grViewQuotes->viewport()->rect().center().ry()).y());
    //
    ui->grViewScaleUpper->centerOn(iMiddlePos*BarGraphicsItem::BarWidth * dHScale,
                             ui->grViewScaleUpper->mapToScene(0,ui->grViewScaleUpper->viewport()->rect().center().ry()).y());
    ui->grViewVolume->centerOn(iMiddlePos*BarGraphicsItem::BarWidth * dHScale,
                             ui->grViewVolume->mapToScene(0,ui->grViewVolume->viewport()->rect().center().ry()).y());
    ui->grViewScaleLower->centerOn(iMiddlePos*BarGraphicsItem::BarWidth * dHScale,
                             ui->grViewScaleLower->mapToScene(0,ui->grViewScaleLower->viewport()->rect().center().ry()).y());
}
//---------------------------------------------------------------------------------------------------------------
template<typename T>
bool GraphViewForm::RollSliderToMidTime(std::time_t tMidPos)
{
    bool bSuccess;
    auto It = holder->beginIteratorByDate<T>(iSelectedInterval,tMidPos,bSuccess);
    auto ItEnd (holder->end<T>());
    if (bSuccess && It != ItEnd){
        SliderValueChanged<T>(It.realPosition());
        SetSliderToPos(It.realPosition() + iLeftShift);
//        ui->grViewQuotes->centerOn((It.realPosition() + iLeftShift)*BarGraphicsItem::BarWidth * dHScale,
//                                 ui->grViewQuotes->mapToScene(0,ui->grViewQuotes->viewport()->rect().center().ry()).y());

    }
    return bSuccess;
}
//---------------------------------------------------------------------------------------------------------------
template<typename T>
void GraphViewForm::SliderValueChanged(int iMidPos)
{

    /////////////////////////////////////////////////////////////////////////////////////////
    bool bSuccess;
    auto ItDefender = holder->beginIteratorByDate<T>(iSelectedInterval,0,bSuccess);
    if (!bSuccess) return;
    //
    size_t iMaxSize = holder->getViewGraphSize(iSelectedInterval);
    if (iMaxSize ==0 ) return;
    /////////////////////////////////////////////////////////////////////////////////////////

    size_t iViewWidth = ui->grViewQuotes->width()/(dHScale *BarGraphicsItem::BarWidth);

    size_t iMid = iMidPos > 0 ? (size_t) iMidPos : 0;
    iMid = iMid < iMaxSize ? iMid : iMaxSize - 1;

    size_t iBeg      = iMid > iViewWidth  + iLeftShift ? iMid - iViewWidth  - iLeftShift : 0 ;
    size_t iEnd      = iMid + iViewWidth;

    DrawVertLines(iBeg - iLeftShift,iEnd);

    iEnd = iEnd < iMaxSize ? iEnd : iMaxSize - 1;

    /////////////////////////////////////////////////////////////////

    if (bSuccess && iMaxSize > 0){
        T tM = holder->getByIndex<T>(iSelectedInterval,iMid);
        T tB = holder->getByIndex<T>(iSelectedInterval,iBeg );
        T tE = holder->getByIndex<T>(iSelectedInterval,iEnd );

        tStoredMiddlePointPosition = tM.Period();
        ////////////

        size_t j=0;
        while (j < vShowedGraphicsBars.size()){
            if (vShowedGraphicsBars[j]->Period() < tB.Period() ||   // out of view range
                vShowedGraphicsBars[j]->Period() > tE.Period() ||   // out of view range
                vShowedGraphicsBars[j]->realPosition() >= iMaxSize  // leftovers from old fillings
                    ){

               ui->grViewQuotes->scene()->removeItem(vShowedGraphicsBars[j]);
               delete (vShowedGraphicsBars[j]);
               vShowedGraphicsBars.erase(std::next(vShowedGraphicsBars.begin(),j));

            }
            else{
                T t = holder->getByIndex<T>(iSelectedInterval, vShowedGraphicsBars[j]->realPosition());
                if (t.Period() != vShowedGraphicsBars[j]->Period()){ // leftovers from old fillings

                    ui->grViewQuotes->scene()->removeItem(vShowedGraphicsBars[j]);
                    delete (vShowedGraphicsBars[j]);
                    vShowedGraphicsBars.erase(std::next(vShowedGraphicsBars.begin(),j));

                }
                else{
                    ++j;
                }
            }
        }
        //
        ui->grViewQuotes->scene()->sceneRect(); //invalidate scene
        //
        auto It = holder->beginIteratorByDate<T>(iSelectedInterval,tB.Period(),bSuccess);
        auto ItEnd = holder->beginIteratorByDate<T>(iSelectedInterval,tE.Period()+1,bSuccess);
        auto ItTotalEnd = holder->end<T>();

        while(It != ItTotalEnd && It != ItEnd){
            auto ItFound = std::find_if(vShowedGraphicsBars.begin(),vShowedGraphicsBars.end(),[&](const BarGraphicsItem *p){
                if(p->Period() == It->Period() &&
                   p->realPosition() == It.realPosition()
                        ) {
                    return true;
                }
                else return false;});
            if (ItFound == vShowedGraphicsBars.end())
            {
                BarGraphicsItem *item = new BarGraphicsItem(*It,It.realPosition(),3);
                vShowedGraphicsBars.push_back(item);
                ui->grViewQuotes->scene()->addItem(item);
                item->setPos((It.realPosition() + iLeftShift) * BarGraphicsItem::BarWidth * dHScale , -realYtoViewPortY(It->Close()));
            }
            ++It;
        }
    }
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::resizeEvent(QResizeEvent *event)
{
    int iNewSize = iStoredMaxSize + iLeftShift + iRightShift - ui->grViewQuotes->width()/(dHScale * BarGraphicsItem::BarWidth);
    iNewSize = iNewSize < 0? 0 : iNewSize;
    ui->horizontalScrollBar->setMaximum(iNewSize);
    ///////////
    QPoint pR1= ui->grViewR1->pos();
    btnScaleHViewPlus->move(pR1.x()-30, pR1.y()+1 );

    QWidget::resizeEvent(event);
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::showEvent(QShowEvent *event)
{
    int iNewSize = iStoredMaxSize + iLeftShift + iRightShift - ui->grViewQuotes->width()/(dHScale * BarGraphicsItem::BarWidth);
    iNewSize = iNewSize < 0? 0 : iNewSize;
    ui->horizontalScrollBar->setMaximum(iNewSize);
    ///////////
    QPoint pR1= ui->grViewR1->pos();
    btnScaleHViewPlus->move(pR1.x()-30, pR1.y()+1 );

    QWidget::showEvent(event);
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::slotSceneRectChanged( const QRectF & rect)
{
//    {
//        ThreadFreeCout pcout;
//        pcout <<"rect changed to X:"<<rect.x()<<"\n";
//        pcout <<"rect changed to Y:"<<rect.y()<<"\n";
//    }

    QRectF newRecVR1 = QRectF(0,rect.y(),   ui->grViewR1->maximumWidth(),rect.height());
    QRectF newRecVL1 = QRectF(0,rect.y(),   ui->grViewL1->maximumWidth(),rect.height());
    QRectF newRecVScrl = QRectF(0,rect.y(), ui->grVertScroll->maximumWidth(),rect.height());
    ui->grViewR1->scene()->setSceneRect(newRecVR1);
    ui->grViewL1->scene()->setSceneRect(newRecVL1);
    ui->grVertScroll->scene()->setSceneRect(newRecVScrl);



    QRectF newRecScUp = QRectF(rect.x(),0,rect.width(), ui->grViewScaleUpper->maximumHeight());
    QRectF newRecVolume = QRectF(rect.x(),- ui->grViewVolume->maximumHeight(),rect.width(), ui->grViewVolume->maximumHeight());
    QRectF newRecScLow = QRectF(rect.x(),0,rect.width(), ui->grViewScaleLower->maximumHeight());


    ui->grViewScaleUpper->scene()->setSceneRect(newRecScUp);
    ui->grViewVolume->scene()->setSceneRect(newRecVolume);
    ui->grViewScaleLower->scene()->setSceneRect(newRecScLow);

}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::slotVerticalScrollBarValueChanged(int iV)
{
    ui->grViewL1->verticalScrollBar()->setValue(iV);
    ui->grViewR1->verticalScrollBar()->setValue(iV);
    ui->grViewQuotes->verticalScrollBar()->setValue(iV);
    ui->grVertScroll->verticalScrollBar()->setValue(iV);
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::slotVScaleValueChanged(double dScale)
{
    mVScale[iSelectedInterval] = dScale;
    //
    std::time_t tMinDate = holder->getViewGraphDateMin(Bar::eInterval::pTick);
    std::time_t tMaxDate =  holder->getViewGraphDateMax(Bar::eInterval::pTick);
    slotInvalidateGraph(tMinDate,tMaxDate,true);
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::slotVVolumeScaleValueChanged(double)
{
    //mVScale

}

//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::slotHScaleValueChanged(double dScale)
{
    dHScale = dScale;
    std::time_t tMinDate = holder->getViewGraphDateMin(Bar::eInterval::pTick);
    std::time_t tMaxDate =  holder->getViewGraphDateMax(Bar::eInterval::pTick);
    slotInvalidateGraph(tMinDate,tMaxDate,true);

    {
        ThreadFreeCout pcout;
        pcout <<"HScale changed: "<<dScale<<"\n";

    }

}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::DrawVertLines(int iBeg, int iEnd){

    if (iSelectedInterval == Bar::eInterval::pTick)
        DrawVertLinesT<BarTick>(iBeg,iEnd);
    else
        DrawVertLinesT<Bar>(iBeg,iEnd);

}
//---------------------------------------------------------------------------------------------------------------
template<typename T>
void GraphViewForm::DrawVertLinesT(int iBeg, int iEnd){
//    {
//        ThreadFreeCout pcout;
//        pcout <<"draw lines from:"<<iBeg<<"\n";
//        pcout <<"draw lines to:"<<iEnd<<"\n";
//    }
    //int iCount{0};

    EraseLinesUpper(mVLinesViewQuotes,  iBeg, ui->grViewQuotes->scene());
    EraseLinesUpper(mVLinesScaleUpper,  iBeg, ui->grViewScaleUpper->scene());
    EraseLinesUpper(mVLinesVolume,      iBeg, ui->grViewVolume->scene());

    EraseLinesLower(mVLinesViewQuotes,  iEnd, ui->grViewQuotes->scene());
    EraseLinesLower(mVLinesScaleUpper,  iEnd, ui->grViewScaleUpper->scene());
    EraseLinesLower(mVLinesVolume,      iEnd, ui->grViewVolume->scene());

    QPen blackSolidPen(Qt::black,0.5,Qt::SolidLine);
    QPen blackDashPen(Qt::gray,1,Qt::DashLine);
    QPen blackDotPen(Qt::gray,1,Qt::DotLine);
    QPen simpleDashPen(Qt::gray,1,Qt::DashLine);


    QFont fontTime;
    fontTime.setPixelSize(11);
    fontTime.setBold(false);
    fontTime.setFamily("Times New Roman");

    QFont fontNumb;
    fontNumb.setPixelSize(14);
    fontNumb.setBold(false);
    fontNumb.setFamily("Times New Roman");

    int iLineH = ui->grViewScaleUpper->scene()->sceneRect().height()/2;
    static const int iShiftH {4};
    //////////////////////////////////////////////////////////////
    qreal xCur{0};
    qreal xPre{0};
    int iFCount{0};
    int iDayCounter{0};
    int iSecCounter{0};
    std::time_t tTmp{0};

    std::tm tmPre = *threadfree_localtime(&tTmp);
    std::tm tmCur = tmPre;

    bool bFifstLine{true};
    int iSizeMax =  holder->getViewGraphSize(iSelectedInterval);
    tmCur = *threadfree_localtime(&tTmp);

    QRectF rectQuotes =  ui->grViewQuotes->scene()->sceneRect();
    QRectF rectVolume =  ui->grViewVolume->scene()->sceneRect();
    /////
    for (int i = iBeg; i<= iEnd; ++i){
        auto ItSL = mVLinesScaleUpper.find(i);
        if (ItSL == mVLinesScaleUpper.end()){
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            xCur = (i + iLeftShift)     * BarGraphicsItem::BarWidth * dHScale;
            xPre = (i - 1 + iLeftShift) * BarGraphicsItem::BarWidth * dHScale;
            //if (iLeftShift <= i && i - iLeftShift < iSizeMax){
            if ( i > 0 && i  < iSizeMax){
                //----------------
                T & tM = holder->getByIndex<T>(iSelectedInterval,i );
                tTmp = tM.Period();
                tmCur = *threadfree_localtime(&tTmp);
                //------------------------------------------------------------------------
                // draw small lines down under :)
                if (!bFifstLine && iFCount < 3){
                    DrawLineToScene(i, xCur,0,xCur,1, mVLinesScaleUpper, ui->grViewScaleUpper->scene(),blackSolidPen,tTmp,true);
                }
                else{
                    DrawLineToScene(i, xCur,0,xCur,5, mVLinesScaleUpper, ui->grViewScaleUpper->scene(),blackSolidPen,tTmp,true);
                }
                //------------------------------------------------------------------------
                //  draw year line
                if(!bFifstLine
                        && tmPre.tm_year != tmCur.tm_year){
                    DrawLineToScene(i, xCur, 0,xCur,iLineH, mVLinesScaleUpper, ui->grViewScaleUpper->scene(),blackSolidPen,tTmp,true); // down line

                    DrawLineToScene(i, xCur, -iShiftH ,xCur, - rectVolume.height() + iShiftH * 2, mVLinesVolume, ui->grViewVolume ->scene(),blackDashPen,tTmp,true); // volume line
                    DrawLineToScene(i, xCur, -iShiftH ,xCur, - rectQuotes.height() + iShiftH * 2, mVLinesViewQuotes, ui->grViewQuotes->scene(),blackDashPen,tTmp,true); // middle line
                    //TODO: print the year
                    iDayCounter = 0;
                    DrawIntToScene(i, xCur, iLineH ,tmCur.tm_year + 1900,mVLinesScaleUpper, ui->grViewScaleUpper->scene(), fontNumb);
                }
                //  draw month line
                else if(!bFifstLine
                        && tmPre.tm_mon != tmCur.tm_mon
                        && iSelectedInterval != Bar::eInterval::pMonth // for months don't draw
                        && (iSelectedInterval != Bar::eInterval::pWeek || tmCur.tm_mon % 3 ==0) // for weeks  - only draw only for every third
                        ){
                    DrawLineToScene(i, xCur, 0,xCur,iLineH, mVLinesScaleUpper, ui->grViewScaleUpper->scene(),blackSolidPen,tTmp,true); // down line
                    //
                    DrawLineToScene(i, xCur, -iShiftH ,xCur, - rectVolume.height() + iShiftH * 2, mVLinesVolume, ui->grViewVolume ->scene(),blackDashPen,tTmp,true); // volume line
                    DrawLineToScene(i, xCur, -iShiftH ,xCur, - rectQuotes.height() + iShiftH * 2, mVLinesViewQuotes, ui->grViewQuotes->scene(),blackDashPen,tTmp,true); // middle line
                    //TODO: print the month
                    iDayCounter = 0;
                    DrawIntToScene(i, xCur, iLineH ,tmCur.tm_mon + 1 ,mVLinesScaleUpper, ui->grViewScaleUpper->scene(), fontNumb);
                }
                //  draw day line
                else if(!bFifstLine
                        && (tmPre.tm_mday != tmCur.tm_mday || tmPre.tm_mon != tmCur.tm_mon || tmPre.tm_year != tmCur.tm_year)
                        && ((iSelectedInterval == Bar::eInterval::pDay && iDayCounter % 8 == 0 ) ||
                            (iSelectedInterval == Bar::eInterval::p180 && iDayCounter % 4 == 0 ) ||
                            (iSelectedInterval == Bar::eInterval::p120 && iDayCounter % 4 == 0 )
                            )
                        ){
                    DrawLineToScene(i, xCur, -iShiftH ,xCur, - rectVolume.height() + iShiftH * 2, mVLinesVolume, ui->grViewVolume ->scene(),blackDashPen,tTmp,true); // volume line
                    DrawLineToScene(i, xCur, -iShiftH ,xCur, - rectQuotes.height() + iShiftH * 2, mVLinesViewQuotes, ui->grViewQuotes->scene(),blackDashPen,tTmp,true); // middle line
                    //TODO: print the day
                    if (iFCount > 3){
                        DrawIntToScene(i, xCur, -1 ,tmCur.tm_mday,mVLinesScaleUpper, ui->grViewScaleUpper->scene(), fontNumb);
                        iFCount = 0;
                    }
                    iFCount = 0;
                }
                //  draw interday
                else if(!bFifstLine
                        && (!(tmPre.tm_mday != tmCur.tm_mday || tmPre.tm_mon != tmCur.tm_mon || tmPre.tm_year != tmCur.tm_year))
                        && ((iSelectedInterval == Bar::eInterval::p1  && tmCur.tm_min % 15 == 0 ) ||
                            (iSelectedInterval == Bar::eInterval::p5  && tmCur.tm_min  == 0 ) ||
                            (iSelectedInterval == Bar::eInterval::p10 && tmCur.tm_min == 0 && tmCur.tm_hour % 2 == 0 ) ||
                            (iSelectedInterval == Bar::eInterval::p15 && tmCur.tm_min == 0 && tmCur.tm_hour % 2 == 0 )
                            )
                        ){
                    DrawLineToScene(i, xCur, -iShiftH ,xCur, - rectVolume.height() + iShiftH * 2, mVLinesVolume, ui->grViewVolume ->scene(),blackDashPen,tTmp,true); // volume line
                    DrawLineToScene(i, xCur, -iShiftH ,xCur, - rectQuotes.height() + iShiftH * 2, mVLinesViewQuotes, ui->grViewQuotes->scene(),blackDashPen,tTmp,true); // middle line
                    //TODO: print the interday time
                    if (iFCount > 4){
                        DrawTimeToScene(i, xCur, -1 ,tmCur,mVLinesScaleUpper, ui->grViewScaleUpper->scene(), fontTime);
                        iFCount = -2;
                    }
                }
                //  draw ticks
                else if(!bFifstLine
                        && iSelectedInterval == Bar::eInterval::pTick && (tmPre.tm_min != tmCur.tm_min)
                        ){

                    //
                    DrawLineToScene(i, xCur, -iShiftH ,xCur, - rectVolume.height() + iShiftH * 2, mVLinesVolume, ui->grViewVolume ->scene(),blackDashPen,tTmp,true); // volume line
                    DrawLineToScene(i, xCur, -iShiftH ,xCur, - rectQuotes.height() + iShiftH * 2, mVLinesViewQuotes, ui->grViewQuotes->scene(),blackDashPen,tTmp,true); // middle line
                    //TODO: print the interday time

                    if (iFCount > 4){
                        DrawTimeToScene(i, xCur, -1 ,tmCur,mVLinesScaleUpper, ui->grViewScaleUpper->scene(), fontTime);
                        iFCount = -2;
                    }

                    iSecCounter = 0;
                }
                else if(!bFifstLine
                        && iSelectedInterval == Bar::eInterval::pTick
                        && (tmPre.tm_sec != tmCur.tm_sec /*&& iSecCounter >=8 */)
                        ){
                    //
                    DrawLineToScene(i, xCur, -iShiftH ,xCur, - rectQuotes.height() + iShiftH * 2, mVLinesViewQuotes, ui->grViewQuotes->scene(),blackDotPen,tTmp,true); // middle line
                    iSecCounter = 0;
                }
                ///
                bFifstLine = false;
            }
            else{
                DrawLineToScene(i, xCur,0,xCur,5, mVLinesScaleUpper, ui->grViewScaleUpper->scene(),blackSolidPen);
            }
            //------------------------------------------------------------------------
            // draw horisontal line in ScaleViews
            //DrawLineToScene(i, xCur,0,xPre,0, mVLinesScaleUpper, ui->grViewScaleUpper->scene(),blackSolidPen,0);
            DrawLineToScene(i, xCur,iLineH,xPre,iLineH, mVLinesScaleUpper, ui->grViewScaleUpper->scene(),blackSolidPen,0);
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            iFCount++;
            iDayCounter++;
            iSecCounter++;
            tmPre = tmCur;
        }
    }
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::DrawIntToScene(const int idx,const  qreal x,const  qreal y,const  int n,
                                    std::map<int,std::vector<QGraphicsItem *>>& mM, QGraphicsScene *scene, const QFont & font)
{
     QGraphicsTextItem * item = new QGraphicsTextItem(QString::number(n));
     item->setFont(font);
     mM[idx].push_back(item);
     scene->addItem(item);
     item->setPos(x,y);

}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::DrawTimeToScene(const int idx,const  qreal x,const  qreal y,const  std::tm & tmT,
                                    std::map<int,std::vector<QGraphicsItem *>>& mM, QGraphicsScene *scene, const QFont & font)
{
   std::stringstream ss;
   ss <<std::setfill('0');
   ss << std::setw(2)<<tmT.tm_hour<<":";
   ss <<":";
   ss << std::setw(2)<<tmT.tm_min;

    QGraphicsTextItem * item = new QGraphicsTextItem(QString::fromStdString(ss.str()));
    item->setFont(font);
    mM[idx].push_back(item);
    scene->addItem(item);
    item->setPos(x,y);

}

//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::DrawLineToScene(const int idx,const  qreal x1,const  qreal y1,const qreal x2,const  qreal y2,
                                    std::map<int,std::vector<QGraphicsItem *>>& mM, QGraphicsScene *scene, const QPen & pen, const  std::time_t t, bool bHasTooltip)
{
    QGraphicsLineItem *item = new QGraphicsLineItem(0,0,x2-x1,y2-y1);
    item->setPen(pen);
    mM[idx].push_back(item);
    scene->addItem(item);
    item->setPos(x1,y1);

    if (bHasTooltip && t != 0){
        std::stringstream ss;
        ss <<threadfree_localtime_to_str(&t);
        item->setToolTip(QString::fromStdString(ss.str()));
    }
}

//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::EraseLinesLower(std::map<int,std::vector<QGraphicsItem *>>& mM, int iStart, QGraphicsScene * scene)
{
    auto It (mM.lower_bound(iStart));
    auto ItCp(It);
    while(It != mM.end()){
        for (auto &t:It->second){
            scene->removeItem(t);
            delete t;
            t = nullptr;
        }
        It->second.clear();
        It++;
    }
    mM.erase(ItCp,mM.end());
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::EraseLinesUpper(std::map<int,std::vector<QGraphicsItem *>>& mM, int iEnd, QGraphicsScene *scene)
{
    auto ItEnd (mM.upper_bound(iEnd));
    auto It(mM.begin());
    while(It != ItEnd){
        for (auto &t:It->second){
            scene->removeItem(t);
            delete t;
            t = nullptr;
        }
        It->second.clear();
        It++;
    }
    mM.erase(mM.begin(),ItEnd);
}
//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------

