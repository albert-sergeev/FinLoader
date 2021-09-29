#include "graphviewform.h"
#include "ui_graphviewform.h"
#include "storage.h"

#include<QScrollBar>
#include<QWheelEvent>
#include<QMessageBox>
#include<QSizePolicy>

#include<sstream>
#include<iomanip>
#include<cmath>

#include "threadfreelocaltime.h"
#include "threadfreecout.h"
#include "plusbutton.h"

using namespace std::chrono_literals;
using seconds=std::chrono::duration<double>;
using milliseconds=std::chrono::duration<double,
    std::ratio_multiply<seconds::period,std::milli>
    >;


//---------------------------------------------------------------------------------------------------------------
GraphViewForm::GraphViewForm(const int TickerID, std::vector<Ticker> &v, std::shared_ptr<GraphHolder> hldr, QWidget *parent) :
    QWidget(parent),
    iTickerID{TickerID},
    tTicker{0,"","",1},
    vTickersLst{v},
    pathBlue{nullptr},
    pathRed{nullptr},
    pathGreen{nullptr},
    aiInvalidateCounter{0},
    defInit{aiInvalidateCounter},
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

    //-------------------------------------------------------------
    fontTime.setPixelSize(11);
    fontTime.setBold(false);
    fontTime.setFamily("Times New Roman");

    fontNumb.setPixelSize(14);
    fontNumb.setBold(false);
    fontNumb.setFamily("Times New Roman");
    //-------------------------------------------------------------

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
    connect(swtCandle,SIGNAL(stateChanged(int)),this,SLOT(slotCandleStateChanged(int)));
    //-------------------------------------------------------------
    QHBoxLayout *ltView =  new QHBoxLayout();
    ltView->setContentsMargins(0,0,0,0);
    ltView->setMargin(3);
    ui->grViewL2->setLayout(ltView);
    QLabel *lblSign = new QLabel(tr("SIGN"));
    lblSign->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    lblSign->setAlignment(Qt::AlignCenter);
    ltView->addWidget(lblSign);
    //-------------------------------------------------------------
    QHBoxLayout *ltViewR1 =  new QHBoxLayout();
    ltViewR1->setContentsMargins(0,0,0,0);
    ltViewR1->setMargin(3);
    ui->grViewR4->setLayout(ltViewR1);
    QLabel *lblSignR = new QLabel(tr("SIGN"));
    lblSignR->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    lblSignR->setAlignment(Qt::AlignCenter);
    ltViewR1->addWidget(lblSignR);
    //-------------------------------------------------------------
    QHBoxLayout *ltView2 =  new QHBoxLayout();
    ltView2->setContentsMargins(0,0,0,0);
    ltView2->setMargin(3);
    ui->grViewL3->setLayout(ltView2);
    btnHelp = new QPushButton(tr("Help"));
    btnHelp->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    ltView2->addWidget(btnHelp);
    //btnHelp->setVisible(false);
    //-------------------------------------------------------------
    QHBoxLayout *ltViewR2 =  new QHBoxLayout();
    ltViewR2->setContentsMargins(0,0,0,0);
    ltViewR2->setMargin(3);
    ui->grViewL4->setLayout(ltViewR2);
    btnHelpR = new QPushButton(tr("Help"));
    btnHelpR->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    ltViewR2->addWidget(btnHelpR);
    //btnHelp->setVisible(false);
    //-------------------------------------------------------------
    indicatorMemo = new Memometer(this);
    indicatorMemo->setValue(0);
    indicatorMemo->setToolTipText(QString(tr("Memory usage: 0")).toStdString());
    //-------------------------------------------------------------

    btnScaleHViewPlus       = new PlusButton(true,this);
    btnScaleHViewMinus      = new PlusButton(false,this);

    btnScaleVViewPlus       = new PlusButton(true,this);
    btnScaleVViewMinus      = new PlusButton(false,this);

    btnScaleVVolumePlus     = new PlusButton(true,this);
    btnScaleVVolumeMinus    = new PlusButton(false,this);

    QFont fontDef;
    fontDef.setPixelSize(8);
    fontDef.setBold(false);
    fontDef.setFamily("Times New Roman");

    btnScaleHViewDefault    = new TransparentButton(tr("DEFAULT"),this);
    btnScaleVViewDefault    = new TransparentButton(tr("DEFAULT"),this);
    btnScaleVVolumeDefault  = new TransparentButton(tr("DEFAULT"),this);

    btnScaleHViewDefault->setFont(fontDef);
    btnScaleVViewDefault->setFont(fontDef);
    btnScaleVVolumeDefault->setFont(fontDef);

    btnScaleHViewDefault->installEventFilter(this);
    btnScaleVViewDefault->installEventFilter(this);
    btnScaleVVolumeDefault->installEventFilter(this);


    connect(btnScaleHViewPlus,SIGNAL(clicked(bool)),this,SLOT(slotHScaleQuotesClicked(bool)));
    connect(btnScaleHViewMinus,SIGNAL(clicked(bool)),this,SLOT(slotHScaleQuotesClicked(bool)));

    connect(btnScaleVViewPlus,SIGNAL(clicked(bool)),this,SLOT(slotVScaleQuotesClicked(bool)));
    connect(btnScaleVViewMinus,SIGNAL(clicked(bool)),this,SLOT(slotVScaleQuotesClicked(bool)));

    connect(btnScaleVVolumePlus,SIGNAL(clicked(bool)),this,SLOT(slotVScaleVolumeClicked(bool)));
    connect(btnScaleVVolumeMinus,SIGNAL(clicked(bool)),this,SLOT(slotVScaleVolumeClicked(bool)));


    btnScaleHViewPlus->setToolTip(tr("increase the horizontal scale of the graph"));
    btnScaleHViewMinus->setToolTip(tr("reduce the horizontal scale of the graph"));

    btnScaleVViewPlus->setToolTip(tr("increase the vertical scale of the graph"));
    btnScaleVViewMinus->setToolTip(tr("reduce the vertical scale of the graph"));

    btnScaleVVolumePlus->setToolTip(tr("increase the vertical scale of the volume graph"));
    btnScaleVVolumeMinus->setToolTip(tr("reduce the vertical scale of the volume graph"));

    btnScaleHViewDefault->setToolTip(tr("Set default the horizontal scale of the graph"));
    btnScaleVViewDefault->setToolTip(tr("Set default the vertical scale of the graph"));
    btnScaleVVolumeDefault->setToolTip(tr("Set default the vertical scale of the volume graph"));

    connect(btnScaleHViewDefault,SIGNAL(clicked()),this,SLOT(slotScaleHViewDefaultClicked()));
    connect(btnScaleVViewDefault,SIGNAL(clicked()),this,SLOT(slotScaleVViewDefaultClicked()));
    connect(btnScaleVVolumeDefault,SIGNAL(clicked()),this,SLOT(slotScaleVVolumeDefaultClicked()));


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
    lblSign->setText(QString::fromStdString(tTicker.TickerSign()));
    lblSignR->setText(QString::fromStdString(tTicker.TickerSign()));
    //-------------------------------------------------------------------------------------
    iSelectedInterval   = tTicker.StoredSelectedInterval();
    dStoredVValue       = tTicker.StoredVValue();
    dHScale             = tTicker.StoredHScale() == 0 ? 1 : tTicker.StoredHScale();
    bOHLC               = tTicker.OHLC();
    swtCandle->setChecked(bOHLC);

    if (dHScale == 1) btnScaleHViewDefault->hide();
    else btnScaleHViewDefault->show();

    tStoredRightPointPosition           = tTicker.StoredTimePosition();
    iStoredRightAggregate               = tTicker.StoredRightAggregate();

    mVScale[Bar::eInterval::pTick]      = tTicker.StoredVScale(Bar::eInterval::pTick);
    mVScale[Bar::eInterval::p1]         = tTicker.StoredVScale(Bar::eInterval::p1);
    mVScale[Bar::eInterval::p5]         = tTicker.StoredVScale(Bar::eInterval::p5);
    mVScale[Bar::eInterval::p10]        = tTicker.StoredVScale(Bar::eInterval::p10);
    mVScale[Bar::eInterval::p15]        = tTicker.StoredVScale(Bar::eInterval::p15);
    mVScale[Bar::eInterval::p30]        = tTicker.StoredVScale(Bar::eInterval::p30);
    mVScale[Bar::eInterval::p60]        = tTicker.StoredVScale(Bar::eInterval::p60);
    mVScale[Bar::eInterval::p120]       = tTicker.StoredVScale(Bar::eInterval::p120);
    mVScale[Bar::eInterval::p180]       = tTicker.StoredVScale(Bar::eInterval::p180);
    mVScale[Bar::eInterval::pDay]       = tTicker.StoredVScale(Bar::eInterval::pDay);
    mVScale[Bar::eInterval::pWeek]      = tTicker.StoredVScale(Bar::eInterval::pWeek);
    mVScale[Bar::eInterval::pMonth]     = tTicker.StoredVScale(Bar::eInterval::pMonth);

    mVVolumeScale[Bar::eInterval::pTick]      = tTicker.StoredVVolumeScale(Bar::eInterval::pTick);
    mVVolumeScale[Bar::eInterval::p1]         = tTicker.StoredVVolumeScale(Bar::eInterval::p1);
    mVVolumeScale[Bar::eInterval::p5]         = tTicker.StoredVVolumeScale(Bar::eInterval::p5);
    mVVolumeScale[Bar::eInterval::p10]        = tTicker.StoredVVolumeScale(Bar::eInterval::p10);
    mVVolumeScale[Bar::eInterval::p15]        = tTicker.StoredVVolumeScale(Bar::eInterval::p15);
    mVVolumeScale[Bar::eInterval::p30]        = tTicker.StoredVVolumeScale(Bar::eInterval::p30);
    mVVolumeScale[Bar::eInterval::p60]        = tTicker.StoredVVolumeScale(Bar::eInterval::p60);
    mVVolumeScale[Bar::eInterval::p120]       = tTicker.StoredVVolumeScale(Bar::eInterval::p120);
    mVVolumeScale[Bar::eInterval::p180]       = tTicker.StoredVVolumeScale(Bar::eInterval::p180);
    mVVolumeScale[Bar::eInterval::pDay]       = tTicker.StoredVVolumeScale(Bar::eInterval::pDay);
    mVVolumeScale[Bar::eInterval::pWeek]      = tTicker.StoredVVolumeScale(Bar::eInterval::pWeek);
    mVVolumeScale[Bar::eInterval::pMonth]     = tTicker.StoredVVolumeScale(Bar::eInterval::pMonth);

    //-------------------------------------------------------------------------------------

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

    std::time_t dtBegin{0};
    std::time_t dtEnd{0};
    bool bSuccess{false};

    {   // lock holder
        auto ItDefender(holder->beginIteratorByDate<BarTick>(iSelectedInterval,0,bSuccess));
        if (bSuccess){
            dtBegin = holder->getViewGraphDateMin(Bar::eInterval::pTick);
            dtEnd   = holder->getViewGraphDateMax(Bar::eInterval::pTick);
            ItDefender.unlock();
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


    ui->grViewQuotes->horizontalScrollBar()->setInvertedControls(false);
    ui->grHorizScroll->horizontalScrollBar()->setInvertedControls(false);
    ui->grViewVolume->horizontalScrollBar()->setInvertedControls(false);


    ui->grViewL1->installEventFilter(this);
    ui->grViewL2->installEventFilter(this);
    ui->grViewL3->installEventFilter(this);
    ui->grViewL4->installEventFilter(this);
    ui->grViewR1->installEventFilter(this);
    ui->grViewR2->installEventFilter(this);
    ui->grViewR3->installEventFilter(this);
    ui->grViewR4->installEventFilter(this);

    ui->grViewQuotes->installEventFilter(this);
    ui->grViewScaleUpper->installEventFilter(this);
    ui->grViewVolume->installEventFilter(this);
    ui->grViewScaleLower->installEventFilter(this);

    ui->grViewQuotes->verticalScrollBar()->installEventFilter(this);
    ui->grViewScaleUpper->verticalScrollBar()->installEventFilter(this);
    ui->grViewVolume->verticalScrollBar()->installEventFilter(this);
    ui->grViewScaleLower->verticalScrollBar()->installEventFilter(this);

    ui->grViewQuotes->horizontalScrollBar()->installEventFilter(this);
    ui->grViewScaleUpper->horizontalScrollBar()->installEventFilter(this);
    ui->grViewVolume->horizontalScrollBar()->installEventFilter(this);
    ui->grViewScaleLower->horizontalScrollBar()->installEventFilter(this);

    ui->grViewL1->installEventFilter(this);

    dtFastShowAverageActivity = std::chrono::steady_clock::now();

   // {ThreadFreeCout pcout; pcout<<"const out\n";}

//    connect(ui->btnTestLoad,SIGNAL(clicked()),this,SLOT(slotLoadGraphButton()));


}
//---------------------------------------------------------------------------------------------------------------
GraphViewForm::~GraphViewForm()
{
    if(swtCandle)   {delete swtCandle;  swtCandle = nullptr;}

    delete ui;
}
//---------------------------------------------------------------------------------------------------------------
//void GraphViewForm::slotLoadGraphButton()
//{
//    std::time_t tNow =std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
//    std::time_t tBegin = Storage::dateAddMonth(tNow,-12);
//    std::time_t tEnd = tNow;

//    emit NeedLoadGraph(iTickerID, tBegin, tEnd);
//}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::slotInvalidateGraph(std::time_t dtBegin, std::time_t dtEnd, bool bNeedToRescale)
{
    queueRepaint.Push({dtBegin,dtEnd,bNeedToRescale});
    slotProcessRepaintQueue();
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::slotProcessRepaintQueue()
{
    bool bSuccess{false};
    auto pData (queueRepaint.Pop(bSuccess));
    while(bSuccess)
    {
        auto data(*pData.get());
        //////
        if(bSuccess && data.Type == RepainTask::eRepaintType::InvalidateRepaint){
            if (iSelectedInterval == Bar::eInterval::pTick) {
                bSuccess = RepainInvalidRange<BarTick>(data);
            }
            else{
                bSuccess = RepainInvalidRange<Bar>(data);
            }
        }
        else if (bSuccess && data.Type & RepainTask::eRepaintType::PaintViewport){
            bool bFrames    = data.Type & RepainTask::eRepaintType::FastFrames;
            bool bBars      = data.Type & RepainTask::eRepaintType::FastBars;
            bool bVolumes   = data.Type & RepainTask::eRepaintType::FastVolumes;
            bSuccess = PaintViewPort(bFrames,bBars,bVolumes,data.bStoreRightPos,data.bInvalidate);
        }
        else{
            if(bSuccess && (data.Type &(RepainTask::eRepaintType::FastBars | RepainTask::eRepaintType::FastVolumes))){
                bSuccess = FastPaintBars(data);
            }
            if(bSuccess && (data.Type & RepainTask::eRepaintType::FastFrames)){
                bSuccess = FastPaintFrames(data);
            }
            if(bSuccess && (data.Type & RepainTask::eRepaintType::FastAverages)){
                bSuccess = FastPaintAverages(data);
            }
        }
        ///////
        if(!bSuccess){ // if not, postpone for the future
            queueRepaint.Push(data);
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
    auto It (holder->beginIteratorByDate<T>(iSelectedInterval,Bar::DateAccommodate(data.dtStart,iSelectedInterval),bSuccess));
    if (bSuccess){
        auto ItEnd (holder->beginIteratorByDate<T>(iSelectedInterval,Bar::DateAccommodate(data.dtEnd,iSelectedInterval),bSuccess));
        if (bSuccess){
            {
                ThreadFreeCout pcout;
                pcout <<"do invalidate {"<<data.Type<<"} ";
                pcout <<"sign {"<<tTicker.TickerSign()<<"}\n";


//                pcout <<"data.dtStart: "<<threadfree_gmtime_to_str(&data.dtStart)<<"\n";
//                pcout <<"data.dtEnd: "<<threadfree_gmtime_to_str(&data.dtEnd)<<"\n";
            }
            InvalidateCounterDefender def(aiInvalidateCounter);
            auto ItTotalEnd (holder->end<T>());

            if (It == ItTotalEnd || It.realPosition() > ItEnd.realPosition()) {
                return true;
            }
            //////
            /// \brief ItTotalEnd
            ///{ThreadFreeCout pcout; pcout<<"inv 1";}


            int iSize =  (int)holder->getViewGraphSize(iSelectedInterval);
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
                if ((dStoredVolumeHighMax /*- dStoredVolumeLowMin*/) <= 0){
                    mVVolumeScale[iSelectedInterval] = 1.0;
                }
                else{
                    mVVolumeScale[iSelectedInterval] = (ui->grViewVolume->maximumHeight() - iVolumeViewPortHighStrip * 2)/(dStoredVolumeHighMax /*- dStoredVolumeLowMin*/);
                }
            }

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

            // clean invalid range
            int iEnd = (int)ItEnd.realPosition() + 1;
            iEnd = iEnd > iSize ? iSize : iEnd;
            //
            EraseLinesMid(mShowedGraphicsBars,  (int)It.realPosition(),iEnd, ui->grViewQuotes->scene());
            EraseLinesMid(mShowedVolumes,       (int)It.realPosition(),iEnd, ui->grViewVolume->scene());


            if (!bWasTotalErase && (int)It.realPosition() < iEnd){

                qreal x1 = (It.realPosition() + iLeftShift ) * BarGraphicsItem::BarWidth * dHScale;
                qreal x2 = (iEnd + iLeftShift ) * BarGraphicsItem::BarWidth * dHScale;

                QRectF recS = ui->grViewQuotes->scene()->sceneRect();
                QRectF invS  = QRectF(x1,recS.y(),x2,recS.width());

                QRectF recVS = ui->grViewVolume->scene()->sceneRect();
                QRectF invVS  = QRectF(x1,recVS.y(),x2,recVS.width());

                ui->grViewQuotes->scene()->invalidate(invS);
                ui->grViewVolume->scene()->invalidate(invVS);

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
                disconnect(ui->grHorizScroll->horizontalScrollBar(),SIGNAL(valueChanged(int)),this,SLOT(slotHorizontalScrollBarValueChanged(int)));
                grScene->setSceneRect(newRec);
                connect(ui->grHorizScroll->horizontalScrollBar(),SIGNAL(valueChanged(int)),this,SLOT(slotHorizontalScrollBarValueChanged(int)));

                EraseFrames();

                ReDoHLines();
            }

            // resize Scene rect for Volume
            if (iStoredMaxSize !=iSize || bNeedRescale || data.bNeedToRescale){

                int iNewVolumeSceneH = (mVVolumeScale.at(iSelectedInterval) * (dStoredVolumeHighMax - dStoredVolumeLowMin) + iVolumeViewPortHighStrip);
                if (ui->grViewVolume->maximumHeight() > iNewVolumeSceneH){
                    iNewVolumeSceneH = ui->grViewVolume->maximumHeight();
                }

                QRectF newRec(0,-iNewVolumeSceneH,
                              (iSize + iLeftShift + iRightShift) * BarGraphicsItem::BarWidth * dHScale  ,
                              iNewVolumeSceneH
                              );

                ui->grViewVolume->scene()->setSceneRect(newRec);
            }

            iStoredMaxSize  = iSize;
            tStoredMinDate  = tMinDate;
            tStoredMaxDate  = tMaxDate;

            //---------------------------
            def.free();

            if (!this->isHidden()){

                if (tStoredRightPointPosition != 0){
                    SetSliderToPos(tStoredRightPointPosition, iStoredRightAggregate);
                }
                else{
                    SetSliderToPos(tStoredMaxDate, iRightShift);
                }
                SetSliderToVertPos(dStoredVValue);
            }
            PaintViewPort   (true,true,true, false,true);
        }
    }
    return bSuccess;
}

//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::resizeEvent(QResizeEvent *event)
{
    InvalidateCounterDefender def(aiInvalidateCounter);
    ///////////
    RepositionPlusMinusButtons();
    ResizeMemometer();

    QWidget::resizeEvent(event);
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::showEvent(QShowEvent *event)
{


    if (tStoredRightPointPosition != 0){
        SetSliderToPos(tStoredRightPointPosition, iStoredRightAggregate);
    }
    else{
        SetSliderToPos(tStoredMaxDate, iRightShift);
    }
    defInit.free();
    SetSliderToVertPos(dStoredVValue);
    ///////////
    RepositionPlusMinusButtons();
    ResizeMemometer();

    QWidget::showEvent(event);
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::slotSceneRectChanged( const QRectF & rect)
{
    RefreshHLines();

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

    if(iStoredMaxSize > 0 && aiInvalidateCounter == 0){
        dStoredVValue = sceneYtoRealY(-iV) - (sceneYtoRealY(-iV) - sceneYtoRealY(-iV - ui->grViewQuotes->verticalScrollBar()->pageStep())) / 2 ;
    }
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::slotHorizontalScrollBarValueChanged(int iH)
{

    ui->grViewScaleUpper->horizontalScrollBar()->setValue(iH);
    ui->grViewVolume->horizontalScrollBar()->setValue(iH);
    ui->grViewScaleLower->horizontalScrollBar()->setValue(iH);
    ui->grViewQuotes->horizontalScrollBar()->setValue(iH);
    ui->grHorizScroll->horizontalScrollBar()->setValue(iH);

    PaintViewPort(true,true,true,true,false);
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
                                    std::map<int,std::vector<QGraphicsItem *>>& mM, QGraphicsScene *scene, const QFont & font, const qreal zvalue)
{
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) <<n;

     QGraphicsTextItem * item = new QGraphicsTextItem(QString::fromStdString(ss.str()));
     item->setFont(font);
     mM[idx].push_back(item);
     item->setZValue(zvalue);
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
                                    std::map<int,std::vector<QGraphicsItem *>>& mM, QGraphicsScene *scene, const QFont & font, const qreal zvalue)
{
     QGraphicsTextItem * item = new QGraphicsTextItem(QString::number(n));
     item->setFont(font);
     mM[idx].push_back(item);
     item->setZValue(zvalue);
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
                                    std::map<int,std::vector<QGraphicsItem *>>& mM, QGraphicsScene *scene, const QFont & font, const qreal zvalue)
{
   std::stringstream ss;
   ss <<std::setfill('0');
   ss << std::setw(2)<<tmT.tm_hour<<":";
   ss << std::setw(2)<<tmT.tm_min;

    QGraphicsTextItem * item = new QGraphicsTextItem(QString::fromStdString(ss.str()));
    item->setFont(font);
    mM[idx].push_back(item);
    item->setZValue(zvalue);
    scene->addItem(item);
    item->setPos(x,y);

}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::DrawIntermittentLineToScene(const int idx,const  qreal x1,const  qreal y1,const qreal x2,const  qreal y2,
                     std::map<int,std::vector<QGraphicsItem *>>& mM, QGraphicsScene *scene, const QPen & pen,
                     const  std::time_t t, bool bHasTooltip, const qreal zvalue)
{
    std::stringstream ss;
    if (bHasTooltip && t != 0){
        ss <<threadfree_gmtime_to_str(&t);
    }

    DrawIntermittentLineToScene(idx,x1,y1,x2,y2,mM,scene, pen,ss.str(),  bHasTooltip,zvalue);
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::DrawIntermittentLineToScene(const int idx,const  qreal x1,const  qreal y1,const qreal x2,const  qreal y2,
                     std::map<int,std::vector<QGraphicsItem *>>& mM, QGraphicsScene *scene, const QPen & pen,
                     const  std::string sToolTip, bool bHasTooltip, const qreal zvalue)
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
            DrawLineToScene(idx,x1,s.first - 6 , x2, s.second + 6, mM, scene,pen,sToolTip, bHasTooltip,zvalue);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::DrawLineToScene(const int idx,const  qreal x1,const  qreal y1,const qreal x2,const  qreal y2,
                                    std::map<int,std::vector<QGraphicsItem *>>& mM, QGraphicsScene *scene,
                                    const QPen & pen, const  std::time_t t, bool bHasTooltip, const qreal zvalue)
{
    std::stringstream ss;
    if (bHasTooltip && t != 0){
        ss <<threadfree_gmtime_to_str(&t);
    }
    DrawLineToScene(idx,x1,y1,x2,y2,mM, scene,pen,ss.str(), bHasTooltip,zvalue);
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::DrawLineToScene(const int idx,const  qreal x1,const  qreal y1,const qreal x2,const  qreal y2,
                     std::map<int,std::vector<QGraphicsItem *>>& mM, QGraphicsScene *scene, const QPen & pen,
                     const  std::string sToolTip, bool bHasTooltip, const qreal zvalue)
{

    QGraphicsLineItem *item = new QGraphicsLineItem(0,0,x2-x1,y2-y1);
    item->setPen(pen);
    mM[idx].push_back(item);

    item->setZValue(zvalue);
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
    if (iStart > iEnd) return;

    if constexpr (std::is_same_v<T, std::map<int,std::pair<std::time_t,bool>>>){
        auto It     (mM.lower_bound(iStart));
        auto ItEnd  (mM.upper_bound(iEnd));
        mM.erase(It,ItEnd);
    }
    else if constexpr (std::is_same_v<T, std::map<int,QPointF>>){
        auto It     (mM.lower_bound(iStart));
        auto ItEnd  (mM.upper_bound(iEnd));
        mM.erase(It,ItEnd);
    }
    else{
        auto It     (mM.lower_bound(iStart));
        auto ItCp   (It);
        auto ItEnd  (mM.upper_bound(iEnd));

        while( It != ItEnd){
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
}
//---------------------------------------------------------------------------------------------------------------
template<typename T>
void GraphViewForm::EraseLinesLower(T& mM, int iStart, QGraphicsScene * scene)
//void GraphViewForm::EraseLinesLower(std::map<int,std::vector<QGraphicsItem *>>& mM, int iStart, QGraphicsScene * scene)
{
    if constexpr (std::is_same_v<T, std::map<int,std::pair<std::time_t,bool>>>){
        auto It (mM.lower_bound(iStart));
        mM.erase(It,mM.end());
    }
    else if constexpr (std::is_same_v<T, std::map<int,QPointF>>){
        auto It (mM.lower_bound(iStart));
        mM.erase(It,mM.end());
    }
    else{
        auto It (mM.lower_bound(iStart));
        if (It == mM.end()) return;
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
}
//---------------------------------------------------------------------------------------------------------------
template<typename T>
void GraphViewForm::EraseLinesUpper(T& mM, int iEnd, QGraphicsScene *scene)
//void GraphViewForm::EraseLinesUpper(std::map<int,std::vector<QGraphicsItem *>>& mM, int iEnd, QGraphicsScene *scene)
{
    if constexpr (std::is_same_v<T, std::map<int,std::pair<std::time_t,bool>>>){
        auto ItEnd (mM.upper_bound(iEnd));
        mM.erase(mM.begin(),ItEnd);
    }
    else if constexpr (std::is_same_v<T, std::map<int,QPointF>>){
        auto ItEnd (mM.upper_bound(iEnd));
        mM.erase(mM.begin(),ItEnd);
    }
    else{
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

        slotSetNewHScaleQuotes(dHScale);

//        Erase();

//        int iNewWidth =  (iStoredMaxSize + iLeftShift + iRightShift) * BarGraphicsItem::BarWidth * dHScale;

//        QRectF newRec(0,ui->grViewQuotes->scene()->sceneRect().y(),
//                      iNewWidth  ,
//                      ui->grViewQuotes->scene()->sceneRect().height()
//                      );
//        disconnect(ui->grHorizScroll->horizontalScrollBar(),SIGNAL(valueChanged(int)),this,SLOT(slotHorizontalScrollBarValueChanged(int)));
//        grScene->setSceneRect(newRec);
//        connect(ui->grHorizScroll->horizontalScrollBar(),SIGNAL(valueChanged(int)),this,SLOT(slotHorizontalScrollBarValueChanged(int)));

//        PaintViewPort(true,true,true,false,false);

//        if (tStoredRightPointPosition != 0){
//            SetSliderToPos(tStoredRightPointPosition, iStoredRightAggregate);
//        }
//        else{
//            SetSliderToPos(tStoredMaxDate, iRightShift);
//        }


//        ui->grViewQuotes->scene()->invalidate(ui->grViewQuotes->sceneRect());
//        ui->grViewVolume->scene()->invalidate(ui->grViewVolume->sceneRect());
    }
}
//---------------------------------------------------------------------------------------------------------------

void GraphViewForm::slotSetNewHScaleQuotes(double dNewScale)
{
    dHScale = dNewScale;

    Erase();

    int iNewWidth =  (iStoredMaxSize + iLeftShift + iRightShift) * BarGraphicsItem::BarWidth * dHScale;

    QRectF newRec(0,ui->grViewQuotes->scene()->sceneRect().y(),
                  iNewWidth  ,
                  ui->grViewQuotes->scene()->sceneRect().height()
                  );
    disconnect(ui->grHorizScroll->horizontalScrollBar(),SIGNAL(valueChanged(int)),this,SLOT(slotHorizontalScrollBarValueChanged(int)));
    grScene->setSceneRect(newRec);
    connect(ui->grHorizScroll->horizontalScrollBar(),SIGNAL(valueChanged(int)),this,SLOT(slotHorizontalScrollBarValueChanged(int)));

    PaintViewPort(true,true,true,false,false);

    if (tStoredRightPointPosition != 0){
        SetSliderToPos(tStoredRightPointPosition, iStoredRightAggregate);
    }
    else{
        SetSliderToPos(tStoredMaxDate, iRightShift);
    }


    ui->grViewQuotes->scene()->invalidate(ui->grViewQuotes->sceneRect());
    ui->grViewVolume->scene()->invalidate(ui->grViewVolume->sceneRect());

    if (dHScale == 1) btnScaleHViewDefault->hide();
    else btnScaleHViewDefault->show();
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::slotVScaleQuotesClicked(bool bPlus)
{
    int iNewViewPortH = (mVScale.at(iSelectedInterval) * (dStoredHighMax - dStoredLowMin)  + iViewPortLowStrip + iViewPortHighStrip );
    double dNewScale = mVScale[iSelectedInterval];
    if (dNewScale == 0) dNewScale = 1.0;

    if (iViewPortHeight < iNewViewPortH || bPlus){
        if (bPlus) dNewScale *= 1.1;
        else dNewScale *= 0.9;

        slotSetNewVScaleQuotes(dNewScale);
    }
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::slotSetNewVScaleQuotes(double dNewScale)
{
    InvalidateCounterDefender def(aiInvalidateCounter);

    mVScale[iSelectedInterval] = dNewScale;

    //int iNewViewPortH = (mVScale.at(iSelectedInterval) * (dStoredHighMax - dStoredLowMin)  + iViewPortLowStrip + iViewPortHighStrip );

    EraseFrames();
    EraseBars();
    EraseMovingAverages();
    RefreshHLines();

    ui->grViewL1->scene()->invalidate(ui->grViewL1->sceneRect());
    ui->grViewR1->scene()->invalidate(ui->grViewR1->sceneRect());

    int iNewViewPortH = (mVScale.at(iSelectedInterval) * (dStoredHighMax - dStoredLowMin)  + iViewPortLowStrip + iViewPortHighStrip );

    if ( iViewPortHeight > iNewViewPortH){
        iNewViewPortH = iViewPortHeight;
    }

    QRectF newRec(0,-iNewViewPortH,
                  ui->grViewQuotes->scene()->sceneRect().width()  ,
                  iNewViewPortH
                  );
    disconnect(ui->grHorizScroll->horizontalScrollBar(),SIGNAL(valueChanged(int)),this,SLOT(slotHorizontalScrollBarValueChanged(int)));
    grScene->setSceneRect(newRec);
    connect(ui->grHorizScroll->horizontalScrollBar(),SIGNAL(valueChanged(int)),this,SLOT(slotHorizontalScrollBarValueChanged(int)));



    def.free();
    PaintViewPort(true,true,false,false,false);

    ui->grViewQuotes->scene()->invalidate(ui->grViewQuotes->sceneRect());

    SetSliderToVertPos(dStoredVValue);
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::slotVScaleVolumeClicked(bool bPlus)
{

    double dNewScale = mVVolumeScale[iSelectedInterval];

    if ( dNewScale == 0) dNewScale = 1.0;
    if (bPlus) dNewScale *= 1.1;
    else dNewScale *= 0.9;

    slotSetNewVScaleVolume(dNewScale);
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::slotSetNewVScaleVolume(double dNewScale){

    mVVolumeScale[iSelectedInterval] = dNewScale;

    EraseLinesLower(mShowedVolumes,        0, ui->grViewVolume->scene());

    int iNewVolumeSceneH = (mVVolumeScale.at(iSelectedInterval) * (dStoredVolumeHighMax - dStoredVolumeLowMin) + iVolumeViewPortHighStrip);
    if (ui->grViewVolume->maximumHeight() > iNewVolumeSceneH){
        iNewVolumeSceneH = ui->grViewVolume->maximumHeight();
    }

    QRectF newRec(0,-iNewVolumeSceneH,
                  ui->grViewVolume->scene()->sceneRect().width(),
                  iNewVolumeSceneH
                  );

    ui->grViewVolume->scene()->setSceneRect(newRec);

    PaintViewPort(false,false,true,false,false);

    ui->grViewVolume->scene()->invalidate(ui->grViewVolume->sceneRect());
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::Erase()
{
    EraseFrames();
    EraseBars();
    EraseMovingAverages();
    EraseVolumes();
    EraseTimeScale();

    stFastShowAverages.clear();
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::EraseTimeScale(){
    mTimesScale.clear();
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::EraseBars()
{
    EraseLinesLower(mShowedGraphicsBars,  0, ui->grViewQuotes->scene());
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::EraseMovingAverages()
{
//    EraseLinesLower(mMovingBlue,    0, ui->grViewQuotes->scene());
//    EraseLinesLower(mMovingRed,     0, ui->grViewQuotes->scene());
//    EraseLinesLower(mMovingGreen,   0, ui->grViewQuotes->scene());
    mMovingBlue.clear();
    mMovingRed.clear();
    mMovingGreen.clear();

    if (pathBlue) {grScene->removeItem(pathBlue); pathBlue = nullptr;}
    if (pathRed)  {grScene->removeItem(pathRed); pathRed = nullptr;}
    if (pathGreen){grScene->removeItem(pathGreen); pathGreen = nullptr;}
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
    EraseInvariantFrames(true,false);

    //////////////////////////
    EraseLinesLower(mVFramesViewQuotes      , 0, ui->grViewQuotes->scene());
    EraseLinesLower(mVFramesScaleUpper      , 0, ui->grViewScaleUpper->scene());
    EraseLinesLower(mVFramesVolume          , 0, ui->grViewVolume->scene());
    EraseLinesLower(mVFramesHorisSmallScale , 0, ui->grViewScaleUpper->scene());
    EraseLinesLower(mVFramesHorisSmallScaleExtremities , 0, ui->grViewScaleUpper->scene());
    //////////////////////////
    EraseLinesLower(mLeftFrames             , 0, ui->grViewL1->scene());
    EraseLinesLower(mRightFrames            , 0, ui->grViewR1->scene());

}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::EraseInvariantFrames      (bool bHorizontal,bool bRepain){

    if (bHorizontal){
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
        if(bRepain){
            PaintHorizontalFrames();
            PaintHorizontalScales();
        }
    }
    else{
        EraseLinesLower(mLeftFrames             , 0, ui->grViewL1->scene());
        EraseLinesLower(mRightFrames            , 0, ui->grViewR1->scene());

        if(bRepain){
            PaintVerticalSideScales();
        }
    }
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::SetMinMaxDateToControls()
{
    {
        std::tm* tmSt=threadfree_gmtime(&tStoredMinDate);
        const QDate dtS(tmSt->tm_year+1900,tmSt->tm_mon+1,tmSt->tm_mday);
        const QTime tmS(tmSt->tm_hour,tmSt->tm_min,tmSt->tm_sec);
        QDateTime dt (dtS,tmS);
        ui->dtBeginDate->setDateTime(dt);
    }
    {
        std::tm* tmSt=threadfree_gmtime(&tStoredMaxDate);
        const QDate dtS(tmSt->tm_year+1900,tmSt->tm_mon+1,tmSt->tm_mday);
        const QTime tmS(tmSt->tm_hour,tmSt->tm_min,tmSt->tm_sec);
        QDateTime dt (dtS,tmS);
        ui->dtEndDate->setDateTime(dt);
    }
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::dateTimeBeginChanged(const QDateTime&)
{

    const QDate tmD(ui->dtBeginDate->date());

    std::tm tmSt;
    {
        tmSt.tm_year   = tmD.year() - 1900;
        tmSt.tm_mon    = tmD.month() - 1;
        tmSt.tm_mday   = tmD.day();
        tmSt.tm_hour   = 0;
        tmSt.tm_min    = 0;
        tmSt.tm_sec    = 0;
        tmSt.tm_isdst  = 0;
    }
    std::time_t tS (mktime_gm(&tmSt));

//    ThreadFreeCout pcout;
//    pcout <<"begtime: "<<threadfree_gmtime_to_str(&tS)<<"\n";

    if (tS < tStoredMinDate){
        emit NeedLoadGraph(iTickerID, tS, tStoredMaxDate);
        SetMinMaxDateToControls(); // keep old value
    }
    else if (tS > tStoredMinDate){
        int n=QMessageBox::warning(0,tr("Warning"),
                                   tr("Do you want to unload extra data?"),
                                   QMessageBox::Yes|QMessageBox::Cancel
                                   );
        if (n==QMessageBox::Yes){
            tStoredMinDate = tS;
            holder->shrink_extras_left(tS);
            Erase();
            emit NeedLoadGraph(iTickerID, tS, tStoredMaxDate);
        }
    }
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
    ResizeMemometer();

}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::RepositionPlusMinusButtons()
{
    QPoint pQ   = ui->grViewQuotes->pos();
    QPoint pV   = ui->grViewVolume->pos();
    QPoint pU   = ui->grViewScaleUpper->pos();
    QPoint pL   = ui->grViewScaleLower->pos();
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

    qreal rX = this->width() - ui->grVertScroll->width() - 3;
    if (!ui->grViewR1->isHidden()){
        rX -= ui->grViewR1->width();
    }

    //QRect rectQ = ui->grViewQuotes->rect();
    //QRect rectL = ui->grViewL1->rect();
    //rX = pQ.x() + rectQ.width();
    //pQ.x() + rectQ.width() + rectL.width() + rAdd - 15

    btnScaleVViewPlus->move     (rX - 15, pQ.y() + 10 );
    btnScaleVViewMinus->move    (rX - 15, pQ.y() + 30 );
    btnScaleVViewDefault->move    (rX - btnScaleVViewDefault->width()  + 1, pQ.y() + 50 );

    btnScaleHViewPlus->move     (rX - 30, pT.y() - 20 );
    btnScaleHViewMinus->move    (rX - 15, pT.y() - 20 );
    btnScaleHViewDefault->move    (rX - btnScaleHViewDefault->width()  + 1, pT.y() - 40 );

    btnScaleVVolumePlus->move   (rX - 15, pV.y() + 10 );
    btnScaleVVolumeMinus->move  (rX - 15, pV.y() + 30 );
    //btnScaleVVolumeDefault->move  (rX - btnScaleVVolumeDefault->width() + 1, pV.y() + 50 );
    btnScaleVVolumeDefault->move  (rX - btnScaleVVolumeDefault->width() - 15 - 5 + 1, pV.y() + 10 );
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
    if (tStoredRightPointPosition !=0 && iSelectedInterval <  iNewInterval){
      tStoredRightPointPosition = Bar::DateAccommodate(tStoredRightPointPosition,iNewInterval,true);
    }

    iSelectedInterval =  iNewInterval;

    dStoredLowMin           = 0;
    dStoredHighMax          = 0;
    dStoredVolumeLowMin     = 0;
    dStoredVolumeHighMax    = 0;
    iStoredMaxSize          = 0;
    /////
    Erase();

    slotInvalidateGraph(tStoredMinDate, tStoredMaxDate,true);

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
 bool GraphViewForm::PaintViewPort   (bool bFrames ,bool bBars ,bool bVolumes, bool bStoreRightPos, bool bInvalidate)
 {
     if (aiInvalidateCounter == 0){
         auto iRange = getViewPortRangeToHolder();
         PaintViewPort   (iRange.first/*iBeg*/,iRange.second/* iEnd*/, bFrames, bBars, bVolumes,bStoreRightPos,bInvalidate);
         return true;
     }
     else{
         RepainTask task;
         task.Type = RepainTask::eRepaintType::PaintViewport;
         if (bFrames){
             task.Type |=  RepainTask::eRepaintType::FastFrames;
         }
         if (bBars){
             task.Type |=  RepainTask::eRepaintType::FastBars;
             task.Type |=  RepainTask::eRepaintType::FastAverages;
         }
         if (bVolumes){
             task.Type |= RepainTask::eRepaintType::FastVolumes;
         }
         task.bInvalidate = bInvalidate;
         task.bStoreRightPos = bStoreRightPos;
         queueRepaint.Push(task);
         return false;
     }
 }
 //---------------------------------------------------------------------------------------------------------------
 void GraphViewForm::PaintViewPort   (int iStart, int iEnd,bool bFrames,bool bBars,bool bVolumes, bool bStoreRightPos, bool bInvalidate)
 {
     RepainTask task(0,iStart,iEnd,false);
     task.bInvalidate = bInvalidate;
     task.bStoreRightPos = bStoreRightPos;

     {
         if (bFrames){
             task.Type |=  RepainTask::eRepaintType::FastFrames;
         }
         if (bBars){
             task.Type |=  RepainTask::eRepaintType::FastBars;
             task.Type |=  RepainTask::eRepaintType::FastAverages;
         }
         if (bVolumes){
             task.Type |= RepainTask::eRepaintType::FastVolumes;
         }

         if (bBars || bVolumes || bFrames){

             task.iLetShift = 100;
             queueRepaint.Push(task);
             slotProcessRepaintQueue();
         }
     }
 }
 //---------------------------------------------------------------------------------------------------------------
 bool GraphViewForm::FastPaintFrames(RepainTask & data)
 {
     bool bSuccess{true};

     if (!data.bReplacementMode){

         if(!data.holder){
             if(!FastLoadHolder(data)){
                 return false;
             }
         }

         if (bSuccess) {bSuccess = PaintHorizontalScales();}
         if (bSuccess) {bSuccess = PaintHorizontalFrames ();}
         if (bSuccess) {bSuccess = PaintVerticalSideScales();}
         if (bSuccess) {bSuccess = PaintVerticalFrames(data.holder,
                                                       data.iStart,
                                                       data.iEnd,
                                                       data.iLetShift,
                                                       data.bReplacementMode,
                                                       data.bInvalidate);}
     }
     else{
         //if (bSuccess) {bSuccess = PaintHorizontalScales();}
         //if (bSuccess) {bSuccess = PaintHorizontalFrames ();}
         //if (bSuccess) {bSuccess = PaintVerticalSideScales();}
         if (bSuccess) {bSuccess = PaintVerticalFrames(data.holder,
                                                       data.iStart,
                                                       data.iEnd,
                                                       data.iLetShift,
                                                       data.bReplacementMode,
                                                       data.bInvalidate);}
     }


     return bSuccess;
 }
 //---------------------------------------------------------------------------------------------------------------
 bool GraphViewForm::FastPaintBars(RepainTask & data)
 {
     if(!data.holder){
         if(!FastLoadHolder(data)){
             return false;
         }
     }

     bool bPaintBars = (data.Type & RepainTask::eRepaintType::FastBars) > 0 ? true: false;
     bool bPaintVolumes = (data.Type & RepainTask::eRepaintType::FastVolumes) > 0 ? true: false;


     bool bSuccess {false};
     if (iSelectedInterval == Bar::eInterval::pTick){
         bSuccess = PaintBars<BarTick>(data.holder, data.iStart, data.iEnd , bPaintBars, bPaintVolumes, data.bStoreRightPos,data.bReplacementMode);
     }
     else{
         bSuccess = PaintBars<Bar>(data.holder, data.iStart, data.iEnd , bPaintBars, bPaintVolumes, data.bStoreRightPos,data.bReplacementMode);
     }
     /////
     return bSuccess;
 }
 //---------------------------------------------------------------------------------------------------------------
 bool GraphViewForm::FastPaintAverages(RepainTask & data)
 {
     if(!data.holder){
         if(!FastLoadHolder(data)){
             return false;
         }
     }
     return PaintMovingAverages (data.holder, data.iStart, data.iEnd,data.bReplacementMode);
 }
 //---------------------------------------------------------------------------------------------------------------
 bool GraphViewForm::FastLoadHolder(RepainTask &data)
 {
     return holder->CloneHolder(data.holder,iSelectedInterval,
                                data.iStart > 0     ? data.iStart       : 0,
                                data.iEnd > 0       ? data.iEnd         : 0,
                                data.iLetShift > 0  ? data.iLetShift    : 0
                                );
 }
 //---------------------------------------------------------------------------------------------------------------
 template<typename T>
 bool GraphViewForm::PaintBars (std::shared_ptr<GraphHolder> local_holder,int iStartI, int iEndI ,
                                bool bPaintBars, bool bPaintVolumes,
                                bool bStoreRightPos, bool bReplacementMode)
{
     InvalidateCounterDefender def(aiInvalidateCounter);
//     {
//         ThreadFreeCout pcout;
//         pcout << "PaintBars <"<<iStartI<<":"<<iEndI<<">\n";
//     }
     /////////////////////////////////////////////////////////////////////////////////////////
     if (!local_holder) return true;
     bool bSuccess;
     auto ItDefender = local_holder->beginIteratorByDate<T>(iSelectedInterval,0,bSuccess);
     if (!bSuccess) return false;
     //
     int iMaxSize = (int)local_holder->getViewGraphSize(iSelectedInterval);
     if (iMaxSize == 0 ) return true;

     const Graph<T>& graph = local_holder->getGraph<T>(iSelectedInterval);
     const int iShift {(int)graph.GetShiftIndex()};
     /////////////////////////////////////////////////////////////////////////////////////////


//     if (bReplacementMode)
//     {
//         ThreadFreeCout pcout;
//         pcout <<"src {"<<iStartI<<":"<<iEndI<<"}\n";
//     }

     //size_t iViewWidth = ui->grViewQuotes->width()/(dHScale *BarGraphicsItem::BarWidth);


     int iBeg    = iStartI - iShift >= 0    ? iStartI - iShift  : 0 ;
     iBeg        = iBeg < iMaxSize          ? iBeg              : iMaxSize - 1;

     int iEnd    = iEndI >= 0               ? iEndI             : 0 ;
     iEnd        = iEnd - iShift < iMaxSize ? iEnd - iShift     : iMaxSize - 1;

     /////////////////////////////////////////////////////////////////

     if (bSuccess && iMaxSize > 0){

         if (!bReplacementMode)
         {
             if (bPaintBars){
                 EraseLinesUpper(mShowedGraphicsBars,  iBeg + iShift, ui->grViewQuotes->scene());
                 EraseLinesLower(mShowedGraphicsBars,  iEnd + iShift, ui->grViewQuotes->scene());
             }
             if (bPaintVolumes){
                 EraseLinesUpper(mShowedVolumes,        iBeg + iShift, ui->grViewVolume->scene());
                 EraseLinesLower(mShowedVolumes,        iEnd + iShift, ui->grViewVolume->scene());
             }
             EraseLinesUpper(mTimesScale,        iBeg + iShift, nullptr);
             EraseLinesLower(mTimesScale,        iEnd + iShift, nullptr);
         }
         else{
             if (bPaintBars){
                 EraseLinesMid(mShowedGraphicsBars,  iBeg + iShift,iEnd + iShift, ui->grViewQuotes->scene());
             }
             if (bPaintVolumes){
                 EraseLinesMid(mShowedVolumes,  iBeg + iShift,iEnd + iShift, ui->grViewVolume->scene());
             }
             EraseLinesMid(mTimesScale,        iBeg + iShift,iEnd + iShift, nullptr);
         }
         ///////////////////
         std::stringstream ss;
         std::time_t tTmp;
         QPen bluePen(Qt::blue,1,Qt::SolidLine);

         for (int i = iBeg ; i <= iEnd; ++i){
             qreal xCur = (i + iShift + iLeftShift)     * BarGraphicsItem::BarWidth * dHScale;
             const T &b = graph[i];
             if(mTimesScale.find(i + iShift) == mTimesScale.end()){
                 mTimesScale[i + iShift] = {b.Period(),false};
             }
             //
             if (bPaintBars){
                 auto ItFound = mShowedGraphicsBars.find(i + iShift);

                 if (ItFound == mShowedGraphicsBars.end())
                 {

                     ///////////////////////////////////////////////////////////////
                     ////// resize scene if needed

                     //if (bReplacementMode)
                     {
                         QRectF scRect = ui->grViewQuotes->scene()->sceneRect();

                         qreal xNewRight = (i + iShift + iLeftShift + iRightShift) * BarGraphicsItem::BarWidth * dHScale;
                         qreal xNewHalfRight = (i + iShift + iLeftShift + iRightShift/3) * BarGraphicsItem::BarWidth * dHScale;

                         if(b.High() > dStoredHighMax)      dStoredHighMax = b.High();
                         if(b.Low() < dStoredLowMin)        dStoredLowMin = b.Low();
                         if (iStoredMaxSize < i + iShift + 1)   iStoredMaxSize = i + iShift + 1;

                         int iNewViewPortH = (mVScale.at(iSelectedInterval) * (dStoredHighMax - dStoredLowMin)  + iViewPortLowStrip + iViewPortHighStrip );
                         if ( iViewPortHeight > iNewViewPortH){
                             iNewViewPortH = iViewPortHeight;
                         }

                         ////
                         RepainTask task(0,0,0,false);
                         task.Type |=  RepainTask::eRepaintType::FastFrames;
                         task.bReplacementMode = true;

                         task.iStart =  iBeg + iShift;
                         task.iEnd   =  iEnd + iShift;

                         if(scRect.height() < iNewViewPortH ){
                             QRectF newRec(0,-iNewViewPortH,xNewRight,iNewViewPortH);
                             grScene->setSceneRect(newRec);

                             ReDoHLines();

                             EraseFrames();
                             EraseInvariantFrames(true,true);
                             EraseInvariantFrames(false,true);
                             task.iLetShift = 100;
                             queueRepaint.Push(task);
                         }
                         else if(scRect.x() + scRect.width() < xNewHalfRight){
                             QRectF newRec(0,scRect.y(),xNewRight,scRect.height());

                             disconnect(ui->grHorizScroll->horizontalScrollBar(),SIGNAL(valueChanged(int)),this,SLOT(slotHorizontalScrollBarValueChanged(int)));
                             grScene->setSceneRect(newRec);
                             connect(ui->grHorizScroll->horizontalScrollBar(),SIGNAL(valueChanged(int)),this,SLOT(slotHorizontalScrollBarValueChanged(int)));

                             EraseInvariantFrames(true,true);
                             EraseInvariantFrames(false,true);

                             //task.iLetShift = 100;
                             //queueRepaint.Push(task);
                         }

                     }
                     ///////////////////////////////////////////////////////////////
                     /// paint bars

                     BarGraphicsItem *item = new BarGraphicsItem(b,i + iShift,3,mVScale[iSelectedInterval]);
                     mShowedGraphicsBars[i + iShift].push_back(item);
                     item->setZValue(5); // always on top
                     item->SetOHLC(&GraphViewForm::IsOHLC,this);
                     ui->grViewQuotes->scene()->addItem(item);
                     item->setPos(xCur , -realYtoSceneY(b.Close()));
                }
             }

             //====================================================
             if (bPaintVolumes){
                 auto ItFound = mShowedVolumes.find(i + iShift);

                 if (ItFound == mShowedVolumes.end())
                 {
                     ss.str("");
                     ss.clear();
                     tTmp = b.Period();
                     ss << threadfree_gmtime_to_str(&tTmp)<<"\r\n";
                     ss << b.Volume();

                     DrawLineToScene(i + iShift, xCur,-3,xCur,-realYtoSceneYVolume(b.Volume()),
                                     mShowedVolumes, ui->grViewVolume->scene(),bluePen,ss.str(),true,5);
                 }
             }
         }
         ////////////////////////////////////////////////////////////////////
         if(bReplacementMode && iBeg <= iEnd){

             auto It (mTimesScale.lower_bound(iBeg + iShift));
             auto ItEnd (mTimesScale.upper_bound(iEnd + iShift));

             bool bWas{false};
             while(It != ItEnd){
                 if (!It->second.second){
                     bWas = true;
                     break;
                 }
             }
             if (bWas){
                 RepainTask task(0,0,0,false);
                 task.Type |=  RepainTask::eRepaintType::FastFrames;
                 task.bReplacementMode = true;

                 task.iStart =  iBeg + iShift;
                 task.iEnd   =  iEnd + iShift;
                 task.iLetShift = 40;
                 queueRepaint.Push(task);
                 //FastPaintFrames(task);
             }
         }
         if (!bReplacementMode && bStoreRightPos && iEndI > 0){
             if(mTimesScale.find(iEndI) != mTimesScale.end()){
                 tStoredRightPointPosition = Bar::DateAccommodate(mTimesScale[iEndI].first,iSelectedInterval,true);
                 iStoredRightAggregate  = 0;
             }
             else{
                 if(mTimesScale.find(iEnd) != mTimesScale.end()){
                     tStoredRightPointPosition = Bar::DateAccommodate(mTimesScale[iEnd].first,iSelectedInterval,true);
                     iStoredRightAggregate  = iEndI > iEnd ? iEndI - iEnd : 0;
                 }
             }
         }
     }
     return true;
}
 //---------------------------------------------------------------------------------------------------------------
void GraphViewForm::ReDoHLines()
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
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::RefreshHLines()
{
     for (auto &d: vHLines){
         d.second = false;
     }
}
//---------------------------------------------------------------------------------------------------------------
bool GraphViewForm::PaintHorizontalFrames       ()
{
     // TODO: paint only viewport
     if (vHLines.size() > 1){
         double dDelta = realYtoSceneY(vHLines[1].first) - realYtoSceneY(vHLines[0].first);
         if (dDelta < 60){
             ReDoHLines();
         }
     }
     //ReDoHLines();
     //RefreshHLines();
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
     return true;
}
//---------------------------------------------------------------------------------------------------------------
bool GraphViewForm::PaintVerticalSideScales     ()
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

     return true;
}
//---------------------------------------------------------------------------------------------------------------
bool GraphViewForm::PaintVerticalFrames ( std::shared_ptr<GraphHolder> local_holder, int iStart, int iEnd,
                                           int iLeftStock,
                                           bool bReplacementMode,
                                           bool bInvalidate)
{
     if (!bReplacementMode){
         if (!local_holder) return true;
         bool bSuccess;

         if (iSelectedInterval == Bar::eInterval::pTick){
             auto ItDefender = local_holder->beginIteratorByDate<BarTick>(iSelectedInterval,0,bSuccess);
             if (!bSuccess) return false;

             return PainVerticalFramesT<BarTick>   (local_holder, iStart, iEnd,iLeftStock,bReplacementMode,bInvalidate);
         }
         else{
             auto ItDefender = local_holder->beginIteratorByDate<Bar>(iSelectedInterval,0,bSuccess);
             if (!bSuccess) return false;

             return PainVerticalFramesT<Bar>       (local_holder, iStart, iEnd,iLeftStock,bReplacementMode,bInvalidate);
         }
     }
     else{

        if (iSelectedInterval == Bar::eInterval::pTick){
            return PainVerticalFramesT<BarTick>   (local_holder, iStart, iEnd,iLeftStock,bReplacementMode,bInvalidate);
        }
         else{
             return PainVerticalFramesT<Bar>       (local_holder, iStart, iEnd,iLeftStock,bReplacementMode,bInvalidate);
         }
     }
}
//---------------------------------------------------------------------------------------------------------------
template<typename T>
bool GraphViewForm::PainVerticalFramesT(std::shared_ptr<GraphHolder> local_holder,int iBegSrc, int iEndSrc,
                                         int iLeftStock,
                                         bool bReplacementMode,
                                         bool bInvalidate)
{

     const Graph<T>  grTmp(iTickerID,iSelectedInterval);
     const Graph<T>& graph = !bReplacementMode  ? local_holder->getGraph<T>(iSelectedInterval)              : grTmp;
     const int iShift = !bReplacementMode       ? (int)graph.GetShiftIndex()                                : 0;
     const int iMaxSize = !bReplacementMode     ? (int)local_holder->getGraph<T>(iSelectedInterval).size()  : iStoredMaxSize;
     if (iMaxSize == 0 ) return true;
     if (bReplacementMode && mTimesScale.empty()) return true;
     /////////////////////////////////////////////////////////////////////////////////////////
     /////////////////////////////////////////////////////////////////////////////////////////
//     {
//         ThreadFreeCout pcout;
//         pcout <<"paint frames {iBegSrc:iEndSrc} {"<<iBegSrc<<":"<<iEndSrc<<"}\n";
//     }

     int iSBeg  = {iBegSrc < iShift ? 0 : iBegSrc - iShift};
     int iSLeftBeg = iSBeg > iLeftStock ? iSBeg - iLeftStock : 0;
     int iSEnd  = {iEndSrc < iShift ? 0 : iEndSrc - iShift};

//     {
//         ThreadFreeCout pcout;
//         pcout <<"paint frames {iSLeftBeg:iSBeg:iSEnd} {"<<iSLeftBeg<<":"<<iSBeg<<":"<<iSEnd<<"}\n";
//     }

     if (!bReplacementMode){
         EraseLinesUpper(mVFramesViewQuotes,        iBegSrc, ui->grViewQuotes->scene());
         EraseLinesLower(mVFramesViewQuotes,        iEndSrc, ui->grViewQuotes->scene());

         EraseLinesUpper(mVFramesScaleUpper,        iBegSrc, ui->grViewScaleUpper->scene());
         EraseLinesLower(mVFramesScaleUpper,        iEndSrc, ui->grViewScaleUpper->scene());

         EraseLinesUpper(mVFramesVolume,            iBegSrc, ui->grViewVolume->scene());
         EraseLinesLower(mVFramesVolume,            iEndSrc, ui->grViewVolume->scene());

         EraseLinesUpper(mVFramesHorisSmallScale,   iBegSrc, ui->grViewScaleUpper->scene());
         EraseLinesLower(mVFramesHorisSmallScale,   iEndSrc, ui->grViewScaleUpper->scene());

         EraseLinesUpper(mVFramesHorisSmallScaleExtremities,   iEndSrc, ui->grViewScaleUpper->scene());
     }
     else{
         EraseLinesUpper(mVFramesHorisSmallScaleExtremities,  iEndSrc, ui->grViewScaleUpper->scene());

         iSBeg = iSLeftBeg;
     }


     QPen blackSolidPen(Qt::black,0.5,Qt::SolidLine);
     QPen blackDashPen(Qt::gray,1,Qt::DashLine);
     QPen blackDotPen(Qt::gray,1,Qt::DotLine);
    // QPen simpleDashPen(Qt::gray,1,Qt::DashLine);

    // QPen redPen(Qt::red,1,Qt::SolidLine);

     int iLineH = ui->grViewScaleUpper->scene()->sceneRect().height()/2;
     static const int iShiftH {4};
     //////////////////////////////////////////////////////////////
     qreal xCur{0};
     //qreal xPre{0};
     int iFCount{0};
     int iDayCounter{0};
     int iSecCounter{0};
     std::time_t tTmp{0};

     std::tm tmPre = *threadfree_gmtime(&tTmp);
     std::tm tmCur = tmPre;

     bool bFifstLine{true};

     tmCur = *threadfree_gmtime(&tTmp);

     QRectF rectQuotes =  ui->grViewQuotes->scene()->sceneRect();
     QRectF rectVolume =  ui->grViewVolume->scene()->sceneRect();
     /////
     bool bLineExists{false};
     int iInvalidate_X_BEG_1{iSBeg + iShift};
     int iInvalidate_X_BEG_2{0};
     //
     /////
     std::stringstream ss;

     //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
     // preliminary (left) scale
     for (int j = iBegSrc ; j <= 0 ; ++j) {
         auto ItSL = mVFramesHorisSmallScale.find(j);
         if (ItSL == mVFramesHorisSmallScale.end() || ItSL->second.size() == 0){
             xCur = (j + iLeftShift)     * BarGraphicsItem::BarWidth * dHScale;
             DrawLineToScene(j, xCur,0,xCur,5, mVFramesHorisSmallScale, ui->grViewScaleUpper->scene(),blackSolidPen,tTmp,true);
         }
     }

     //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
     /// creating shift constants
     std::call_once(GraphViewForm_init_consts_call_once_flag,&GraphViewForm::init_const,this);
     //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

     // main ciclus
     for (int indx = iSLeftBeg; indx<= iSEnd; ++indx){
         bLineExists = false;
         //
         ss.str("");
         ss.clear();
         auto ItSL = mVFramesHorisSmallScale.find(indx + iShift);
         if (ItSL != mVFramesHorisSmallScale.end() && ItSL->second.size() >0){

             bLineExists = true;
             if (iInvalidate_X_BEG_2 == 0){
                 iInvalidate_X_BEG_2 = indx + iShift;
             }
         }
         //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
         //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
         xCur = (indx + iShift + iLeftShift)     * BarGraphicsItem::BarWidth * dHScale;
         if ( indx >= 0 && indx  < iMaxSize
              && (!bReplacementMode || mTimesScale.find(indx + iShift) != mTimesScale.end())
              ){
             //--------------------------
             if(!bReplacementMode){
                 const T & tM = graph[indx];// holder->getByIndex<T>(iSelectedInterval,indx + iShift);
                 tTmp = tM.Period();
             }
             else{
                 tTmp = mTimesScale[indx + iShift].first;
             }
             tmCur = *threadfree_gmtime(&tTmp);
             //--------------------------
             if (!bFifstLine ){
                 //------------------------------------------------------------------------
                 // set mark that the time has been processed
                 if (iSBeg <= indx && !bLineExists ){
                     mTimesScale[indx + iShift].second = true;
                 }
                 //------------------------------------------------------------------------
                 // draw small lines down under :)
                 if (iSBeg <= indx && !bLineExists ){//&& false
                     if (iFCount <= 0){
                          DrawLineToScene(indx + iShift, xCur,0,xCur,1, mVFramesHorisSmallScale, ui->grViewScaleUpper->scene(),blackSolidPen,tTmp,true);
                     }
                     else {
                          DrawLineToScene(indx + iShift, xCur,0,xCur,5, mVFramesHorisSmallScale, ui->grViewScaleUpper->scene(),blackSolidPen,tTmp,true);
                     }
                 }
                 //------------------------------------------------------------------------
                 //  draw year line
                 if(tmPre.tm_year != tmCur.tm_year){
                     iDayCounter = 0;

                     if (iSBeg <= indx && !bLineExists){
                         ss <<"Year\n";
                         ss <<threadfree_gmtime_date_to_str(&tTmp);

                         DrawLineToScene             (indx + iShift, xCur, 0,xCur,iLineH * 2, mVFramesScaleUpper, ui->grViewScaleUpper->scene(),blackSolidPen,tTmp,true); // down line

                         DrawLineToScene             (indx + iShift, xCur, -iShiftH ,xCur, - rectVolume.height() + iShiftH * 2, mVFramesVolume, ui->grViewVolume ->scene(),blackDashPen,tTmp,true); // volume line
                         DrawIntermittentLineToScene (indx + iShift, xCur, -iShiftH ,xCur, - rectQuotes.height() + iShiftH * 2, mVFramesViewQuotes, ui->grViewQuotes->scene(),blackDashPen,ss.str(),true); // middle line

                         DrawIntToScene(indx + iShift, xCur, iLineH + iLineH/2 ,tmCur.tm_year + 1900,Qt::AlignmentFlag::AlignLeft, Qt::AlignmentFlag::AlignCenter,
                                        mVFramesScaleUpper, ui->grViewScaleUpper->scene(), fontNumb);
                     }
                 }
                 //  draw month line
                 else if(tmPre.tm_mon != tmCur.tm_mon
                         && iSelectedInterval != Bar::eInterval::pMonth // for months don't draw
                         && (iSelectedInterval != Bar::eInterval::pWeek || tmCur.tm_mon % 3 ==0) // for weeks  - only draw only for every third
                         ){
                     iDayCounter = 0;

                     if (iSBeg <= indx && !bLineExists){
                         ss <<"Month\n";
                         ss <<threadfree_gmtime_date_to_str(&tTmp);
                         //

                         DrawLineToScene             (indx + iShift, xCur, 0,xCur,iLineH * 2, mVFramesScaleUpper, ui->grViewScaleUpper->scene(),blackSolidPen,tTmp,true); // down line
                         //
                         DrawLineToScene             (indx + iShift, xCur, -iShiftH ,xCur, - rectVolume.height() + iShiftH * 2, mVFramesVolume, ui->grViewVolume ->scene(),blackDashPen,tTmp,true); // volume line
                         DrawIntermittentLineToScene (indx + iShift, xCur, -iShiftH ,xCur, - rectQuotes.height() + iShiftH * 2, mVFramesViewQuotes, ui->grViewQuotes->scene(),blackDashPen,ss.str(),true); // middle line

                         DrawIntToScene(indx + iShift, xCur, iLineH + iLineH/2,tmCur.tm_mon + 1 ,Qt::AlignmentFlag::AlignLeft, Qt::AlignmentFlag::AlignCenter,
                                        mVFramesScaleUpper, ui->grViewScaleUpper->scene(), fontNumb);

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
                     if (iSBeg <= indx && !bLineExists){
                         ss <<"Day\n";
                         ss <<threadfree_gmtime_date_to_str(&tTmp);

                         DrawLineToScene             (indx + iShift, xCur, -iShiftH ,xCur, - rectVolume.height() + iShiftH * 2, mVFramesVolume, ui->grViewVolume ->scene(),blackDashPen,tTmp,true); // volume line
                         DrawIntermittentLineToScene (indx + iShift, xCur, -iShiftH ,xCur, - rectQuotes.height() + iShiftH * 2, mVFramesViewQuotes, ui->grViewQuotes->scene(),blackDashPen,ss.str(),true); // middle line

                         if (iFCount > 0)
                         {
                             DrawIntToScene(indx + iShift, xCur, iLineH/2 ,tmCur.tm_mday,Qt::AlignmentFlag::AlignLeft, Qt::AlignmentFlag::AlignCenter,
                                            mVFramesScaleUpper, ui->grViewScaleUpper->scene(), fontNumb);
                         }
                         else{
                             DrawIntToScene(indx + iShift, xCur, iLineH/2 ,0, Qt::AlignmentFlag::AlignLeft, Qt::AlignmentFlag::AlignCenter,
                                            mVFramesScaleUpper, ui->grViewScaleUpper->scene(), fontNumb);

                         }

                     }
                     if (iFCount > 0){
                         if (tmCur.tm_mday <10)
                             iFCount = - iConstWidthNumb1;
                         else
                             iFCount = - iConstWidthNumb2;
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
                     if (iSBeg <= indx && !bLineExists){
                         DrawLineToScene             (indx + iShift, xCur, -iShiftH ,xCur, - rectVolume.height() + iShiftH * 2, mVFramesVolume, ui->grViewVolume ->scene(),blackDashPen,tTmp,true); // volume line
                         DrawIntermittentLineToScene (indx + iShift, xCur, -iShiftH ,xCur, - rectQuotes.height() + iShiftH * 2, mVFramesViewQuotes, ui->grViewQuotes->scene(),blackDashPen,tTmp,true); // middle line

                         if (iFCount > 1)
                         {
                             DrawTimeToScene(indx + iShift, xCur, -1 ,tmCur,mVFramesScaleUpper, ui->grViewScaleUpper->scene(), fontTime);
                         }
                     }

                     if (iFCount > 1)
                     {
                         iFCount = - iConstWidthTime;
                     }

                 }
                 //  draw ticks
                 else if(iSelectedInterval == Bar::eInterval::pTick && (tmPre.tm_min != tmCur.tm_min)){

                     if (iSBeg <= indx && !bLineExists){
                         //
                         DrawLineToScene             (indx + iShift, xCur, -iShiftH ,xCur, - rectVolume.height() + iShiftH * 2, mVFramesVolume, ui->grViewVolume ->scene(),blackDashPen,tTmp,true); // volume line
                         DrawIntermittentLineToScene (indx + iShift, xCur, -iShiftH ,xCur, - rectQuotes.height() + iShiftH * 2, mVFramesViewQuotes, ui->grViewQuotes->scene(),blackDashPen,tTmp,true); // middle line

                         if (iFCount > 0)
                         {
                             DrawTimeToScene(indx + iShift, xCur, -1 ,tmCur,mVFramesScaleUpper, ui->grViewScaleUpper->scene(), fontTime);
                         }

                     }
                     if (iFCount > 0)
                     {
                         iFCount = - iConstWidthTime;
                     }

                     iSecCounter = 0;
                 }
                 else if(iSelectedInterval == Bar::eInterval::pTick
                         && (tmPre.tm_sec != tmCur.tm_sec /*&& iSecCounter >=8 */)
                         ){
                     //
                     if (iSBeg <= indx + iShift && !bLineExists){
                        DrawIntermittentLineToScene(indx + iShift, xCur, -iShiftH ,xCur, - rectQuotes.height() + iShiftH * 2, mVFramesViewQuotes, ui->grViewQuotes->scene(),blackDotPen,tTmp,true); // middle line
                     }
                     iSecCounter = 0;
                 }
                 else{
                     // free line
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
                 if ( indx + iShift >= iMaxSize){ // only right
                     if (mVFramesHorisSmallScaleExtremities.find(indx + iShift) == mVFramesHorisSmallScaleExtremities.end()){
                        DrawLineToScene(indx + iShift, xCur,0,xCur,5, mVFramesHorisSmallScaleExtremities, ui->grViewScaleUpper->scene(),blackSolidPen);
                        //DrawLineToScene(indx + iShift, xCur,0,xCur,5, mVFramesHorisSmallScaleExtremities, ui->grViewScaleUpper->scene(),redPen);
                     }
                 }
             }
             //bFifstLine = false;
         }
         //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
         //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
         //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

         iFCount++;
         iSecCounter++;
         tmPre = tmCur;
     }

     if (bInvalidate)
     {
         QRectF rec =  ui->grViewQuotes->scene()->sceneRect();

         QRectF newRec((iInvalidate_X_BEG_1 + iLeftShift) * BarGraphicsItem::BarWidth * dHScale,
                       rec.y(),
                       (iInvalidate_X_BEG_2 + iLeftShift) * BarGraphicsItem::BarWidth * dHScale,
                       rec.height());

         ui->grViewQuotes->scene()->invalidate(newRec);
     }
     /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
     return true;
}
//---------------------------------------------------------------------------------------------------------------
bool GraphViewForm::PaintHorizontalScales()
{

     QColor color(204, 122, 0,155);// orange
     QPen Pen(color,1,Qt::DashLine);

     QPen blackSolidPen(Qt::black,0.5,Qt::SolidLine);
     QPen blackGrayPen(Qt::black,0.5,Qt::SolidLine);
//     QPen blackDashPen(Qt::gray,1,Qt::DashLine);
//     QPen blackDotPen(Qt::gray,1,Qt::DotLine);
//     QPen simpleDashPen(Qt::gray,1,Qt::DashLine);

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

     return true;
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::SetSliderToVertPos(double dPos)
{
    double dHalf = (sceneYtoRealY(-ui->grViewQuotes->verticalScrollBar()->value()) -
                    sceneYtoRealY(-ui->grViewQuotes->verticalScrollBar()->value() - ui->grViewQuotes->verticalScrollBar()->pageStep())
                ) / 2 ;

    double dCur = dPos + dHalf;

    ui->grViewQuotes->verticalScrollBar()->setValue(-realYtoSceneY(dCur));
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
             int iSize = (int)holder->getViewGraphSize(iSelectedInterval);
             xCur = (iSize + iLeftShift - 1) * BarGraphicsItem::BarWidth * dHScale - ui->grViewQuotes->horizontalScrollBar()->pageStep();
             xCur += iRightAggregate * BarGraphicsItem::BarWidth * dHScale;
         }

        It.unlock();
        ui->grViewScaleUpper->horizontalScrollBar()->setValue(xCur);
        ui->grViewVolume->horizontalScrollBar()->setValue(xCur);
        //ui->grViewScaleLower->horizontalScrollBar()->setValue(xCur);
        ui->grViewQuotes->horizontalScrollBar()->setValue(xCur);
        ui->grHorizScroll->horizontalScrollBar()->setValue(xCur);
     }
}
//---------------------------------------------------------------------------------------------------------------
std::pair<int,int> GraphViewForm::getViewPortRangeToHolder()
{
     int iBeg = ui->grViewQuotes->horizontalScrollBar()->value();
     int iEnd = iBeg + ui->grViewQuotes->horizontalScrollBar()->pageStep();

     QRectF rS = ui->grViewQuotes->scene()->sceneRect();
     //QRectF viewportS = ui->grViewQuotes->sceneRect();

     if (ui->grViewQuotes->horizontalScrollBar()->maximum() > 0) { // if slider range was expanded and exceeds viewport

         iBeg = ((((double)iBeg)/dHScale)/(double)BarGraphicsItem::BarWidth);
         iEnd = ((((double)iEnd)/dHScale)/(double)BarGraphicsItem::BarWidth);

         iBeg = iBeg - iLeftShift;
         iEnd = iEnd - iLeftShift;
     }
     else{
         iBeg = -iLeftShift;
         iEnd = ((rS.width()/dHScale)/(double)BarGraphicsItem::BarWidth) - iLeftShift;
     }
     return {iBeg,iEnd};
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::slotFastShowEvent(std::shared_ptr<GraphHolder> ptrHolder)
{

    InvalidateCounterDefender def(aiInvalidateCounter);

     const int iShift = (int)ptrHolder->getShiftIndex(iSelectedInterval);
     int iShBeg = iShift;
     int iShEnd = iShBeg + (int)ptrHolder->getViewGraphSize(iSelectedInterval);

     std::pair<int,int> pViewPortRange = getViewPortRangeToHolder();

     if(!(pViewPortRange.first > iShEnd || pViewPortRange.second < iShBeg)){

         if (iShBeg < pViewPortRange.first)  iShBeg = pViewPortRange.first;
         if (iShEnd > pViewPortRange.second) iShEnd = pViewPortRange.second;


         {
             RepainTask task(0,0,0,false);
             task.Type |= RepainTask::eRepaintType::FastBars;
             task.Type |= RepainTask::eRepaintType::FastVolumes;
             //task.Type |=  RepainTask::eRepaintType::FastFrames;
             task.bReplacementMode = true;

             task.iStart =  iShBeg;
             task.iEnd   =  iShEnd;

             task.holder = ptrHolder;

             queueRepaint.Push(task);

             checkFastShowAverages(iShBeg, iShEnd);
         }
     }
     ////////////////////////////////////////////////////////////////
     slotProcessRepaintQueue();
     ////////////////////////////////////////////////////////////////
}
//---------------------------------------------------------------------------------------------------------------
// paint averages only if there are more then 1 bar or more then 500ms passed
void GraphViewForm::checkFastShowAverages(int iStart, int iEnd){

    milliseconds mLast = std::chrono::steady_clock::now() - dtFastShowAverageActivity;

    for (int i = iStart; i <= iEnd; ++i){
        if (stFastShowAverages.find(i) == stFastShowAverages.end()){
            stFastShowAverages.insert(i);
        }
    }
    if (    (stFastShowAverages.size() > 1 && mLast.count() > 500)
         || (stFastShowAverages.size() > 0 && mLast.count() > 10000)){
        dtFastShowAverageActivity = std::chrono::steady_clock::now();

        RepainTask task(0,0,0,false);
        task.Type |= RepainTask::eRepaintType::FastAverages;

        task.bReplacementMode = true;

        task.iStart =  *stFastShowAverages.begin();
        task.iEnd   =  *stFastShowAverages.rbegin() + 8;

        task.bRecalculateAverages = true;

        queueRepaint.Push(task);
        if(stFastShowAverages.size() > 1){
            auto It (stFastShowAverages.end());
            It--;
            stFastShowAverages.erase(stFastShowAverages.begin(),It);
        }
    }
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::init_const(){

     QGraphicsTextItem itemNumb1("0");
     itemNumb1.setFont(fontNumb);
     iConstWidthNumb1 = ((itemNumb1.boundingRect().width() + 2) / BarGraphicsItem::BarWidth) / dHScale - 1;

     QGraphicsTextItem itemNumb2("00");
     itemNumb2.setFont(fontNumb);
     iConstWidthNumb2 = ((itemNumb2.boundingRect().width() + 2) / BarGraphicsItem::BarWidth) / dHScale - 1;

     QGraphicsTextItem itemTime("00:00");
     itemTime.setFont(fontTime);
     iConstWidthTime = ((itemTime.boundingRect().width() + 2) / BarGraphicsItem::BarWidth) / dHScale - 1;

}
//---------------------------------------------------------------------------------------------------------------
bool GraphViewForm::eventFilter(QObject *watched, QEvent *event)
{
     if (    watched == ui->grViewQuotes
             || watched == ui->grViewScaleUpper
             || watched == ui->grViewVolume
             || watched == ui->grViewScaleLower

             || watched == ui->grViewQuotes->verticalScrollBar()
             || watched == ui->grViewScaleUpper->verticalScrollBar()
             || watched == ui->grViewVolume->verticalScrollBar()
             || watched == ui->grViewScaleLower->verticalScrollBar()

             || watched == ui->grViewQuotes->horizontalScrollBar()
             || watched == ui->grViewScaleUpper->horizontalScrollBar()
             || watched == ui->grViewVolume->horizontalScrollBar()
             || watched == ui->grViewScaleLower->horizontalScrollBar()
             ){
         if(event->type() == QEvent::Wheel){
             QWheelEvent* pe = (QWheelEvent*)event;
             if (pe ){
                 QPoint numPixels = pe->pixelDelta();
                 QPoint numDegrees = pe->angleDelta();

                 auto bt = pe->modifiers();
                 //
                 if (bt.testFlag(Qt::ControlModifier)){
                     if ((!numPixels.isNull()  && numPixels.x() != 0) || (!numDegrees.isNull())){

                         bool bPlus{false};
                         if ((!numPixels.isNull() && numPixels.x() > 0 ) || (!numDegrees.isNull() && numDegrees.y() > 0) ){
                             bPlus = true;
                         }

                         if (watched != ui->grViewVolume && watched != ui->grViewVolume->verticalScrollBar()){
                            slotVScaleQuotesClicked(bPlus);
                         }
                         else{
                             slotVScaleVolumeClicked(bPlus);
                         }
                         event->accept();
                         return true;
                     }
                 }
                 else if (bt.testFlag(Qt::AltModifier)){
                     if ((!numPixels.isNull()  && numPixels.x() != 0) || (!numDegrees.isNull())){

                         bool bPlus{false};
                         if ((!numPixels.isNull() && numPixels.y() > 0 ) || (!numDegrees.isNull() && numDegrees.x() > 0) ){
                             bPlus = true;
                         }
                         slotHScaleQuotesClicked(bPlus);
                     }
                     event->accept();
                     return true;
                 }
                 else if ((bt.testFlag(Qt::ShiftModifier) && bInvertMouseWheel) || (!bt.testFlag(Qt::ShiftModifier) && !bInvertMouseWheel)){
                 }
                 else if ((bt.testFlag(Qt::ShiftModifier) && !bInvertMouseWheel) || bInvertMouseWheel){
                     bool bRet;
                     if (watched != ui->grViewVolume && watched != ui->grViewVolume->verticalScrollBar()){
                         bRet = ui->grViewQuotes->horizontalScrollBar()->event(event);
                         slotHorizontalScrollBarValueChanged(ui->grViewQuotes->horizontalScrollBar()->value());
                     }
                     else{
                         bRet = ui->grViewVolume->horizontalScrollBar()->event(event);
                         slotHorizontalScrollBarValueChanged(ui->grViewVolume->horizontalScrollBar()->value());
                     }


                     event->accept();
                     return bRet;
                 }
             }
         }
     }
     if(watched == ui->grViewL1){
         if(event->type() == QEvent::Hide){
             RepositionPlusMinusButtons();
         }
     }
     if (watched == btnScaleHViewDefault || watched == btnScaleVViewDefault || watched ==btnScaleVVolumeDefault){
         if(event->type() == QEvent::Resize){
             RepositionPlusMinusButtons();
         }
     }
     return QObject::eventFilter(watched, event);
}
//---------------------------------------------------------------------------------------------------------------
bool GraphViewForm::event(QEvent *event)
{
    if(event->type() == QEvent::Close)
    {
        slotSaveUnsavedConfigs();
    }
    return QWidget::event(event);
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::slotSaveUnsavedConfigs()
{
    auto It (std::find_if(vTickersLst.begin(),vTickersLst.end(),[&](const Ticker &t){
                return t.TickerID() == iTickerID;
                }));
    if(It != vTickersLst.end()){
        //tTicker = (*It);
        It->SetStoredSelectedInterval(iSelectedInterval);
        It->SetStoredTimePosition(tStoredRightPointPosition);
        It->SetStoredRightAggregate(iStoredRightAggregate);
        It->SetStoredHValue(dStoredVValue);
        It->SetHScale(dHScale);
        It->SetOHLC(bOHLC);
        ////////////////////////
        It->SetVScale(mVScale);
        It->SetVVolumeScale(mVVolumeScale);

        if (!tTicker.equalFull(*It)){
            emit NeedSaveTickerConig(*It, true);
        }
    }
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::slotCandleStateChanged(int)
{
    bOHLC = swtCandle->isChecked();
    ui->grViewQuotes->scene()->invalidate(ui->grViewQuotes->sceneRect());
    //ui->grViewQuotes->invalidateScene(ui->grViewQuotes->sceneRect());
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::ResizeMemometer()
{
    QPoint pU   = ui->grViewR1->pos();
    QRect rU    = ui->grViewR1->rect();

    if (!ui->grViewR1->isHidden()){
        if (indicatorMemo->isHidden()){
            indicatorMemo->show();
        }
        indicatorMemo->setGeometry(pU.x() + rU.width() - 12, pU.y() ,10,ui->grViewR1->height());
    }
    else{
        indicatorMemo->hide();
    }


}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::slotUsedMemoryChanged(size_t iM,size_t iTotal)
{
    std::stringstream ss;
    ss <<QString(tr("Memory usage: ")).toStdString()<<MemoSizeToStr(iM);

    double dPercent{1};
    if(iTotal !=0 ){
        dPercent = double(iM)/double(iTotal);
        ss <<QString(tr(" from: ")).toStdString()<<MemoSizeToStr(iTotal);
    }
    indicatorMemo->setValue(dPercent);

    indicatorMemo->setToolTipText(ss.str());
}
//---------------------------------------------------------------------------------------------------------------
std::string GraphViewForm::MemoSizeToStr(size_t iSize)
{
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2);
    if (iSize > 1099511627776){
        ss << (iSize/1099511627776.0) << QString(tr("T")).toStdString();
    }
    else if (iSize > 1073741824){
        ss << (iSize/1073741824.0) << QString(tr("G")).toStdString();
    }
    else if (iSize > 1048576){
        ss << (iSize/1048576.0) << QString(tr("M")).toStdString();
    }
    else if (iSize > 1024){
        ss << (iSize/1024.0) << QString(tr("G")).toStdString();
    }
    else{
        ss << (iSize) << QString(tr("B")).toStdString();
    }
    return ss.str();
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::slotInvertMouseWheelChanged(bool b)
{
    bInvertMouseWheel = b;
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::slotShowHelpButtonsChanged(bool b)
{
    btnHelp->setVisible(b);
    btnHelpR->setVisible(b);
}
//---------------------------------------------------------------------------------------------------------------
bool GraphViewForm::PaintMovingAverages (std::shared_ptr<GraphHolder> local_holder,
                                            int iStartI, int iEndI,
                                            bool bReplacementMode)
{
    if (iSelectedInterval == Bar::eInterval::pTick) return true;
    //
    InvalidateCounterDefender def(aiInvalidateCounter);
//    {
//         ThreadFreeCout pcout;
//         pcout << "PaintMovings <"<<iStartI<<":"<<iEndI<<">\n";
//         pcout << "mMovingBlue.size() <"<<mMovingBlue.size()<<">\n";
//    }
    /////////////////////////////////////////////////////////////////////////////////////////
    if (!local_holder) return true;
    bool bSuccess;
    auto ItDefender = local_holder->beginIteratorByDate<Bar>(iSelectedInterval,0,bSuccess);
    if (!bSuccess) return false;
    //
    int iMaxBlueSize = (int)local_holder->getMovingBlueSize(iSelectedInterval);
    int iMaxRedSize = (int)local_holder->getMovingRedSize(iSelectedInterval);
    int iMaxGreenSize = (int)local_holder->getMovingGreenSize(iSelectedInterval);

    const Graph<Bar>& graph = local_holder->getGraph<Bar>(iSelectedInterval);
    const int iShift {(int)graph.GetShiftIndex()};
    /////////////////////////////////////////////////////////////////////////////////////////

    int iBeg     = iStartI - iShift - 1 >= 0    ? iStartI - iShift - 1 : 0 ;


    int iEndBlue =  iEndI >= 0               ? iEndI             : 0 ;
    int iEndRed     {iEndBlue};
    int iEndGreen   {iEndBlue};

    iEndBlue     = iEndBlue  - iShift < iMaxBlueSize  ? iEndBlue  - iShift     : iMaxBlueSize  - 1;
    iEndRed      = iEndRed   - iShift < iMaxRedSize   ? iEndRed   - iShift     : iMaxRedSize   - 1;
    iEndGreen    = iEndGreen - iShift < iMaxGreenSize ? iEndGreen - iShift     : iMaxGreenSize - 1;

    /////////////////////////////////////////////////////////////////

//    mMovingBlue;
//    mMovingRed;
//    mMovingGreen;

    if (!bReplacementMode)
    {
        EraseLinesUpper(mMovingBlue,  iBeg        + iShift, ui->grViewQuotes->scene());
        EraseLinesLower(mMovingBlue,  iEndBlue    + iShift, ui->grViewQuotes->scene());

        EraseLinesUpper(mMovingRed,   iBeg        + iShift, ui->grViewQuotes->scene());
        EraseLinesLower(mMovingRed,   iEndRed     + iShift, ui->grViewQuotes->scene());

        EraseLinesUpper(mMovingGreen, iBeg        + iShift, ui->grViewQuotes->scene());
        EraseLinesLower(mMovingGreen, iEndGreen   + iShift, ui->grViewQuotes->scene());
    }
    else{

//        EraseLinesMid(mMovingBlue, iBeg + iShift,iEndBlue  + iShift, ui->grViewQuotes->scene());
//        EraseLinesMid(mMovingRed,  iBeg + iShift,iEndRed   + iShift, ui->grViewQuotes->scene());
//        EraseLinesMid(mMovingGreen,iBeg + iShift,iEndGreen + iShift, ui->grViewQuotes->scene());
        EraseLinesLower(mMovingBlue,  iBeg    + iShift, ui->grViewQuotes->scene());
        EraseLinesLower(mMovingRed,   iBeg     + iShift, ui->grViewQuotes->scene());
        EraseLinesLower(mMovingGreen, iBeg   + iShift, ui->grViewQuotes->scene());
    }
    /////////////////////////////////////////////////////////////////
    std::stringstream ss;
    QPen bluePen(Qt::blue,1,Qt::SolidLine);
    QPen redPen(Qt::red,1,Qt::SolidLine);
    QPen greenPen(Qt::green,1,Qt::SolidLine);

    double dCurr{0};
    qreal xCur{0};

    /////////////////////////////////////////////////////////////////
    for (int i = iBeg ; i <= iEndBlue; ++i){
        auto ItFound = mMovingBlue.find(i + iShift);
        if (ItFound == mMovingBlue.end())
        {
            xCur = (i + iShift + iLeftShift)     * BarGraphicsItem::BarWidth * dHScale;
            dCurr = local_holder->getMovingBlueByIndex(iSelectedInterval,i);
            if (dCurr > 0){
                mMovingBlue[i + iShift] = {xCur,-realYtoSceneY(dCurr)};
            }
        }
    }
    /////////////////////////////////////////////////////////////////
    for (int i = iBeg ; i <= iEndRed; ++i){
        auto ItFound = mMovingRed.find(i + iShift);
        if (ItFound == mMovingRed.end())
        {
            xCur = (i + iShift + iLeftShift)     * BarGraphicsItem::BarWidth * dHScale;
            dCurr = local_holder->getMovingRedByIndex(iSelectedInterval,i);
            if (dCurr > 0){
                mMovingRed[i + iShift] = {xCur,-realYtoSceneY(dCurr)};
            }
        }
    }
    /////////////////////////////////////////////////////////////////
    for (int i = iBeg ; i <= iEndGreen; ++i){
        auto ItFound = mMovingGreen.find(i + iShift);
        if (ItFound == mMovingGreen.end())
        {
            xCur = (i + iShift + iLeftShift)     * BarGraphicsItem::BarWidth * dHScale;
            dCurr = local_holder->getMovingGreenByIndex(iSelectedInterval,i);
            if (dCurr > 0){
                mMovingGreen[i + iShift] = {xCur,-realYtoSceneY(dCurr)};
            }
        }
    }
    /////////////////////////////////////////////////////////////////

    if (pathBlue)   grScene->removeItem(pathBlue);
    if (pathRed)    grScene->removeItem(pathRed);
    if (pathGreen)  grScene->removeItem(pathGreen);

    QPainterPath blue   = smoothOut(mMovingBlue, 3);
    QPainterPath red    = smoothOut(mMovingRed, 3);
    QPainterPath green  = smoothOut(mMovingGreen, 3);

    pathBlue    = grScene->addPath(blue,bluePen);
    pathRed     = grScene->addPath(red,redPen);
    pathGreen   = grScene->addPath(green,greenPen);

    pathBlue->setZValue(7);
    pathRed->setZValue(7);
    pathGreen->setZValue(7);

    /////////////////////////////////////////////////////////////////
    return true;
}
//---------------------------------------------------------------------------------------------------------------
float GraphViewForm::distance(const QPointF& pt1, const QPointF& pt2)
{
    float hd = (pt1.x() - pt2.x()) * (pt1.x() - pt2.x());
    float vd = (pt1.y() - pt2.y()) * (pt1.y() - pt2.y());
    return std::sqrt(hd + vd);
}
//---------------------------------------------------------------------------------------------------------------
QPointF GraphViewForm::getLineStart(const QPointF& pt1, const QPointF& pt2)
{
    QPointF pt;
    float rat = 10.0 / distance(pt1, pt2);
    if (rat > 0.5) {
        rat = 0.5;
    }
    pt.setX((1.0 - rat) * pt1.x() + rat * pt2.x());
    pt.setY((1.0 - rat) * pt1.y() + rat * pt2.y());
    return pt;
}
//---------------------------------------------------------------------------------------------------------------
QPointF GraphViewForm::getLineEnd(const QPointF& pt1, const QPointF& pt2)
{
    QPointF pt;
    float rat = 10.0 / distance(pt1, pt2);
    if (rat > 0.5) {
        rat = 0.5;
    }
    pt.setX(rat * pt1.x() + (1.0 - rat)*pt2.x());
    pt.setY(rat * pt1.y() + (1.0 - rat)*pt2.y());
    return pt;
}
//---------------------------------------------------------------------------------------------------------------
QPainterPath GraphViewForm::smoothOut(const std::map<int,QPointF> &map, const float& /*factor*/)
{
    QPainterPath path;
    if (map.size() < 3) {
        return path;
    }
    std::vector<QPointF> vV;
    vV.reserve(map.size());
    for (const auto &p:map){
        vV.push_back(p.second);
    }

    QPointF pt1;
    QPointF pt2;

    for (int i = 0; i < (int)vV.size() - 1; i++) {
        pt1 = getLineStart(vV[i], vV[i + 1]);
        if (i == 0) {
            path.moveTo(pt1);
        } else {
            path.quadTo(vV[i], pt1);
        }
        pt2 = getLineEnd(vV[i], vV[i + 1]);
        path.lineTo(pt2);
    }
    return path;
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::slotScaleHViewDefaultClicked()
{
    slotSetNewHScaleQuotes(1);
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::slotScaleVViewDefaultClicked()
{
    double dNewScale {1.0};
    if ((dStoredHighMax - dStoredLowMin) > 0){
        dNewScale = (iViewPortHeight - iViewPortHighStrip - iViewPortLowStrip)/(dStoredHighMax - dStoredLowMin);
    }
    slotSetNewVScaleQuotes(dNewScale);
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::slotScaleVVolumeDefaultClicked()
{
    double dNewScale = (ui->grViewVolume->maximumHeight() - iVolumeViewPortHighStrip * 2)/(dStoredVolumeHighMax /*- dStoredVolumeLowMin*/);
    if (dNewScale <=0 ) dNewScale = 1;
    slotSetNewVScaleVolume(dNewScale);
}
//---------------------------------------------------------------------------------------------------------------
