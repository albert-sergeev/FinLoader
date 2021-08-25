#include "graphviewform.h"
#include "ui_graphviewform.h"
#include "storage.h"

#include<QScrollBar>

#include<sstream>
#include<iomanip>
#include<cmath>

#include "threadfreelocaltime.h"
#include "threadfreecout.h"
#include "plusbutton.h"

//---------------------------------------------------------------------------------------------------------------
GraphViewForm::GraphViewForm(const int TickerID, std::vector<Ticker> &v, std::shared_ptr<GraphHolder> hldr, QWidget *parent) :
    QWidget(parent),    
    iTickerID{TickerID},
    tTicker{0,"","",1},
    vTickersLst{v},
    tStoredRightPointPosition{0},
    iStoredRightAggregate{0},
    tStoredMinDate{0},
    tStoredMaxDate{0},
    iStoredMaxSize{0},
    dStoredLowMin{0},
    dStoredHighMax{0},
    dStoredVolumeLowMin{0},
    dStoredVolumeHighMax{0},
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

    btnScaleHViewPlus       = new PlusButton(true,this);
    btnScaleHViewMinus      = new PlusButton(false,this);

    btnScaleVViewPlus       = new PlusButton(true,this);
    btnScaleVViewMinus      = new PlusButton(false,this);

    btnScaleVVolumePlus     = new PlusButton(true,this);
    btnScaleVVolumeMinus    = new PlusButton(false,this);


    connect(btnScaleHViewPlus,SIGNAL(clicked(bool)),this,SLOT(slotHScaleQuotesClicked(bool)));
    connect(btnScaleHViewMinus,SIGNAL(clicked(bool)),this,SLOT(slotHScaleQuotesClicked(bool)));

    connect(btnScaleVViewPlus,SIGNAL(clicked(bool)),this,SLOT(slotVScaleQuotesClicked(bool)));
    connect(btnScaleVViewMinus,SIGNAL(clicked(bool)),this,SLOT(slotVScaleQuotesClicked(bool)));

    connect(btnScaleVVolumePlus,SIGNAL(clicked(bool)),this,SLOT(slotHScaleVolumeClicked(bool)));
    connect(btnScaleVVolumeMinus,SIGNAL(clicked(bool)),this,SLOT(slotHScaleVolumeClicked(bool)));


    btnScaleHViewPlus->setToolTip(tr("increase the horizontal scale of the graph"));
    btnScaleHViewMinus->setToolTip(tr("reduce the horizontal scale of the graph"));

    btnScaleVViewPlus->setToolTip(tr("increase the vertical scale of the graph"));
    btnScaleVViewMinus->setToolTip(tr("reduce the vertical scale of the graph"));

    btnScaleVVolumePlus->setToolTip(tr("increase the vertical scale of the volume graph"));
    btnScaleVVolumeMinus->setToolTip(tr("reduce the vertical scale of the volume graph"));

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
    // TODO: save default selected interval other settings

    iSelectedInterval   = Bar::eInterval::pTick;
    iMaxGraphViewSize   = 0;
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

    mVVolumeScale[Bar::eInterval::pTick]      = 0;
    mVVolumeScale[Bar::eInterval::p1]         = 0;
    mVVolumeScale[Bar::eInterval::p5]         = 0;
    mVVolumeScale[Bar::eInterval::p10]        = 0;
    mVVolumeScale[Bar::eInterval::p15]        = 0;
    mVVolumeScale[Bar::eInterval::p30]        = 0;
    mVVolumeScale[Bar::eInterval::p60]        = 0;
    mVVolumeScale[Bar::eInterval::p120]       = 0;
    mVVolumeScale[Bar::eInterval::p180]       = 0;
    mVVolumeScale[Bar::eInterval::pDay]       = 0;
    mVVolumeScale[Bar::eInterval::pWeek]      = 0;
    mVVolumeScale[Bar::eInterval::pMonth]     = 0;

    //------------------------------

    grScene = new QGraphicsScene();
    ui->grViewQuotes->setScene(grScene);

    ui->grViewQuotes->setContentsMargins(0,0,0,0);
    ui->grViewQuotes->setFrameStyle(QFrame::NoFrame);

    grSceneScaleUpper   = new QGraphicsScene();
    grSceneVolume       = new QGraphicsScene();
    grSceneHorizScroll  = new QGraphicsScene();
    ui->grViewScaleUpper->setScene(grSceneScaleUpper);
    ui->grViewScaleLower->setScene(grSceneScaleUpper);
    ui->grViewVolume->setScene(grSceneVolume);
    ui->grHorizScroll->setScene(grSceneHorizScroll);

    grSceneViewR1       = new QGraphicsScene();
    grSceneViewL1       = new QGraphicsScene();
    grSceneVertScroll   = new QGraphicsScene();

    ui->grViewR1->setScene(grSceneViewR1);
    ui->grViewL1->setScene(grSceneViewL1);
    ui->grVertScroll->setScene(grSceneVertScroll);


//    ui->grViewQuotes->setRenderHint(QPainter::Antialiasing);
//    ui->grViewScaleUpper->setRenderHint(QPainter::Antialiasing);
//    ui->grViewVolume->setRenderHint(QPainter::Antialiasing);
//    ui->grViewR1->setRenderHint(QPainter::Antialiasing);
//    ui->grViewL1->setRenderHint(QPainter::Antialiasing);




    //painter->setRenderHint(QPainter::Antialiasing, true);



    connect(ui->grViewQuotes->scene(),SIGNAL(sceneRectChanged( const QRectF &)),this,SLOT(slotSceneRectChanged(const QRectF &)));

    connect(ui->grVertScroll->verticalScrollBar()   ,SIGNAL(valueChanged(int)),this,SLOT(slotVerticalScrollBarValueChanged(int)));
    connect(ui->grViewQuotes->verticalScrollBar()   ,SIGNAL(valueChanged(int)),this,SLOT(slotVerticalScrollBarValueChanged(int)));
    connect(ui->grViewL1->verticalScrollBar()       ,SIGNAL(valueChanged(int)),this,SLOT(slotVerticalScrollBarValueChanged(int)));
    connect(ui->grViewR1->verticalScrollBar()       ,SIGNAL(valueChanged(int)),this,SLOT(slotVerticalScrollBarValueChanged(int)));


    connect(ui->grHorizScroll->horizontalScrollBar(),SIGNAL(valueChanged(int)),this,SLOT(slotHorizontalScrollBarValueChanged(int)));

    //----------------------------------------------------------------------------
    connect(ui->dtBeginDate,SIGNAL(dateTimeChanged(const QDateTime&)),this,SLOT(dateTimeBeginChanged(const QDateTime&)));
    connect(ui->dtEndDate,SIGNAL(dateTimeChanged(const QDateTime&)),this,SLOT(dateTimeEndChanged(const QDateTime&)));


    //----------------------------------------------------------------------------

//    ui->horizontalScrollBar->setMinimum( 0);
//    ui->horizontalScrollBar->setMaximum( 0);
//    connect(ui->horizontalScrollBar,SIGNAL(valueChanged(int)),this,SLOT(slotSliderValueChanged(int)));

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

    SetSelectedIntervalToControls();
    connect(ui->btnTick, SIGNAL(clicked()),this,SLOT(slotPeriodButtonChanged()));
    connect(ui->btn1,    SIGNAL(clicked()),this,SLOT(slotPeriodButtonChanged()));
    connect(ui->btn5,    SIGNAL(clicked()),this,SLOT(slotPeriodButtonChanged()));
    connect(ui->btn10,   SIGNAL(clicked()),this,SLOT(slotPeriodButtonChanged()));
    connect(ui->btn15,   SIGNAL(clicked()),this,SLOT(slotPeriodButtonChanged()));
    connect(ui->btn30,   SIGNAL(clicked()),this,SLOT(slotPeriodButtonChanged()));
    connect(ui->btn60,   SIGNAL(clicked()),this,SLOT(slotPeriodButtonChanged()));
    connect(ui->btn120,  SIGNAL(clicked()),this,SLOT(slotPeriodButtonChanged()));
    connect(ui->btn180,  SIGNAL(clicked()),this,SLOT(slotPeriodButtonChanged()));
    connect(ui->btnDay,  SIGNAL(clicked()),this,SLOT(slotPeriodButtonChanged()));
    connect(ui->btnWeek, SIGNAL(clicked()),this,SLOT(slotPeriodButtonChanged()));
    connect(ui->btnMonth,SIGNAL(clicked()),this,SLOT(slotPeriodButtonChanged()));

   // {ThreadFreeCout pcout; pcout<<"const out\n";}

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
        //{ThreadFreeCout pcout; pcout<<"begin repaint invalid\n";}
        auto data(*pData.get());
        //////
        if (iSelectedInterval == Bar::eInterval::pTick) {
            bSuccess = RepainInvalidRange<BarTick>(data);
        }
        else{
            bSuccess = RepainInvalidRange<Bar>(data);
        }
        //{ThreadFreeCout pcout; pcout<<"end repaint invalid\n";}
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
            /// \brief ItTotalEnd
            ///{ThreadFreeCout pcout; pcout<<"inv 1";}

            auto ItTotalEnd (holder->end<T>());
            size_t iSize =  holder->getViewGraphSize(iSelectedInterval);
            std::time_t tMinDate = holder->getViewGraphDateMin(Bar::eInterval::pTick);
            std::time_t tMaxDate =  holder->getViewGraphDateMax(Bar::eInterval::pTick);

            if (tStoredMinDate == 0 || tStoredMinDate > tMinDate)  tStoredMinDate = tMinDate;
            if (tStoredMaxDate == 0 || tStoredMaxDate < tMaxDate)  tStoredMaxDate = tMaxDate;

            SetMinMaxDateToControls();


            auto p = holder->getMinMax(iSelectedInterval,tStoredMinDate,tStoredMaxDate);

            double dLowMin  = std::get<0>(p);
            double dHighMax = std::get<1>(p);
            double dVolumeLowMin  = std::get<2>(p);
            double dVolumeHighMax = std::get<3>(p);
            bool bNeedRescale {false};
            bool bNeedRescaleVolume {false};
            if (dStoredLowMin == 0  || dStoredLowMin  > dLowMin)    { dStoredLowMin = dLowMin;      }
            if (dStoredHighMax == 0 || dStoredHighMax < dHighMax)   { dStoredHighMax = dHighMax;    }
            if (dStoredVolumeLowMin == 0  || dStoredVolumeLowMin  > dVolumeLowMin)    { dStoredVolumeLowMin = dVolumeLowMin;      }
            if (dStoredVolumeHighMax == 0 || dStoredVolumeHighMax < dVolumeHighMax)   { dStoredVolumeHighMax = dVolumeHighMax;    }

            if (mVScale.at(iSelectedInterval) == 0){
                bNeedRescale = true;
                if ((dStoredHighMax - dStoredLowMin) <= 0){
                    mVScale[iSelectedInterval] = 1.0;
                }
                else{
                    mVScale[iSelectedInterval] = (iViewPortHeight - iViewPortHighStrip - iViewPortLowStrip)/(dStoredHighMax - dStoredLowMin);
                }
            }

            if (mVVolumeScale.at(iSelectedInterval) == 0){              
                bNeedRescaleVolume = true;
                if ((dStoredVolumeHighMax - dStoredVolumeLowMin) <= 0){
                    mVVolumeScale[iSelectedInterval] = 1.0;
                }
                else{
                    mVVolumeScale[iSelectedInterval] = (ui->grViewVolume->maximumHeight())/(dStoredVolumeHighMax - dStoredVolumeLowMin);
                }

//                {
//                    ThreadFreeCout pcout;
//                    pcout << std::fixed;// << std::setprecision(2);
//                    pcout<<"dStoredVolumeHighMax =\t"<<dStoredVolumeHighMax<<"\n";
//                    pcout<<"dStoredVolumeLowMin =\t"<<dStoredVolumeLowMin<<"\n";
//                    pcout<<"ui->grViewVolume->maximumHeight() =\t"<<ui->grViewVolume->maximumHeight()<<"\n";
//                    pcout<<"mVVolumeScale[iSelectedInterval] =\t"<<mVVolumeScale[iSelectedInterval]<<"\n";
//                }

            }
            //{ThreadFreeCout pcout; pcout<<"inv 4";}

            // if neсessary clean all
            bool bWasTotalErase{false};
            if (    tStoredMinDate == 0
                || tMinDate != tStoredMinDate
                || (iStoredMaxSize !=iSize && tMinDate >= tStoredMinDate && tMaxDate <= tStoredMaxDate)
                || bNeedRescale
                || bNeedRescaleVolume
                    ){

                EraseLinesLower(mShowedGraphicsBars,  0, ui->grViewQuotes->scene());
                EraseLinesLower(mShowedVolumes,        0, ui->grViewVolume->scene());

                ui->grViewQuotes->scene()->invalidate(ui->grViewQuotes->scene()->sceneRect());
                ui->grViewVolume->scene()->invalidate(ui->grViewVolume->scene()->sceneRect());

                bWasTotalErase = true;
            }
            //{ThreadFreeCout pcout; pcout<<"inv 5";}

            // clean invalid range
            size_t iEnd = ItEnd.realPosition() + 1;
            iEnd = iEnd > iSize ? iSize : iEnd;
            //
            EraseLinesMid(mShowedGraphicsBars,  It.realPosition(),iEnd, ui->grViewQuotes->scene());
            EraseLinesMid(mShowedVolumes,       It.realPosition(),iEnd, ui->grViewVolume->scene());


            if (!bWasTotalErase && It.realPosition() < iEnd){

                qreal x1 = (It.realPosition() + iLeftShift ) * BarGraphicsItem::BarWidth * dHScale;
                qreal x2 = (iEnd + iLeftShift ) * BarGraphicsItem::BarWidth * dHScale;

                QRectF recS = ui->grViewQuotes->scene()->sceneRect();
                QRectF invS  = QRectF(x1,recS.y(),x2,recS.width());

                QRectF recVS = ui->grViewVolume->scene()->sceneRect();
                QRectF invVS  = QRectF(x1,recVS.y(),x2,recVS.width());

                ui->grViewQuotes->scene()->invalidate(invS);
                ui->grViewVolume->scene()->invalidate(invVS);

            }
            //{ThreadFreeCout pcout; pcout<<"inv 6";}

            // resize Scene rect
            if (iStoredMaxSize !=iSize || bNeedRescale || data.bNeedToRescale){

                //{ThreadFreeCout pcout; pcout<<"inv 6.1";}

                int iNewViewPortH = (mVScale.at(iSelectedInterval) * (dStoredHighMax - dStoredLowMin)  + iViewPortLowStrip + iViewPortHighStrip );
                if ( iViewPortHeight > iNewViewPortH){
                    iNewViewPortH = iViewPortHeight;
                }
                //{ThreadFreeCout pcout; pcout<<"inv 6.2";}

                QRectF newRec(0,-iNewViewPortH,
                              (iSize + iLeftShift + iRightShift) * BarGraphicsItem::BarWidth * dHScale  ,
                              iNewViewPortH
                              );
                //{ThreadFreeCout pcout; pcout<<"inv 6.3";}
                disconnect(ui->grHorizScroll->horizontalScrollBar(),SIGNAL(valueChanged(int)),this,SLOT(slotHorizontalScrollBarValueChanged(int)));
                grScene->setSceneRect(newRec);
                connect(ui->grHorizScroll->horizontalScrollBar(),SIGNAL(valueChanged(int)),this,SLOT(slotHorizontalScrollBarValueChanged(int)));
                //{ThreadFreeCout pcout; pcout<<"inv 6.4";}

                //if (!this->isHidden()){
                    //   ui->grViewQuotes->horizontalScrollBar()->triggerAction(QAbstractSlider::SliderToMaximum);
                  //  ui->grHorizScroll->horizontalScrollBar()->triggerAction(QAbstractSlider::SliderToMaximum);
                //}

//                int iNewSize = iSize + iLeftShift + iRightShift - ui->grViewQuotes->width()/(dHScale * BarGraphicsItem::BarWidth);
//                iNewSize = iNewSize < 0? 0 : iNewSize;
//                ui->horizontalScrollBar->setMaximum(iNewSize);
                EraseFrames();
                //{ThreadFreeCout pcout; pcout<<"inv 6.5";}
            }

            //{ThreadFreeCout pcout; pcout<<"inv 7";}
            // resize Scene rect for Volume
            if (iStoredMaxSize !=iSize || bNeedRescale || data.bNeedToRescale){

                int iNewVolumeSceneH = (mVVolumeScale.at(iSelectedInterval) * (dStoredVolumeHighMax - dStoredVolumeLowMin) );

                QRectF newRec(0,-iNewVolumeSceneH,
                              (iSize + iLeftShift + iRightShift) * BarGraphicsItem::BarWidth * dHScale  ,
                              iNewVolumeSceneH
                              );

                ui->grViewVolume->scene()->setSceneRect(newRec);
            }

            iStoredMaxSize  = iSize;
            tStoredMinDate  = tMinDate;
            tStoredMaxDate  = tMaxDate;


            //{ThreadFreeCout pcout; pcout<<"inv 8";}
            //---------------------------

            if (!this->isHidden()){
                if (tStoredRightPointPosition != 0){
                    SetSliderToPos(tStoredRightPointPosition, iStoredRightAggregate);
                }
                else{
                    SetSliderToPos(tStoredMaxDate, iRightShift);
                }
            }
            PaintViewPort   (true,true,true, false);
        }
    }
    return bSuccess;
}

//---------------------------------------------------------------------------------------------------------------
//void GraphViewForm::slotSliderValueChanged(int iBeg)
//{

//    int iMiddle = iBeg + (ui->grViewQuotes->width()/(2 * dHScale * BarGraphicsItem::BarWidth));
//    if (iSelectedInterval == Bar::eInterval::pTick){
//        SliderValueChanged<BarTick>(iMiddle - iLeftShift);
//    }
//    else{
//        SliderValueChanged<Bar>(iMiddle - iLeftShift);
//    }
//    SetSliderToPos(iMiddle);
//}
//---------------------------------------------------------------------------------------------------------------
//void GraphViewForm::SetSliderToPos(int iMiddlePos)
//{
//    // TODO:: viewport going down when slide!!!!! Why?
//    ui->grViewQuotes->centerOn((iMiddlePos /*- iLeftShift*/)*BarGraphicsItem::BarWidth * dHScale,
//                             ui->grViewQuotes->mapToScene(0,ui->grViewQuotes->viewport()->rect().center().ry()).y());
//    //
//    ui->grViewScaleUpper->centerOn(iMiddlePos*BarGraphicsItem::BarWidth * dHScale,
//                             ui->grViewScaleUpper->mapToScene(0,ui->grViewScaleUpper->viewport()->rect().center().ry()).y());
//    ui->grViewVolume->centerOn(iMiddlePos*BarGraphicsItem::BarWidth * dHScale,
//                             ui->grViewVolume->mapToScene(0,ui->grViewVolume->viewport()->rect().center().ry()).y());
//    ui->grViewScaleLower->centerOn(iMiddlePos*BarGraphicsItem::BarWidth * dHScale,
//                             ui->grViewScaleLower->mapToScene(0,ui->grViewScaleLower->viewport()->rect().center().ry()).y());
//}
////---------------------------------------------------------------------------------------------------------------
//template<typename T>
//bool GraphViewForm::RollSliderToMidTime(std::time_t tMidPos)
//{
//    bool bSuccess;
//    auto It = holder->beginIteratorByDate<T>(iSelectedInterval,tMidPos,bSuccess);
//    auto ItEnd (holder->end<T>());
//    if (bSuccess && It != ItEnd){
//        SliderValueChanged<T>(It.realPosition());
//        SetSliderToPos(It.realPosition() + iLeftShift);
////        ui->grViewQuotes->centerOn((It.realPosition() + iLeftShift)*BarGraphicsItem::BarWidth * dHScale,
////                                 ui->grViewQuotes->mapToScene(0,ui->grViewQuotes->viewport()->rect().center().ry()).y());

//    }
//    return bSuccess;
//}
//---------------------------------------------------------------------------------------------------------------
//template<typename T>
//void GraphViewForm::SliderValueChanged(int iMidPos)
//{

//    /////////////////////////////////////////////////////////////////////////////////////////
//    bool bSuccess;
//    auto ItDefender = holder->beginIteratorByDate<T>(iSelectedInterval,0,bSuccess);
//    if (!bSuccess) return;
//    //
//    size_t iMaxSize = holder->getViewGraphSize(iSelectedInterval);
//    if (iMaxSize ==0 ) return;
//    /////////////////////////////////////////////////////////////////////////////////////////

//    size_t iViewWidth = ui->grViewQuotes->width()/(dHScale *BarGraphicsItem::BarWidth);

//    size_t iMid = iMidPos > 0 ? (size_t) iMidPos : 0;
//    iMid = iMid < iMaxSize ? iMid : iMaxSize - 1;

//    size_t iBeg      = iMid > iViewWidth  + iLeftShift ? iMid - iViewWidth  - iLeftShift : 0 ;
//    size_t iEnd      = iMid + iViewWidth;
//    //-------------------------------------------
//    DrawHorizontalLines (iBeg - iLeftShift,iEnd);
//    DrawVertLines       (iBeg - iLeftShift,iEnd);
//    DrawVertSideFrames();
//    //-------------------------------------------

//    iEnd = iEnd < iMaxSize ? iEnd : iMaxSize - 1;

//    /////////////////////////////////////////////////////////////////

//    if (bSuccess && iMaxSize > 0){

//        EraseLinesUpper(mShowedGraphicsBars,  iBeg, ui->grViewQuotes->scene());
//        EraseLinesUpper(mVLinesVolume,        iBeg, ui->grViewVolume->scene());

//        EraseLinesLower(mShowedGraphicsBars,  iEnd, ui->grViewQuotes->scene());
//        EraseLinesLower(mVLinesVolume,        iEnd, ui->grViewVolume->scene());


//        T tM = holder->getByIndex<T>(iSelectedInterval,iMid);
//        T tB = holder->getByIndex<T>(iSelectedInterval,iBeg );
//        T tE = holder->getByIndex<T>(iSelectedInterval,iEnd );

//        tStoredMiddlePointPosition = tM.Period();
//        ////////////

//        //
//        //TODO: what invalidate?
//        //ui->grViewQuotes->scene()->sceneRect(); //invalidate scene
//        //InvalidateScenes();//invalidate scene
//        //
//        auto It = holder->beginIteratorByDate<T>(iSelectedInterval,tB.Period(),bSuccess);
//        auto ItEnd = holder->beginIteratorByDate<T>(iSelectedInterval,tE.Period(),bSuccess);
//        ++ItEnd;
//        auto ItTotalEnd = holder->end<T>();

//        QPen bluePen(Qt::blue,1,Qt::SolidLine);

//        std::stringstream ss;
//        std::time_t tTmp;

//        while(It != ItTotalEnd && It != ItEnd){
//            auto ItFound = std::find_if(mShowedGraphicsBars[It.realPosition()].begin(),
//                                        mShowedGraphicsBars[It.realPosition()].end(),
//                                        [&](const auto &p){
//                                            if(p->Period() == It->Period() &&
//                                               p->realPosition() == It.realPosition()) {
//                                                return true;
//                                             }
//                                             else return false;}
//                                        );

//            if (ItFound == mShowedGraphicsBars[It.realPosition()].end())
//            {
//                BarGraphicsItem *item = new BarGraphicsItem(*It,It.realPosition(),3,mVScale[iSelectedInterval]);
//                mShowedGraphicsBars[It.realPosition()].push_back(item);
//                ui->grViewQuotes->scene()->addItem(item);
//                qreal xCur = (It.realPosition() + iLeftShift)     * BarGraphicsItem::BarWidth * dHScale;
//                item->setPos(xCur , -realYtoSceneY(It->Close()));

//                ss.str("");
//                ss.clear();
//                tTmp = It->Period();
//                ss << threadfree_localtime_to_str(&tTmp)<<"\r\n";
//                ss << It->Volume();

//                DrawLineToScene(It.realPosition(), xCur,-3,xCur,-realYtoSceneYVolume(It->Volume())-3,
//                                mVLinesVolume, ui->grViewVolume->scene(),bluePen,ss.str(),true);
//            }
//            ++It;
//        }
//    }
//}
//---------------------------------------------------------------------------------------------------------------
//template<typename T>
//void GraphViewForm::SliderValueChangedBackup(int iMidPos)
//{

//    /////////////////////////////////////////////////////////////////////////////////////////
//    bool bSuccess;
//    auto ItDefender = holder->beginIteratorByDate<T>(iSelectedInterval,0,bSuccess);
//    if (!bSuccess) return;
//    //
//    size_t iMaxSize = holder->getViewGraphSize(iSelectedInterval);
//    if (iMaxSize ==0 ) return;
//    /////////////////////////////////////////////////////////////////////////////////////////

//    size_t iViewWidth = ui->grViewQuotes->width()/(dHScale *BarGraphicsItem::BarWidth);

//    size_t iMid = iMidPos > 0 ? (size_t) iMidPos : 0;
//    iMid = iMid < iMaxSize ? iMid : iMaxSize - 1;

//    size_t iBeg      = iMid > iViewWidth  + iLeftShift ? iMid - iViewWidth  - iLeftShift : 0 ;
//    size_t iEnd      = iMid + iViewWidth;
//    //-------------------------------------------
//    DrawHorizontalLines (iBeg - iLeftShift,iEnd);
//    DrawVertLines       (iBeg - iLeftShift,iEnd);
//    DrawVertSideFrames();
//    //-------------------------------------------

//    iEnd = iEnd < iMaxSize ? iEnd : iMaxSize - 1;

//    /////////////////////////////////////////////////////////////////

//    if (bSuccess && iMaxSize > 0){
//        T tM = holder->getByIndex<T>(iSelectedInterval,iMid);
//        T tB = holder->getByIndex<T>(iSelectedInterval,iBeg );
//        T tE = holder->getByIndex<T>(iSelectedInterval,iEnd );

//        tStoredMiddlePointPosition = tM.Period();
//        ////////////

//        size_t j=0;
//        while (j < vShowedGraphicsBars.size()){
//            if (vShowedGraphicsBars[j]->Period() < tB.Period() ||   // out of view range
//                vShowedGraphicsBars[j]->Period() > tE.Period() ||   // out of view range
//                vShowedGraphicsBars[j]->realPosition() >= iMaxSize  // leftovers from old fillings
//                    ){

//               ui->grViewQuotes->scene()->removeItem(vShowedGraphicsBars[j]);
//               delete (vShowedGraphicsBars[j]);
//               vShowedGraphicsBars.erase(std::next(vShowedGraphicsBars.begin(),j));

//            }
//            else{
//                T t = holder->getByIndex<T>(iSelectedInterval, vShowedGraphicsBars[j]->realPosition());
//                if (t.Period() != vShowedGraphicsBars[j]->Period()){ // leftovers from old fillings

//                    ui->grViewQuotes->scene()->removeItem(vShowedGraphicsBars[j]);
//                    delete (vShowedGraphicsBars[j]);
//                    vShowedGraphicsBars.erase(std::next(vShowedGraphicsBars.begin(),j));

//                }
//                else{
//                    ++j;
//                }
//            }
//        }
//        //
//        //TODO: what invalidate?
//        ui->grViewQuotes->scene()->sceneRect(); //invalidate scene
//        //InvalidateScenes();//invalidate scene
//        //
//        auto It = holder->beginIteratorByDate<T>(iSelectedInterval,tB.Period(),bSuccess);
//        auto ItEnd = holder->beginIteratorByDate<T>(iSelectedInterval,tE.Period()+1,bSuccess);
//        auto ItTotalEnd = holder->end<T>();

//        while(It != ItTotalEnd && It != ItEnd){
//            auto ItFound = std::find_if(vShowedGraphicsBars.begin(),vShowedGraphicsBars.end(),[&](const BarGraphicsItem *p){
//                if(p->Period() == It->Period() &&
//                   p->realPosition() == It.realPosition()
//                        ) {
//                    return true;
//                }
//                else return false;});
//            if (ItFound == vShowedGraphicsBars.end())
//            {
//                BarGraphicsItem *item = new BarGraphicsItem(*It,It.realPosition(),3);
//                vShowedGraphicsBars.push_back(item);
//                ui->grViewQuotes->scene()->addItem(item);
//                item->setPos((It.realPosition() + iLeftShift) * BarGraphicsItem::BarWidth * dHScale , -realYtoSceneY(It->Close()));
//            }
//            ++It;
//        }
//    }
//}

//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::resizeEvent(QResizeEvent *event)
{
//    int iNewSize = iStoredMaxSize + iLeftShift + iRightShift - ui->grViewQuotes->width()/(dHScale * BarGraphicsItem::BarWidth);
//    iNewSize = iNewSize < 0? 0 : iNewSize;
//    ui->horizontalScrollBar->setMaximum(iNewSize);
    //ui->grViewQuotes->horizontalScrollBar()->triggerAction(QAbstractSlider::SliderToMaximum);
    ///////////
    RepositionPlusMinusButtons();

    QWidget::resizeEvent(event);
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::showEvent(QShowEvent *event)
{
//    int iNewSize = iStoredMaxSize + iLeftShift + iRightShift - ui->grViewQuotes->width()/(dHScale * BarGraphicsItem::BarWidth);
//    iNewSize = iNewSize < 0? 0 : iNewSize;
//    ui->horizontalScrollBar->setMaximum(iNewSize);

    ui->grViewQuotes->horizontalScrollBar()->triggerAction(QAbstractSlider::SliderToMaximum);
    ui->grViewL1->horizontalScrollBar()->triggerAction(QAbstractSlider::SliderToMaximum);
    ui->grViewR1->horizontalScrollBar()->triggerAction(QAbstractSlider::SliderToMaximum);
    ui->grHorizScroll->horizontalScrollBar()->triggerAction(QAbstractSlider::SliderToMaximum);
    ///////////
    RepositionPlusMinusButtons();

    QWidget::showEvent(event);
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::slotSceneRectChanged( const QRectF & rect)
{

    QRectF newRecVR1 =      QRectF(0,rect.y(),   ui->grViewR1->maximumWidth(),rect.height());
    QRectF newRecVL1 =      QRectF(0,rect.y(),   ui->grViewL1->maximumWidth(),rect.height());
    QRectF newRecVScrl =    QRectF(0,rect.y(), ui->grVertScroll->maximumWidth(),rect.height());
    ui->grViewR1->scene()->setSceneRect(newRecVR1);
    ui->grViewL1->scene()->setSceneRect(newRecVL1);
    ui->grVertScroll->scene()->setSceneRect(newRecVScrl);

    QRectF newRecScUp =     QRectF(rect.x(),0,rect.width(), ui->grViewScaleUpper->maximumHeight());
    QRectF newRecVolume =   QRectF(rect.x(),- ui->grViewVolume->maximumHeight(),rect.width(), ui->grViewVolume->maximumHeight());
    //QRectF newRecScLow =    QRectF(rect.x(),0,rect.width(), ui->grViewScaleLower->maximumHeight());
    QRectF newRecHScScrl =   QRectF(rect.x(),0,rect.width(), ui->grHorizScroll->maximumHeight());

    ui->grViewScaleUpper->scene()->setSceneRect(newRecScUp);
    ui->grViewVolume->scene()->setSceneRect(newRecVolume);
    //ui->grViewScaleLower->scene()->setSceneRect(newRecScLow);
    ui->grHorizScroll->scene()->setSceneRect(newRecHScScrl);
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
void GraphViewForm::slotHorizontalScrollBarValueChanged(int iH)
{

    ui->grViewScaleUpper->horizontalScrollBar()->setValue(iH);
    ui->grViewVolume->horizontalScrollBar()->setValue(iH);
    //ui->grViewScaleLower->horizontalScrollBar()->setValue(iH);
    ui->grViewQuotes->horizontalScrollBar()->setValue(iH);
    ui->grHorizScroll->horizontalScrollBar()->setValue(iH);

    PaintViewPort(true,true,true,true);
}
//---------------------------------------------------------------------------------------------------------------
std::tuple<int,int,int,int> GraphViewForm::getHPartStep(double realH, double viewportH)
{
    int iUpCount{0};
    double dParts = (viewportH / 600.0) * 4.0;
    double realPartH = realH/dParts;


    realPartH = realPartH > 0 ? realPartH : -realPartH;
    double dOldPart = realPartH;

    int iBaseH{0};


    if (realPartH >= 1){
        while(realPartH /10 >= 1) {
            realPartH /=10;
            dOldPart = realPartH;
            iUpCount++;
        }
        if (dOldPart >=7.5) {
            iBaseH = 1;
            iUpCount++;}
        else if (dOldPart >=3.33){
            iBaseH = 5 ;
        }
        else if (dOldPart >=1.66){
            iBaseH = 2 ;
        }
        else {
            iBaseH = 1 ;
        }
    }
    else{
        while(realPartH * 10 < 1) {
            realPartH *=10;
            dOldPart = realPartH;
            iUpCount--;
        }
        if (dOldPart >=0.75) {
            iBaseH = 1 ;
        }
        else if (dOldPart >=3.33) {
            iBaseH = 5;
            iUpCount--;
        }
        else if (dOldPart >=1.66) {
            iBaseH = 2;
            iUpCount--;
        }
        else {
            iBaseH = 1; iUpCount--;
        }
    }

    return  {iBaseH,iUpCount,0,0};
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::DrawDoubleToScene(const int idx,const  qreal x ,const  qreal y,const double n,  Qt::AlignmentFlag alignH, Qt::AlignmentFlag alignV,
                                    std::map<int,std::vector<QGraphicsItem *>>& mM, QGraphicsScene *scene, const QFont & font)
{
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) <<n;

     QGraphicsTextItem * item = new QGraphicsTextItem(QString::fromStdString(ss.str()));
     item->setFont(font);
     mM[idx].push_back(item);
     scene->addItem(item);

     qreal X = x;
     if (alignH == Qt::AlignmentFlag::AlignCenter || alignH == Qt::AlignmentFlag::AlignHCenter){
         X = x - item->boundingRect().width()/2;
     }
     else if (alignH == Qt::AlignmentFlag::AlignRight){
         X = x - item->boundingRect().width();
     }

     qreal Y = y;
     if (alignV == Qt::AlignmentFlag::AlignCenter || alignV == Qt::AlignmentFlag::AlignVCenter){
         Y = y - item->boundingRect().height()/2;
     }
     else if (alignV == Qt::AlignmentFlag::AlignBottom){
         Y = y - item->boundingRect().height()/2;
     }

     item->setPos(X,Y);

}

//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::DrawIntToScene(const int idx,const  qreal x,const  qreal y,const  int n, Qt::AlignmentFlag alignH, Qt::AlignmentFlag alignV,
                                    std::map<int,std::vector<QGraphicsItem *>>& mM, QGraphicsScene *scene, const QFont & font)
{
     QGraphicsTextItem * item = new QGraphicsTextItem(QString::number(n));
     item->setFont(font);
     mM[idx].push_back(item);
     scene->addItem(item);

     qreal X = x;
     if (alignH == Qt::AlignmentFlag::AlignCenter || alignH == Qt::AlignmentFlag::AlignHCenter){
         X = x - item->boundingRect().width()/2;
     }
     else if (alignH == Qt::AlignmentFlag::AlignRight){
         X = x - item->boundingRect().width();
     }

     qreal Y = y;
     if (alignV == Qt::AlignmentFlag::AlignCenter || alignV == Qt::AlignmentFlag::AlignVCenter){
         Y = y - item->boundingRect().height()/2;
     }
     else if (alignV == Qt::AlignmentFlag::AlignBottom){
         Y = y - item->boundingRect().height()/2;
     }

     item->setPos(X,Y);

}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::DrawTimeToScene(const int idx,const  qreal x,const  qreal y,const  std::tm & tmT,
                                    std::map<int,std::vector<QGraphicsItem *>>& mM, QGraphicsScene *scene, const QFont & font)
{
   std::stringstream ss;
   ss <<std::setfill('0');
   ss << std::setw(2)<<tmT.tm_hour<<":";
   ss << std::setw(2)<<tmT.tm_min;

    QGraphicsTextItem * item = new QGraphicsTextItem(QString::fromStdString(ss.str()));
    item->setFont(font);
    mM[idx].push_back(item);
    scene->addItem(item);
    item->setPos(x,y);

}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::DrawIntermittentLineToScene(const int idx,const  qreal x1,const  qreal y1,const qreal x2,const  qreal y2,
                     std::map<int,std::vector<QGraphicsItem *>>& mM, QGraphicsScene *scene, const QPen & pen,
                     const  std::time_t t, bool bHasTooltip)
{
    std::stringstream ss;
    if (bHasTooltip && t != 0){
        ss <<threadfree_localtime_to_str(&t);
    }

    DrawIntermittentLineToScene(idx,x1,y1,x2,y2,mM,scene, pen,ss.str(),  bHasTooltip);
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::DrawIntermittentLineToScene(const int idx,const  qreal x1,const  qreal y1,const qreal x2,const  qreal y2,
                     std::map<int,std::vector<QGraphicsItem *>>& mM, QGraphicsScene *scene, const QPen & pen,
                     const  std::string sToolTip, bool bHasTooltip)
{

    static int iCount{0};
    iCount++;

    // break into segments
    auto It = vHLines.begin();
    auto ItEnd = vHLines.end();

    std::vector<std::pair<double,double>> vD;

    double Y1 {y1};
    double Y2 {y2};
    if (Y1 < Y2) std::swap(Y1,Y2);

    double y {Y1};
    double yTmp{0};

    while(It != ItEnd){
        yTmp = -realYtoSceneY(It->first);
        if (yTmp < y){
            vD.push_back({y, yTmp});
            y = yTmp;
        }
        ++It;
    }
    vD.push_back({y, Y2});
    ////////////////////////////////
    // draw segmentes
    for (const auto & s:vD){
        if (-(s.second - s.first) > 12){
            DrawLineToScene(idx,x1,s.first - 6 , x2, s.second + 6, mM, scene,pen,sToolTip, bHasTooltip);
        }
    }



}

//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::DrawLineToScene(const int idx,const  qreal x1,const  qreal y1,const qreal x2,const  qreal y2,
                                    std::map<int,std::vector<QGraphicsItem *>>& mM, QGraphicsScene *scene, const QPen & pen, const  std::time_t t, bool bHasTooltip)
{
    std::stringstream ss;
    if (bHasTooltip && t != 0){
        ss <<threadfree_localtime_to_str(&t);
    }
    DrawLineToScene(idx,x1,y1,x2,y2,mM, scene,pen,ss.str(), bHasTooltip);
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::DrawLineToScene(const int idx,const  qreal x1,const  qreal y1,const qreal x2,const  qreal y2,
                     std::map<int,std::vector<QGraphicsItem *>>& mM, QGraphicsScene *scene, const QPen & pen,
                     const  std::string sToolTip, bool bHasTooltip)
{

    QGraphicsLineItem *item = new QGraphicsLineItem(0,0,x2-x1,y2-y1);
    item->setPen(pen);
    mM[idx].push_back(item);

    //item->setTransformationMode(Qt::SmoothTransformation);
    scene->addItem(item);
    item->setPos(x1,y1);

    if (bHasTooltip)
        item->setToolTip(QString::fromStdString(sToolTip));
}
//---------------------------------------------------------------------------------------------------------------
template<typename T>
void GraphViewForm::EraseLinesMid(T& mM, int iStart,int iEnd, QGraphicsScene *scene)
//void GraphViewForm::EraseLinesMid(std::map<int,std::vector<QGraphicsItem *>>& mM, int iStart,int iEnd, QGraphicsScene *scene)
{
    auto It     (mM.lower_bound(iStart));
    auto ItCp   (It);
    auto ItEnd  (mM.upper_bound(iEnd));

    while(It != ItEnd){
        for (auto &t:It->second){
            scene->removeItem(t);
            delete t;
            t = nullptr;
        }
        It->second.clear();
        It++;
    }
    mM.erase(ItCp,ItEnd);

}
//---------------------------------------------------------------------------------------------------------------
template<typename T>
void GraphViewForm::EraseLinesLower(T& mM, int iStart, QGraphicsScene * scene)
//void GraphViewForm::EraseLinesLower(std::map<int,std::vector<QGraphicsItem *>>& mM, int iStart, QGraphicsScene * scene)
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
template<typename T>
void GraphViewForm::EraseLinesUpper(T& mM, int iEnd, QGraphicsScene *scene)
//void GraphViewForm::EraseLinesUpper(std::map<int,std::vector<QGraphicsItem *>>& mM, int iEnd, QGraphicsScene *scene)
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
void GraphViewForm::InvalidateScenes()
{
    ui->grViewQuotes->scene()->invalidate(ui->grViewQuotes->scene()->sceneRect());
    ui->grViewScaleUpper->scene()->invalidate(ui->grViewScaleUpper->scene()->sceneRect());
    ui->grViewVolume->scene()->invalidate(ui->grViewVolume->scene()->sceneRect());
    ui->grViewL1->scene()->invalidate(ui->grViewL1->scene()->sceneRect());
    ui->grViewR1->scene()->invalidate(ui->grViewR1->scene()->sceneRect());

}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::slotHScaleQuotesClicked(bool bPlus)
{
    if (dHScale ==0) dHScale = 1.0;
    if (dHScale > 0.5 || bPlus){
        if (bPlus) dHScale *= 1.1;
        else dHScale *= 0.9;

//        std::time_t tMinDate = holder->getViewGraphDateMin(Bar::eInterval::pTick);
//        std::time_t tMaxDate =  holder->getViewGraphDateMax(Bar::eInterval::pTick);
        Erase();
        PaintViewPort(true,true,true,true);
        //slotInvalidateGraph(tMinDate,tMaxDate,true);
        //InvalidateScenes();
    }
}
void GraphViewForm::slotVScaleQuotesClicked(bool bPlus)
{
    if (mVScale[iSelectedInterval] ==0) mVScale[iSelectedInterval] = 1.0;
    if (bPlus) mVScale[iSelectedInterval] *= 1.1;
    else mVScale[iSelectedInterval] *= 0.9;

//    std::time_t tMinDate = holder->getViewGraphDateMin(Bar::eInterval::pTick);
//    std::time_t tMaxDate =  holder->getViewGraphDateMax(Bar::eInterval::pTick);
//    EraseFrames();
//    slotInvalidateGraph(tMinDate,tMaxDate,true);
//    InvalidateScenes();
    Erase();
    PaintViewPort(true,true,true,false);
}
void GraphViewForm::slotHScaleVolumeClicked(bool bPlus)
{

    if (mVVolumeScale[iSelectedInterval] ==0) mVVolumeScale[iSelectedInterval] = 1.0;
    if (bPlus) mVVolumeScale[iSelectedInterval] *= 1.1;
    else mVVolumeScale[iSelectedInterval] *= 0.9;

    EraseLinesLower(mShowedVolumes,        0, ui->grViewVolume->scene());
    PaintViewPort(false,false,true,false);

    //ui->grViewVolume->scene()->invalidate(ui->grViewVolume->scene()->sceneRect());
    //ui->grViewVolume->scene()->invalidate(ui->grViewVolume->sceneRect());

//    {
//        ThreadFreeCout pcout;
//        pcout <<( (bPlus)? "Volume plus":"Volume minus")<<"\n";
//    }
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::Erase()
{
    EraseFrames();
    EraseBars();
    EraseVolumes();
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::EraseBars()
{
    EraseLinesLower(mShowedGraphicsBars,  0, ui->grViewQuotes->scene());
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::EraseVolumes()
{
//    ThreadFreeCout pcout;
//    pcout <<"erase volumes\n";

    EraseLinesLower(mShowedVolumes,  0, ui->grViewVolume->scene());
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::EraseFrames()
{
    for (auto & el:vHorizFramesViewQuotes){
        ui->grViewQuotes->scene()->removeItem(el.first);
        delete el.first;
        el.first = nullptr;
    }
    vHorizFramesViewQuotes.clear();
    ///
    for (auto & el:vHorizFramesScaleUpper){
        ui->grViewScaleUpper->scene()->removeItem(el);
        delete el;
        el = nullptr;
    }
    vHorizFramesScaleUpper.clear();
    ///

    //////////////////////////
    EraseLinesLower(mVFramesViewQuotes      , 0, ui->grViewQuotes->scene());
    EraseLinesLower(mVFramesScaleUpper      , 0, ui->grViewScaleUpper->scene());
    EraseLinesLower(mVFramesVolume          , 0, ui->grViewVolume->scene());
    EraseLinesLower(mVFramesHorisSmallScale , 0, ui->grViewScaleUpper->scene());
    //////////////////////////
    EraseLinesLower(mLeftFrames             , 0, ui->grViewL1->scene());
    EraseLinesLower(mRightFrames            , 0, ui->grViewR1->scene());

}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::SetMinMaxDateToControls()
{
    {
        std::tm* tmSt=threadfree_localtime(&tStoredMinDate);
        const QDate dtS(tmSt->tm_year+1900,tmSt->tm_mon+1,tmSt->tm_mday);
        const QTime tmS(tmSt->tm_hour,tmSt->tm_min,tmSt->tm_sec);
        QDateTime dt (dtS,tmS);
        ui->dtBeginDate->setDateTime(dt);
    }
    {
        std::tm* tmSt=threadfree_localtime(&tStoredMaxDate);
        const QDate dtS(tmSt->tm_year+1900,tmSt->tm_mon+1,tmSt->tm_mday);
        const QTime tmS(tmSt->tm_hour,tmSt->tm_min,tmSt->tm_sec);
        QDateTime dt (dtS,tmS);
        ui->dtEndDate->setDateTime(dt);
    }
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::dateTimeBeginChanged(const QDateTime&)
{
    SetMinMaxDateToControls(); // keep old value
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::dateTimeEndChanged(const QDateTime&)
{
    SetMinMaxDateToControls(); // keep old value
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::setFramesVisibility(std::tuple<bool,bool,bool,bool,bool> tp)
{

    auto [bLeft, bRight, bUpper, bLower, bVolume] {tp};


    ui->grViewL1->setVisible(bLeft);
    ui->grViewL2->setVisible(bLeft && bUpper);
    ui->grViewL3->setVisible(bLeft && bVolume);
    ui->grViewL4->setVisible(bLeft && bLower);

    ui->grViewR1->setVisible(bRight);
    ui->grViewR2->setVisible(bRight && bUpper);
    ui->grViewR3->setVisible(bRight && bVolume);
    ui->grViewR4->setVisible(bRight && bLower);

    ui->grViewScaleUpper->setVisible(bUpper);
    ui->grViewVolume->setVisible(bVolume);
//    if (bVolume && !ui->grViewVolume->isVisible()) ui->grViewVolume->show();
//    if (!bVolume && ui->grViewVolume->isVisible()) ui->grViewVolume->hide();

    ui->grViewScaleLower->setVisible(bLower);

    btnScaleVVolumePlus->setVisible(bVolume);
    btnScaleVVolumeMinus->setVisible(bVolume);

    RepositionPlusMinusButtons();

}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::RepositionPlusMinusButtons()
{
    QRect rectQ = ui->grViewQuotes->rect();
    QPoint pQ   = ui->grViewQuotes->pos();
    QPoint pV   = ui->grViewVolume->pos();
    QPoint pU   = ui->grViewScaleUpper->pos();
    QPoint pL   = ui->grViewScaleLower->pos();
    //QPoint pSl   = ui->horizontalScrollBar->pos();
    QPoint pSl   = ui->grHorizScroll->pos();

    QPoint pT = pU;
    if (!ui->grViewScaleUpper->isVisible()){
            pT = pV;
            if (!ui->grViewVolume->isVisible()){
                pT = pL;
                if (!ui->grViewScaleLower->isVisible()){
                    pT = pSl;
                }
            }
    }

    btnScaleVViewPlus->move     (pQ.x() + rectQ.width() - 15, pQ.y() + 10 );
    btnScaleVViewMinus->move    (pQ.x() + rectQ.width() - 15, pQ.y() + 30 );

    btnScaleHViewPlus->move     (pQ.x() + rectQ.width() - 30, pT.y() - 20 );
    btnScaleHViewMinus->move    (pQ.x() + rectQ.width() - 15, pT.y() - 20 );

    btnScaleVVolumePlus->move   (pQ.x() + rectQ.width() - 15, pV.y() + 10 );
    btnScaleVVolumeMinus->move  (pQ.x() + rectQ.width() - 15, pV.y() + 30 );
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::slotPeriodButtonChanged()
{
    Bar::eInterval iNewInterval{Bar::eInterval::pUndefined};

    if (ui->btnTick->isChecked())       {iNewInterval = Bar::eInterval::pTick;     }
    else if (ui->btn1->isChecked())     {iNewInterval = Bar::eInterval::p1;        }
    else if (ui->btn5->isChecked())     {iNewInterval = Bar::eInterval::p5;        }
    else if (ui->btn10->isChecked())    {iNewInterval = Bar::eInterval::p10;       }
    else if (ui->btn15->isChecked())    {iNewInterval = Bar::eInterval::p15;       }
    else if (ui->btn30->isChecked())    {iNewInterval = Bar::eInterval::p30;       }
    else if (ui->btn60->isChecked())    {iNewInterval = Bar::eInterval::p60;       }
    else if (ui->btn120->isChecked())   {iNewInterval = Bar::eInterval::p120;      }
    else if (ui->btn180->isChecked())   {iNewInterval = Bar::eInterval::p180;      }
    else if (ui->btnDay->isChecked())   {iNewInterval = Bar::eInterval::pDay;      }
    else if (ui->btnWeek->isChecked())  {iNewInterval = Bar::eInterval::pWeek;     }
    else if (ui->btnMonth->isChecked()) {iNewInterval = Bar::eInterval::pMonth;    }
    else{
        throw std::runtime_error("No period button selected!");
    }
    //
    if (iSelectedInterval <  iNewInterval){
      tStoredRightPointPosition = Bar::DateAccommodate(tStoredRightPointPosition,iNewInterval,true);
    }

    iSelectedInterval =  iNewInterval;

    dStoredLowMin           = 0 ;
    dStoredHighMax          = 0 ;
    dStoredVolumeLowMin     = 0;
    dStoredVolumeHighMax    = 0;
    /////
//    EraseLinesLower(mShowedGraphicsBars,  0, ui->grViewQuotes->scene());
//    EraseLinesLower(mVFramesVolume,        0, ui->grViewVolume->scene());
    //EraseLinesFrames();
    Erase();

    slotInvalidateGraph(tStoredMinDate, tStoredMaxDate);

}
//---------------------------------------------------------------------------------------------------------------
 void GraphViewForm::SetSelectedIntervalToControls()
 {
     switch (iSelectedInterval) {
     case Bar::eInterval::pTick:    ui->btnTick->setChecked(true); break;
     case Bar::eInterval::p1:       ui->btn1->setChecked(true); break;
     case Bar::eInterval::p5:       ui->btn5->setChecked(true); break;
     case Bar::eInterval::p10:      ui->btn10->setChecked(true); break;
     case Bar::eInterval::p15:      ui->btn15->setChecked(true); break;
     case Bar::eInterval::p30:      ui->btn30->setChecked(true); break;
     case Bar::eInterval::p60:      ui->btn60->setChecked(true); break;
     case Bar::eInterval::p120:     ui->btn120->setChecked(true); break;
     case Bar::eInterval::p180:     ui->btn180->setChecked(true); break;
     case Bar::eInterval::pDay:     ui->btnDay->setChecked(true); break;
     case Bar::eInterval::pWeek:    ui->btnWeek->setChecked(true); break;
     case Bar::eInterval::pMonth:   ui->btnMonth->setChecked(true); break;
     default: break;
     }
 }
//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
 void GraphViewForm::PaintViewPort   (bool bFrames ,bool bBars ,bool bVolumes, bool bStoreRightPos)
 {
     int iBeg = ui->grViewQuotes->horizontalScrollBar()->value();
     int iEnd = iBeg + ui->grViewQuotes->horizontalScrollBar()->pageStep();

     if (ui->grViewQuotes->horizontalScrollBar()->maximum() > 0 ) { // if slider range was espanded
         iBeg = ((iBeg/dHScale)/(double)BarGraphicsItem::BarWidth);
         iEnd = ((iEnd/dHScale)/(double)BarGraphicsItem::BarWidth);

         iBeg = iBeg - iLeftShift;
         iEnd = iEnd - iLeftShift;
     }
     else{
         QRectF rS = ui->grViewQuotes->scene()->sceneRect();

         iBeg = 0;
         iEnd = ((rS.width()/dHScale)/(double)BarGraphicsItem::BarWidth) - iLeftShift;
     }

     PaintViewPort   (iBeg, iEnd, bFrames, bBars, bVolumes,bStoreRightPos);
 }
 //---------------------------------------------------------------------------------------------------------------
 void GraphViewForm::PaintViewPort   (int iStart, int iEnd,bool bFrames,bool bBars,bool bVolumes, bool bStoreRightPos)
 {

     if (bFrames){
        PaintHorizontalScales();
        PaintHorizontalFrames(iStart,iEnd);
        PaintVerticalSideScales();
        PaintVerticalFrames(iStart,iEnd);
     }
     //
     if (iSelectedInterval == Bar::eInterval::pTick){
         PaintBars<BarTick>   (iStart,iEnd,bBars, bVolumes,bStoreRightPos);
     }
     else{
         PaintBars<Bar>       (iStart,iEnd,bBars, bVolumes,bStoreRightPos);
     }
 }
 //---------------------------------------------------------------------------------------------------------------
 template<typename T>
 void GraphViewForm::PaintBars       (int iStartI, int iEndI , bool bPaintBars, bool bPaintVolumes, bool bStoreRightPos)
 {
     /////////////////////////////////////////////////////////////////////////////////////////
     bool bSuccess;
     auto ItDefender = holder->beginIteratorByDate<T>(iSelectedInterval,0,bSuccess);
     if (!bSuccess) return;
     //
     size_t iMaxSize = holder->getViewGraphSize(iSelectedInterval);
     if (iMaxSize == 0 ) return;
     /////////////////////////////////////////////////////////////////////////////////////////

     //size_t iViewWidth = ui->grViewQuotes->width()/(dHScale *BarGraphicsItem::BarWidth);


     size_t iBeg    = iStartI >= 0          ? iStartI   : 0 ;
     iBeg           = iBeg < iMaxSize       ? iBeg      : iMaxSize - 1;

     size_t iEnd    = iEndI >= 0             ? iEndI      : 0 ;
     iEnd           = iEnd < iMaxSize ? iEnd     : iMaxSize - 1;


     if (bStoreRightPos && iEnd > 0){
         tStoredRightPointPosition = Bar::DateAccommodate(holder->getTimeByIndex(iSelectedInterval,iEnd),iSelectedInterval,true);
         iStoredRightAggregate = (iEndI > 0 ? iEndI : 0);
         iStoredRightAggregate = (size_t)iStoredRightAggregate > iEnd ? iStoredRightAggregate - iEnd : 0 ;
     }


     //-------------------------------------------
//     DrawHorizontalLines (iBeg - iLeftShift,iEnd);
//     DrawVertLines       (iBeg - iLeftShift,iEnd);
//     DrawVertSideFrames();
     //-------------------------------------------

//     iEnd = iEnd < iMaxSize ? iEnd : iMaxSize - 1;

     /////////////////////////////////////////////////////////////////

     if (bSuccess && iMaxSize > 0){

         if (bPaintBars){
             EraseLinesUpper(mShowedGraphicsBars,  iBeg, ui->grViewQuotes->scene());
             EraseLinesLower(mShowedGraphicsBars,  iEnd, ui->grViewQuotes->scene());
         }
         if (bPaintVolumes){
             EraseLinesUpper(mShowedVolumes,        iBeg, ui->grViewVolume->scene());
             EraseLinesLower(mShowedVolumes,        iEnd, ui->grViewVolume->scene());
         }
         ///////////////////
         std::stringstream ss;
         std::time_t tTmp;
         QPen bluePen(Qt::blue,1,Qt::SolidLine);

         for (size_t i = iBeg ; i <= iEnd; ++i){
             qreal xCur = (i + iLeftShift)     * BarGraphicsItem::BarWidth * dHScale;
             T &b = holder->getByIndex<T>(iSelectedInterval,i);
             //
             if (bPaintBars){
                 auto ItFound = mShowedGraphicsBars.find(i);

                 if (ItFound == mShowedGraphicsBars.end())
                 {
                           BarGraphicsItem *item = new BarGraphicsItem(b,i,3,mVScale[iSelectedInterval]);
                     mShowedGraphicsBars[i].push_back(item);
                     ui->grViewQuotes->scene()->addItem(item);
                     item->setPos(xCur , -realYtoSceneY(b.Close()));
                }
             }

             //====================================================
             if (bPaintVolumes){
                 auto ItFound = mShowedVolumes.find(i);

                 if (ItFound == mShowedVolumes.end())
                 {
                     ss.str("");
                     ss.clear();
                     tTmp = b.Period();
                     ss << threadfree_localtime_to_str(&tTmp)<<"\r\n";
                     ss << b.Volume();

                     DrawLineToScene(i, xCur,-3,xCur,-realYtoSceneYVolume(b.Volume())-3,
                                     mShowedVolumes, ui->grViewVolume->scene(),bluePen,ss.str(),true);
                 }
             }
         }
     }

     //InvalidateScenes();
 }
 //---------------------------------------------------------------------------------------------------------------
// template<typename T>
// void GraphViewForm::PaintBars       (int iStartI, int iEndI , bool bPaintBars, bool bPaintVolumes)
// {
//     /////////////////////////////////////////////////////////////////////////////////////////
//     bool bSuccess;
//     auto ItDefender = holder->beginIteratorByDate<T>(iSelectedInterval,0,bSuccess);
//     if (!bSuccess) return;
//     //
//     size_t iMaxSize = holder->getViewGraphSize(iSelectedInterval);
//     if (iMaxSize == 0 ) return;
//     /////////////////////////////////////////////////////////////////////////////////////////

//     //size_t iViewWidth = ui->grViewQuotes->width()/(dHScale *BarGraphicsItem::BarWidth);


//     size_t iBeg    = iStartI >= 0          ? iStartI   : 0 ;
//     iBeg           = iBeg < iMaxSize       ? iBeg      : iMaxSize - 1;

//     size_t iEnd    = iEndI >= 0             ? iEndI      : 0 ;
//     iEnd           = iEnd < iMaxSize ? iEnd     : iMaxSize - 1;
//     //-------------------------------------------
////     DrawHorizontalLines (iBeg - iLeftShift,iEnd);
////     DrawVertLines       (iBeg - iLeftShift,iEnd);
////     DrawVertSideFrames();
//     //-------------------------------------------

////     iEnd = iEnd < iMaxSize ? iEnd : iMaxSize - 1;

//     /////////////////////////////////////////////////////////////////

//     if (bSuccess && iMaxSize > 0){

//         if (bPaintBars){
//             EraseLinesUpper(mShowedGraphicsBars,  iBeg, ui->grViewQuotes->scene());
//             EraseLinesLower(mShowedGraphicsBars,  iEnd, ui->grViewQuotes->scene());
//         }
//         if (bPaintVolumes){
//             EraseLinesUpper(mShowedVolumes,        iBeg, ui->grViewVolume->scene());
//             EraseLinesLower(mShowedVolumes,        iEnd, ui->grViewVolume->scene());
//         }



//         T tB = holder->getByIndex<T>(iSelectedInterval,iBeg );
//         T tE = holder->getByIndex<T>(iSelectedInterval,iEnd );

//         //tStoredMiddlePointPosition = tM.Period();
//         tStoredRightPointPosition = iBeg;
//         ////////////

//         //
//         //
//         auto It = holder->beginIteratorByDate<T>(iSelectedInterval,tB.Period(),bSuccess);
//         auto ItEnd = holder->beginIteratorByDate<T>(iSelectedInterval,tE.Period(),bSuccess);
//         ++ItEnd;
//         auto ItTotalEnd = holder->end<T>();

//         QPen bluePen(Qt::blue,1,Qt::SolidLine);

//         std::stringstream ss;
//         std::time_t tTmp;



//         while(It != ItTotalEnd && It != ItEnd){
//             //====================================================
//             qreal xCur = (It.realPosition() + iLeftShift)     * BarGraphicsItem::BarWidth * dHScale;
//             //====================================================
//             if (bPaintBars){
//                 auto ItFound = std::find_if(mShowedGraphicsBars[It.realPosition()].begin(),
//                                             mShowedGraphicsBars[It.realPosition()].end(),
//                                             [&](const auto &p){
//                                                 if(p->Period() == It->Period() &&
//                                                    p->realPosition() == It.realPosition()) {
//                                                     return true;
//                                                  }
//                                                  else return false;}
//                                             );
//                 if (ItFound == mShowedGraphicsBars[It.realPosition()].end())
//                 {
//                     BarGraphicsItem *item = new BarGraphicsItem(*It,It.realPosition(),3,mVScale[iSelectedInterval]);
//                     mShowedGraphicsBars[It.realPosition()].push_back(item);
//                     ui->grViewQuotes->scene()->addItem(item);
//                     item->setPos(xCur , -realYtoSceneY(It->Close()));
//                }
//             }

//             //====================================================
//             if (bPaintVolumes){

//                 auto ItFound = std::find_if(mShowedVolumes[It.realPosition()].begin(),
//                                             mShowedVolumes[It.realPosition()].end(),
//                                             [&](const auto &p){
//                                                 if(p->Period() == It->Period() &&
//                                                    p->realPosition() == It.realPosition()) {
//                                                     return true;
//                                                  }
//                                                  else return false;}
//                                             );
//                 if (ItFound == mShowedVolumes[It.realPosition()].end())
//                 {
//                     ss.str("");
//                     ss.clear();
//                     tTmp = It->Period();
//                     ss << threadfree_localtime_to_str(&tTmp)<<"\r\n";
//                     ss << It->Volume();

//                     DrawLineToScene(It.realPosition(), xCur,-3,xCur,-realYtoSceneYVolume(It->Volume())-3,
//                                     mShowedVolumes, ui->grViewVolume->scene(),bluePen,ss.str(),true);
//                 }
//             }
//             //====================================================

//             ++It;
//         }
//     }

//     //InvalidateScenes();
// }
 //---------------------------------------------------------------------------------------------------------------
 void GraphViewForm::PaintHorizontalFrames       (int /*iStart*/, int /*iEnd*/)
 {
     double realH = dStoredHighMax - dStoredLowMin;
     double viewPortH = realYtoSceneY(dStoredHighMax) - realYtoSceneY(dStoredLowMin);

     auto [stepBase,stepPow,subBase,subPow] =  getHPartStep( realH,viewPortH);
     double dFirst = (int(dStoredLowMin / std::pow(10,stepPow)) - int(dStoredLowMin / std::pow(10,stepPow)) % stepBase)*std::pow(10,stepPow);
     double dStep = stepBase * std::pow(10,stepPow);

     double dCurrent = dFirst + dStep;

     vHLines.clear();
     while(dCurrent < dStoredHighMax){
         vHLines.push_back({dCurrent,false});
         dCurrent +=  dStep;
     }
     ////
     int iMoveCount{0};
     bool bWas{false};
     size_t i = 0;
     while ( i < vHorizFramesViewQuotes.size()){
         bWas = false;
         for (auto &d: vHLines){
             if (vHorizFramesViewQuotes[i].second == d.first){
                 d.second = true;
                 bWas = true;
                 break;
             }
         }
         if (!bWas){
             ui->grViewQuotes->scene()->removeItem(vHorizFramesViewQuotes[i].first);
             delete vHorizFramesViewQuotes[i].first;
             vHorizFramesViewQuotes.erase(std::next(vHorizFramesViewQuotes.begin(),i));
         }
         else{
             iMoveCount++;
              ++i;
         }
     }
     ///////////

     //QPen Pen(Qt::black,1,Qt::DashLine);
     //QColor color(31, 53, 200,40);
     //QColor color(255, 159, 0,255);// orange

     //QColor color(255, 153, 0,55);// orange
     QColor color(204, 122, 0,155);// orange

     //QPen Pen(Qt::black,0.5,Qt::DashLine);
     QPen Pen(color,1,Qt::DashLine);


     int iCount{0};
     for(const auto &d:vHLines){
         if (!d.second){
             QGraphicsLineItem *item = new QGraphicsLineItem(0,0,ui->grViewQuotes->scene()->sceneRect().width(),0);
             item->setPen(Pen);
             vHorizFramesViewQuotes.push_back({item,d.first});
             ui->grViewQuotes->scene()->addItem(item);
             item->setPos(0,-realYtoSceneY(d.first));

             std::stringstream ss;
             ss <<d.first;
             item->setToolTip(QString::fromStdString(ss.str()));

             iCount++;
         }
     }
 }
 //---------------------------------------------------------------------------------------------------------------
 void GraphViewForm::PaintVerticalSideScales     ()
 {
     QRectF rL = ui->grViewL1->scene()->sceneRect();
     QRectF rR = ui->grViewR1->scene()->sceneRect();

     //QPen blackSolidPen(Qt::black,1,Qt::SolidLine);
     QPen blackHalfPen(Qt::black,0.5,Qt::SolidLine);
 //    QPen blackDashPen(Qt::gray,1,Qt::DashLine);
 //    QPen blackDotPen(Qt::gray,1,Qt::DotLine);
 //    QPen simpleDashPen(Qt::gray,1,Qt::DashLine);

     QFont font;
     font.setPixelSize(12);
     font.setBold(false);
     font.setFamily("Times New Roman");


     if (mLeftFrames.size() == 0){
         DrawLineToScene(0, rL.width() - 1, 0, rL.width()-1 , -rL.height(), mLeftFrames, ui->grViewL1->scene(),blackHalfPen,0,false);
         DrawLineToScene(0, 0,          0,          0 , -rR.height(), mRightFrames, ui->grViewR1->scene(),blackHalfPen,0,false);

         qreal y;
         for(const auto &d:vHLines){
             y  = -realYtoSceneY(d.first);
             DrawLineToScene(0, rL.width(), y, rL.width() -3 , y, mLeftFrames,  ui->grViewL1->scene(),blackHalfPen,0,false);
             DrawLineToScene(0, 0,          y,             2 , y, mRightFrames, ui->grViewR1->scene(),blackHalfPen,0,false);


             DrawDoubleToScene(0, rL.width() - 3, y, d.first,Qt::AlignmentFlag::AlignRight ,Qt::AlignmentFlag::AlignCenter ,mLeftFrames, ui->grViewL1->scene(), font);
             DrawDoubleToScene(0, 3             , y, d.first,Qt::AlignmentFlag::AlignLeft  ,Qt::AlignmentFlag::AlignCenter ,mRightFrames, ui->grViewR1->scene(), font);

         }

     }
 }
 //---------------------------------------------------------------------------------------------------------------
 void GraphViewForm::PaintVerticalFrames         (int iStart, int iEnd)
 {
     if (iSelectedInterval == Bar::eInterval::pTick){
         PainVerticalFramesT<BarTick>   (iStart, iEnd);
     }
     else{
         PainVerticalFramesT<Bar>       (iStart, iEnd);
     }
 }
 //---------------------------------------------------------------------------------------------------------------
 template<typename T>
 void GraphViewForm::PainVerticalFramesT         (int iBeg, int iEnd)
 {
     /// we must know what was before
     if (iBeg < 0) iBeg = 0;
     int iLeftBeg = iBeg;

     EraseLinesUpper(mVFramesViewQuotes,  iBeg, ui->grViewQuotes->scene());
     EraseLinesUpper(mVFramesScaleUpper,  iBeg, ui->grViewScaleUpper->scene());
     EraseLinesUpper(mVFramesVolume,      iBeg, ui->grViewVolume->scene());

     EraseLinesLower(mVFramesViewQuotes,  iEnd, ui->grViewQuotes->scene());
     EraseLinesLower(mVFramesScaleUpper,  iEnd, ui->grViewScaleUpper->scene());
     EraseLinesLower(mVFramesVolume,      iEnd, ui->grViewVolume->scene());

     EraseLinesUpper(mVFramesHorisSmallScale,      iBeg, ui->grViewScaleUpper->scene());
     EraseLinesLower(mVFramesHorisSmallScale,      iEnd, ui->grViewScaleUpper->scene());



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
     //qreal xPre{0};
     int iFCount{0};
     int iDayCounter{0};
     int iSecCounter{0};
     std::time_t tTmp{0};

     std::tm tmPre = *threadfree_localtime(&tTmp);
     std::tm tmCur = tmPre;

     bool bFifstLine{true};
     int iSizeMax =  holder->getViewGraphSize(iSelectedInterval);
     tmCur = *threadfree_localtime(&tTmp);


     std::time_t tBegTmp{0};
     std::time_t tNextTmp{0};

     if (iBeg  < iSizeMax && iSelectedInterval >= Bar::eInterval::p120){
         T &b = holder->getByIndex<T>(iSelectedInterval, iBeg);
         tBegTmp = b.Period();
         std::tm tmBegTmp = *threadfree_localtime(&tBegTmp);
         tmBegTmp.tm_mon--;
         if (tmBegTmp.tm_mon < 0){
             tmBegTmp.tm_mon = 11;
             tmBegTmp.tm_year--;
         }
         tNextTmp = std::mktime(&tmBegTmp);
     }
     else if (iBeg  < iSizeMax && iSelectedInterval >= Bar::eInterval::p15) {
         T &b = holder->getByIndex<T>(iSelectedInterval, iBeg);
         tBegTmp = b.Period();
         tNextTmp = tBegTmp > 86400 ? tBegTmp - 86400 : 0;
         ;
     }
     else if (iBeg  < iSizeMax && iSelectedInterval >= Bar::eInterval::p1) {
         T &b = holder->getByIndex<T>(iSelectedInterval, iBeg);
         tBegTmp = b.Period();
         tNextTmp = tBegTmp > 3600 ? tBegTmp - 3600 : 0;
     }
     else if (iBeg  < iSizeMax) {
         T &b = holder->getByIndex<T>(iSelectedInterval, iBeg);
         tBegTmp = b.Period();
         tNextTmp = tBegTmp > 60 ? tBegTmp - 60 : 0;
     }


     bool bSuccess{false};
     if (iSelectedInterval == Bar::eInterval::pTick){
         auto It  (holder->beginIteratorByDate<BarTick>(iSelectedInterval,tNextTmp,bSuccess));
         if (bSuccess){
             iLeftBeg = It.realPosition();
         }

     }
     else{
         auto It  (holder->beginIteratorByDate<Bar>(iSelectedInterval,tNextTmp,bSuccess));
         if (bSuccess){
             iLeftBeg = It.realPosition();
         }
     }


     QRectF rectQuotes =  ui->grViewQuotes->scene()->sceneRect();
     QRectF rectVolume =  ui->grViewVolume->scene()->sceneRect();
     /////
     bool bLineExists{false};
     int iInvalidate_X_BEG_1{iBeg};
     int iInvalidate_X_BEG_2{0};
     //
     /////
     // for tests
    // bool bFirstDraw{false}; // TODO delete (for tests)
     std::stringstream ss;
//     if (bHasTooltip && t != 0){
//         ss <<threadfree_localtime_to_str(&t);
//     }
     //
//     {
//         ThreadFreeCout pcout;
//         pcout << "=====================\n";
//         pcout << "begin:\t"<<iLeftBeg<<"\n";
//         pcout << "beg:\t"<<iBeg<<"\n";
//         pcout << "end:\t"<<iEnd<<"\n";
//     }
     for (int i = iLeftBeg; i<= iEnd; ++i){
         bLineExists = false;
         //
         //bFirstDraw = false;
         ss.str("");
         ss.clear();
//         auto ItSL = mVLinesViewQuotes.find(i);
//         if (ItSL != mVLinesViewQuotes.end() && ItSL->second.size() >0){
         auto ItSL = mVFramesHorisSmallScale.find(i);
         if (ItSL != mVFramesHorisSmallScale.end() && ItSL->second.size() >0){

             bLineExists = true;
             if (iInvalidate_X_BEG_2 == 0){
                 iInvalidate_X_BEG_2 = i;
//                 ThreadFreeCout pcout;
//                 bFirstDraw = true; // TODO delete (for tests)
//                 pcout << "begin exists:\t"<<i<<"\n";
             }
         }

         //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
         QGraphicsTextItem * itemNumb1 = new QGraphicsTextItem("0");
         itemNumb1->setFont(fontNumb);
         int iShiftNumb1 = ((itemNumb1->boundingRect().width() + 2) / BarGraphicsItem::BarWidth) / dHScale - 1;
         delete  itemNumb1;
         QGraphicsTextItem * itemNumb2 = new QGraphicsTextItem("00");
         itemNumb2->setFont(fontNumb);
         int iShiftNumb2 = ((itemNumb2->boundingRect().width() + 2) / BarGraphicsItem::BarWidth) / dHScale - 1;
         delete  itemNumb2;

         QGraphicsTextItem * itemTime = new QGraphicsTextItem("00:00");
         itemTime->setFont(fontTime);
         int iShiftTime = ((itemTime->boundingRect().width() + 2) / BarGraphicsItem::BarWidth) / dHScale - 1;
         delete  itemTime;
         //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
         //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
         //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
         xCur = (i + iLeftShift)     * BarGraphicsItem::BarWidth * dHScale;
         //xPre = (i - 1 + iLeftShift) * BarGraphicsItem::BarWidth * dHScale;
         //if (iLeftShift <= i && i - iLeftShift < iSizeMax){
         if ( i >= 0 && i  < iSizeMax ){

             T & tM = holder->getByIndex<T>(iSelectedInterval,i );
             tTmp = tM.Period();
             tmCur = *threadfree_localtime(&tTmp);
             //--------------------------
             if (!bFifstLine ){

                 //------------------------------------------------------------------------
                 // draw small lines down under :)
                 if (iBeg <= i && !bLineExists ){//&& false

                     //if (mVLinesHorisSmallScale.find(i) == mVLinesHorisSmallScale.end())
                     {
                         if (iFCount <= 0){
                              DrawLineToScene(i, xCur,0,xCur,1, mVFramesHorisSmallScale, ui->grViewScaleUpper->scene(),blackSolidPen,tTmp,true);
                         }
                         else {
                              DrawLineToScene(i, xCur,0,xCur,5, mVFramesHorisSmallScale, ui->grViewScaleUpper->scene(),blackSolidPen,tTmp,true);
                         }
                     }

//                     if (tmPre.tm_mday != tmCur.tm_mday || tmPre.tm_mon != tmCur.tm_mon || tmPre.tm_year != tmCur.tm_year){
//                         ThreadFreeCout pcout;
//                         pcout << "day line:\t"<<i<<"\n";
//                         pcout << "iSelectedInterval:\t"<<iSelectedInterval<<"\n";
//                         pcout << "iDayCounter:\t"<<iDayCounter<<"\n";

////                         if((tmPre.tm_mday != tmCur.tm_mday || tmPre.tm_mon != tmCur.tm_mon || tmPre.tm_year != tmCur.tm_year)
////                                                  && ((iSelectedInterval == Bar::eInterval::pDay && iDayCounter % 8 == 0 ) ||
////                                                      (iSelectedInterval == Bar::eInterval::p180 && iDayCounter % 4 == 0 ) ||
////                                                      (iSelectedInterval == Bar::eInterval::p120 && iDayCounter % 4 == 0 ) ||
////                                                      (iSelectedInterval < Bar::eInterval::p120)
////                                                      )
////                                                  ){
////                             pcout << "condition: true\n";
////                         }
////                         else{
////                             pcout << "condition: false\n";
////                         }

//                     }
                 }


                 //------------------------------------------------------------------------
                 //  draw year line
                 if(tmPre.tm_year != tmCur.tm_year){
//                     {
//                         ThreadFreeCout pcout;
//                        pcout << "condition: year {"<<i<<"}\n";
//                     }
                     iDayCounter = 0;

                     if (iBeg <= i && !bLineExists){
                         ss <<"Year\n";
                         ss <<threadfree_localtime_date_to_str(&tTmp);

                         DrawLineToScene             (i, xCur, 0,xCur,iLineH * 2, mVFramesScaleUpper, ui->grViewScaleUpper->scene(),blackSolidPen,tTmp,true); // down line

                         DrawLineToScene             (i, xCur, -iShiftH ,xCur, - rectVolume.height() + iShiftH * 2, mVFramesVolume, ui->grViewVolume ->scene(),blackDashPen,tTmp,true); // volume line
                         DrawIntermittentLineToScene (i, xCur, -iShiftH ,xCur, - rectQuotes.height() + iShiftH * 2, mVFramesViewQuotes, ui->grViewQuotes->scene(),blackDashPen,ss.str(),true); // middle line

                         DrawIntToScene(i, xCur, iLineH + iLineH/2 ,tmCur.tm_year + 1900,Qt::AlignmentFlag::AlignLeft, Qt::AlignmentFlag::AlignCenter,
                                        mVFramesScaleUpper, ui->grViewScaleUpper->scene(), fontNumb);
//                         if (!bFirstDraw){
//                             ThreadFreeCout pcout;
//                             bFirstDraw = true; // TODO delete (for tests)
//                             pcout << "draw year:\t"<<i<<"\tline exists"<<bLineExists<<"\n";
//                         }
                     }
                 }
                 //  draw month line
                 else if(tmPre.tm_mon != tmCur.tm_mon
                         && iSelectedInterval != Bar::eInterval::pMonth // for months don't draw
                         && (iSelectedInterval != Bar::eInterval::pWeek || tmCur.tm_mon % 3 ==0) // for weeks  - only draw only for every third
                         ){
//                     {
//                        ThreadFreeCout pcout;
//                        pcout << "condition: month {"<<i<<"}\n";
//                     }
                     iDayCounter = 0;

                     if (iBeg <= i && !bLineExists){
                         ss <<"Month\n";
                         ss <<threadfree_localtime_date_to_str(&tTmp);
                         //

                         DrawLineToScene             (i, xCur, 0,xCur,iLineH * 2, mVFramesScaleUpper, ui->grViewScaleUpper->scene(),blackSolidPen,tTmp,true); // down line
                         //
                         DrawLineToScene             (i, xCur, -iShiftH ,xCur, - rectVolume.height() + iShiftH * 2, mVFramesVolume, ui->grViewVolume ->scene(),blackDashPen,tTmp,true); // volume line
                         DrawIntermittentLineToScene (i, xCur, -iShiftH ,xCur, - rectQuotes.height() + iShiftH * 2, mVFramesViewQuotes, ui->grViewQuotes->scene(),blackDashPen,ss.str(),true); // middle line

                         DrawIntToScene(i, xCur, iLineH + iLineH/2,tmCur.tm_mon + 1 ,Qt::AlignmentFlag::AlignLeft, Qt::AlignmentFlag::AlignCenter,
                                        mVFramesScaleUpper, ui->grViewScaleUpper->scene(), fontNumb);

//                         if (!bFirstDraw){
//                             ThreadFreeCout pcout;
//                             bFirstDraw = true; // TODO delete (for tests)
//                             pcout << "draw month:\t"<<i<<"\tline exists"<<bLineExists<<"\n";
//                         }
                     }
                 }
                 //  draw day line
                 else if((tmPre.tm_mday != tmCur.tm_mday || tmPre.tm_mon != tmCur.tm_mon || tmPre.tm_year != tmCur.tm_year)
                         && ((iSelectedInterval == Bar::eInterval::pDay && iDayCounter % 8 == 0 ) ||
                             (iSelectedInterval == Bar::eInterval::p180 && iDayCounter % 4 == 0 ) ||
                             (iSelectedInterval == Bar::eInterval::p120 && iDayCounter % 4 == 0 ) ||
                             (iSelectedInterval < Bar::eInterval::p120)
                             )
                         ){
//                     {
//                        ThreadFreeCout pcout;
//                        pcout << "condition: day {"<<i<<"}\n";
//                     }
                     if (iBeg <= i && !bLineExists){
//                         {
//                            ThreadFreeCout pcout;
//                            pcout << "in day {"<<i<<"}\n";
//                         }
                         ss <<"Day\n";
                         ss <<threadfree_localtime_date_to_str(&tTmp);

                         DrawLineToScene             (i, xCur, -iShiftH ,xCur, - rectVolume.height() + iShiftH * 2, mVFramesVolume, ui->grViewVolume ->scene(),blackDashPen,tTmp,true); // volume line
                         DrawIntermittentLineToScene (i, xCur, -iShiftH ,xCur, - rectQuotes.height() + iShiftH * 2, mVFramesViewQuotes, ui->grViewQuotes->scene(),blackDashPen,ss.str(),true); // middle line

                         if (iFCount > 0)
                         {
                             DrawIntToScene(i, xCur, iLineH/2 ,tmCur.tm_mday,Qt::AlignmentFlag::AlignLeft, Qt::AlignmentFlag::AlignCenter,
                                            mVFramesScaleUpper, ui->grViewScaleUpper->scene(), fontNumb);
                         }
                         else{
                             DrawIntToScene(i, xCur, iLineH/2 ,0, Qt::AlignmentFlag::AlignLeft, Qt::AlignmentFlag::AlignCenter,
                                            mVFramesScaleUpper, ui->grViewScaleUpper->scene(), fontNumb);

                             ThreadFreeCout pcout;
                             pcout << "--------------\n";
                             pcout << "iSelectedInterval:\t"<<iSelectedInterval<<"\n";
                             pcout << "i:\t"<<i<<"\n";
                             pcout << "iBeg:\t"<<iBeg<<"\n";
                             pcout << "iLeftBeg:\t"<<iLeftBeg<<"\n";
                             pcout << "iFCount:\t"<<iFCount<<"\n";
                             pcout << "tBegTmp:\t"<<threadfree_localtime_to_str(&tBegTmp)<<"\n";
                             pcout << "tNextTmp:\t"<<threadfree_localtime_to_str(&tNextTmp)<<"\n";
                         }

//                         if (!bFirstDraw){
//                            ThreadFreeCout pcout;
//                            bFirstDraw = true; // TODO delete (for tests)
//                            pcout << "draw day:\t"<<i<<"\tline exists"<<bLineExists<<"\n";
//                         }
                     }
                     if (iFCount > 0){
                         if (tmCur.tm_mday <10)
                             iFCount = - iShiftNumb1;
                         else
                             iFCount = - iShiftNumb2;
                     }
                 }
                 //  draw interday
                 else if((!(tmPre.tm_mday != tmCur.tm_mday || tmPre.tm_mon != tmCur.tm_mon || tmPre.tm_year != tmCur.tm_year))
                         && ((iSelectedInterval == Bar::eInterval::p1  && tmCur.tm_min % 15 == 0 ) ||
                             (iSelectedInterval == Bar::eInterval::p5  && tmCur.tm_min  == 0 ) ||
                             (iSelectedInterval == Bar::eInterval::p10 && tmCur.tm_min == 0 && tmCur.tm_hour % 2 == 0 ) ||
                             (iSelectedInterval == Bar::eInterval::p15 && tmCur.tm_min == 0 && tmCur.tm_hour % 2 == 0 )
                             )
                         ){
//                     {
//                        ThreadFreeCout pcout;
//                        pcout << "condition: interday {"<<i<<"}\n";
//                     }
                     if (iBeg <= i && !bLineExists){
                         DrawLineToScene             (i, xCur, -iShiftH ,xCur, - rectVolume.height() + iShiftH * 2, mVFramesVolume, ui->grViewVolume ->scene(),blackDashPen,tTmp,true); // volume line
                         DrawIntermittentLineToScene (i, xCur, -iShiftH ,xCur, - rectQuotes.height() + iShiftH * 2, mVFramesViewQuotes, ui->grViewQuotes->scene(),blackDashPen,tTmp,true); // middle line

                         if (iFCount > 1)
                         {
                             DrawTimeToScene(i, xCur, -1 ,tmCur,mVFramesScaleUpper, ui->grViewScaleUpper->scene(), fontTime);
                         }
                     }

                     if (iFCount > 1)
                     {
                         iFCount = - iShiftTime;
                     }

                 }
                 //  draw ticks
                 else if(iSelectedInterval == Bar::eInterval::pTick && (tmPre.tm_min != tmCur.tm_min)){
//                     {
//                        ThreadFreeCout pcout;
//                        pcout << "condition: tick w/minit {"<<i<<"}\n";
//                     }

                     if (iBeg <= i && !bLineExists){
                         //
                         DrawLineToScene             (i, xCur, -iShiftH ,xCur, - rectVolume.height() + iShiftH * 2, mVFramesVolume, ui->grViewVolume ->scene(),blackDashPen,tTmp,true); // volume line
                         DrawIntermittentLineToScene (i, xCur, -iShiftH ,xCur, - rectQuotes.height() + iShiftH * 2, mVFramesViewQuotes, ui->grViewQuotes->scene(),blackDashPen,tTmp,true); // middle line

                         if (iFCount > 0)
                         {
                             DrawTimeToScene(i, xCur, -1 ,tmCur,mVFramesScaleUpper, ui->grViewScaleUpper->scene(), fontTime);
                         }

                     }
                     if (iFCount > 0)
                     {
                         iFCount = - iShiftTime;
                     }

                     iSecCounter = 0;
                 }
                 else if(iSelectedInterval == Bar::eInterval::pTick
                         && (tmPre.tm_sec != tmCur.tm_sec /*&& iSecCounter >=8 */)
                         ){
                     //
//                     {
//                        ThreadFreeCout pcout;
//                        pcout << "condition: tick {"<<i<<"}\n";
//                     }
                     if (iBeg <= i && !bLineExists){
                        DrawIntermittentLineToScene(i, xCur, -iShiftH ,xCur, - rectQuotes.height() + iShiftH * 2, mVFramesViewQuotes, ui->grViewQuotes->scene(),blackDotPen,tTmp,true); // middle line
                     }
                     iSecCounter = 0;
                 }
                 else{
//                     {
//                        ThreadFreeCout pcout;
//                        pcout << "condition: out {"<<i<<"}\n";
//                     }
//                     if ((tmPre.tm_mday != tmCur.tm_mday || tmPre.tm_mon != tmCur.tm_mon || tmPre.tm_year != tmCur.tm_year)
//                             && iSelectedInterval == Bar::eInterval::p120){
//                         if (!bFirstDraw){
//                            ThreadFreeCout pcout;
//                             bFirstDraw = true; // TODO delete (for tests)
//                             pcout << "silent day:\t"<<i<<"\tline exists"<<bLineExists<<"\n";
//                             pcout << "iDayCounter:\t"<<iDayCounter<<"\t\%"<<(iDayCounter % 4)<<"\n";
//                         }
//                     }

                 }
                 ///

             }
             else{
                 bFifstLine = false;
             }

             if (tmPre.tm_mday != tmCur.tm_mday || tmPre.tm_mon != tmCur.tm_mon || tmPre.tm_year != tmCur.tm_year){
                 iDayCounter++;
             }
         }
         else{
             if (!bLineExists){
                 DrawLineToScene(i, xCur,0,xCur,5, mVFramesHorisSmallScale, ui->grViewScaleUpper->scene(),blackSolidPen);
             }
             //bFifstLine = false;
         }
         //------------------------------------------------------------------------
         // draw horisontal line in ScaleViews
         if (iBeg <= i && !bLineExists){
             //DrawLineToScene(i, xCur,0,xPre,0, mVLinesScaleUpper, ui->grViewScaleUpper->scene(),blackSolidPen,0);
            // DrawLineToScene(i, xCur,iLineH,xPre,iLineH, mVLinesScaleUpper, ui->grViewScaleUpper->scene(),blackSolidPen,0);
         }
         //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
         //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
         //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

         iFCount++;
         iSecCounter++;
         tmPre = tmCur;
     }

     QRectF rec =  ui->grViewQuotes->scene()->sceneRect();

     QRectF newRec((iInvalidate_X_BEG_1 + iLeftShift) * BarGraphicsItem::BarWidth * dHScale,
                   rec.y(),
                   (iInvalidate_X_BEG_2 + iLeftShift) * BarGraphicsItem::BarWidth * dHScale,
                   rec.height());

     ui->grViewQuotes->scene()->invalidate(newRec);

 }
 //---------------------------------------------------------------------------------------------------------------
 void GraphViewForm::PaintHorizontalScales()
 {


     QColor color(204, 122, 0,155);// orange
     QPen Pen(color,1,Qt::DashLine);

     QPen blackSolidPen(Qt::black,0.5,Qt::SolidLine);
     QPen blackGrayPen(Qt::black,0.5,Qt::SolidLine);
     QPen blackDashPen(Qt::gray,1,Qt::DashLine);
     QPen blackDotPen(Qt::gray,1,Qt::DotLine);
     QPen simpleDashPen(Qt::gray,1,Qt::DashLine);

     int iLineH = ui->grViewScaleUpper->scene()->sceneRect().height()/2;


     if (vHorizFramesScaleUpper.size() ==0)
     {
         QGraphicsLineItem *item = new QGraphicsLineItem(0,0,ui->grViewScaleUpper->scene()->sceneRect().width(),0);
         item->setPen(Pen);
         vHorizFramesScaleUpper.push_back({item});
         ui->grViewScaleUpper->scene()->addItem(item);
         item->setPos(0,0);

         item = new QGraphicsLineItem(0,0,ui->grViewScaleUpper->scene()->sceneRect().width(),0);
         item->setPen(blackSolidPen);
         vHorizFramesScaleUpper.push_back({item});
         ui->grViewScaleUpper->scene()->addItem(item);
         item->setPos(0,iLineH);

         item = new QGraphicsLineItem(0,0,ui->grViewScaleUpper->scene()->sceneRect().width(),0);
         item->setPen(blackGrayPen);
         vHorizFramesScaleUpper.push_back({item});
         ui->grViewScaleUpper->scene()->addItem(item);
         item->setPos(0,iLineH * 2 -1);


     }




 }
 //---------------------------------------------------------------------------------------------------------------
 void GraphViewForm::SetSliderToPos(std::time_t tRightPos, int iRightAggregate)
 {
     if (iSelectedInterval == Bar::eInterval::pTick){
         SetSliderToPosT<BarTick>(tRightPos, iRightAggregate);
     }
     else{
         SetSliderToPosT<Bar>(tRightPos, iRightAggregate);
     }
 }
 //---------------------------------------------------------------------------------------------------------------
 template<typename T>
 void GraphViewForm::SetSliderToPosT(std::time_t tRightPos, int iRightAggregate)
 {
     bool bSuccess;
     auto It = holder->beginIteratorByDate<T>(iSelectedInterval,tRightPos,bSuccess);
     auto ItEnd (holder->end<T>());
     if (bSuccess ){
         int xCur{0};

         if (It != ItEnd){
             xCur = (It.realPosition() + iLeftShift - 1) * BarGraphicsItem::BarWidth * dHScale - ui->grViewQuotes->horizontalScrollBar()->pageStep();
             xCur += iRightAggregate * BarGraphicsItem::BarWidth * dHScale;
         }
         else{
             int iSize = holder->getViewGraphSize(iSelectedInterval);
             xCur = (iSize + iLeftShift - 1) * BarGraphicsItem::BarWidth * dHScale - ui->grViewQuotes->horizontalScrollBar()->pageStep();
             xCur += iRightAggregate * BarGraphicsItem::BarWidth * dHScale;
         }

        ui->grViewScaleUpper->horizontalScrollBar()->setValue(xCur);
        ui->grViewVolume->horizontalScrollBar()->setValue(xCur);
        //ui->grViewScaleLower->horizontalScrollBar()->setValue(xCur);
        ui->grViewQuotes->horizontalScrollBar()->setValue(xCur);
        ui->grHorizScroll->horizontalScrollBar()->setValue(xCur);
     }
 }
 //---------------------------------------------------------------------------------------------------------------
 //---------------------------------------------------------------------------------------------------------------
 //---------------------------------------------------------------------------------------------------------------
 //---------------------------------------------------------------------------------------------------------------
 //---------------------------------------------------------------------------------------------------------------
 //---------------------------------------------------------------------------------------------------------------
 //---------------------------------------------------------------------------------------------------------------


