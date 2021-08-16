#include "graphviewform.h"
#include "ui_graphviewform.h"
#include "storage.h"

#include<sstream>

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

    ///----------------------------
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
    dVScale             = 20.0;
    dHScale             = 1;
    dTailFreeZone       = 0.1;
    bOHLC               = true;

    connect(ui->btnTestLoad,SIGNAL(clicked()),this,SLOT(slotLoadGraphButton()));
    //------------------------------

    grScene = new QGraphicsScene();
    ui->grViewQuotes->setScene(grScene);

    ui->scrlBarHorizontal->setMinimum(0);
    ui->scrlBarHorizontal->setMaximum(0);
    connect(ui->scrlBarHorizontal,SIGNAL(valueChanged(int)),this,SLOT(slotSliderValueChanged(int)));

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
void GraphViewForm::slotInvalidateGraph(std::time_t dtBegin, std::time_t dtEnd)
{
    bool bSuccess{false};
    queueRepaint.Push({dtBegin,dtEnd});
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
            queueRepaint.Push({dtBegin,dtEnd});
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
            auto ItTotalEnd (holder->end<T>());
            size_t iSize =  holder->getViewGraphSize(iSelectedInterval);
            std::time_t tMinDate = holder->getViewGraphDateMin(Bar::eInterval::pTick);
            std::time_t tMaxDate =  holder->getViewGraphDateMax(Bar::eInterval::pTick);

            auto p = holder->getMinMax(tStoredMinDate,tStoredMaxDate);
            double dLowMin  = p.first;
            double dHighMax = p.second;

            // if neсessary clean all
            if (    tStoredMinDate == 0
                || tMinDate != tStoredMinDate
                || (iStoredMaxSize !=iSize && tMinDate >= tStoredMinDate && tMaxDate <= tStoredMaxDate)
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
            // resize
            if (iStoredMaxSize !=iSize){

                QRectF newRec(0, -dVScale * dLowMin ,(iSize + iLeftShift + iRightShift) * BarGraphicsItem::BarWidth  ,- dVScale * (dHighMax - dLowMin));
                ui->grViewQuotes->setSceneRect(newRec);

                //ui->scrlBarHorizontal->setMinimum(0);

                int iNewSize = iSize + iLeftShift + iRightShift - ui->grViewQuotes->width()/BarGraphicsItem::BarWidth;
                iNewSize = iNewSize < 0? 0 : iNewSize;
                ui->scrlBarHorizontal->setMaximum(iNewSize);
//                {
//                    ThreadFreeCout pcout;
//                    pcout <<"init scrollsize to:"<< iNewSize<<"\n";
//                }
            }

            iStoredMaxSize  = iSize;
            tStoredMinDate  = tMinDate;
            tStoredMaxDate  = tMaxDate;
            dStoredLowMin   = dLowMin;
            dStoredHighMax  = dHighMax;

            //---------------------------
            bool bS{false};
            auto ItMiddle (holder->beginIteratorByDate<T>(iSelectedInterval,tStoredMiddlePointPosition,bS));
            if (tStoredMiddlePointPosition != 0 && bS){
                ui->scrlBarHorizontal->setValue(ItMiddle.realPosition() + iLeftShift);
                //slotSliderValueChanged(ItMiddle.realPosition() + iLeftShift);
            }
            else{
                ui->scrlBarHorizontal->setValue(iSize + iLeftShift + iRightShift - ui->grViewQuotes->width()/BarGraphicsItem::BarWidth);
                //slotSliderValueChanged(iSize + iLeftShift);
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

//    {
//        ThreadFreeCout pcout;
//        pcout <<"new scroll walue: "<< iBeg - iLeftShift<<"\n";
//    }

    ////////////////////
   // int iEnd = iBeg + ui->grViewQuotes->width()/BarGraphicsItem::BarWidth;
    int iMiddle = iBeg + (ui->grViewQuotes->width()/(2*BarGraphicsItem::BarWidth));
//    {
//        ThreadFreeCout pcout;
//        pcout << "iBeg: "<<iBeg<<"\n";
//        pcout << "iEnd: "<<iEnd<<"\n";
//        pcout << "iMiddle: "<<iMiddle<<"\n";
//    }
    if (iSelectedInterval == Bar::eInterval::pTick){
        SliderValueChanged<BarTick>(iBeg);
    }
    else{
        SliderValueChanged<Bar>(iBeg);
    }
//- iLeftShift
    ui->grViewQuotes->centerOn((iMiddle /*- iLeftShift*/)*BarGraphicsItem::BarWidth,0);
    //ui->grViewQuotes->horizontalScrollBar()->setValue(i*BarGraphicsItem::BarWidth);
}
//---------------------------------------------------------------------------------------------------------------
template<typename T>
void GraphViewForm::SliderValueChanged(int iPos)
{


    //
    size_t iBeg      = iPos > iLeftShift ? iPos - iLeftShift : 0 ;
    size_t iEnd      = iBeg + ui->grViewQuotes->width()/BarGraphicsItem::BarWidth;
    size_t iMiddle   = iBeg + (iEnd - iBeg)/2;
    /////////////////////////////////////////////////////////////////
    bool bSuccess;
    auto ItDefender = holder->beginIteratorByDate<T>(iSelectedInterval,0,bSuccess);
    if (!bSuccess) return;
    /////////////////////////////////////////////////////////////////
    size_t iMaxSize = holder->getViewGraphSize(iSelectedInterval);
    if (iMaxSize ==0 ) return;
    /////////////////////////////////////////////////////////////////
//    {
//        ThreadFreeCout pcout;
//        pcout << "t iMaxSize: "<<iMaxSize<<"\n";
//        pcout << "t iBeg: "<<iBeg<<"\n";
//        pcout << "t iEnd: "<<iEnd<<"\n";
//    }

    iBeg    = iBeg >= 0 ? iBeg : 0;
    iEnd    = iEnd >= 0 ? iEnd : 0;
    iMiddle = iMiddle >= 0 ? iMiddle : 0;

    iBeg = iBeg < iMaxSize ? iBeg : iMaxSize - 1;
    iEnd = iEnd < iMaxSize ? iEnd : iMaxSize - 1;
    iMiddle = iMiddle < iMaxSize ? iMiddle:iMaxSize - 1;

//    {
//        ThreadFreeCout pcout;
//        pcout << "real iBeg: "<<iBeg<<"\n";
//        pcout << "real iEnd: "<<iEnd<<"\n";
//    }

    if (bSuccess && iMaxSize > 0){
        //TODO:: change tails addition depend on scale
        T tM = holder->getByIndex<T>(iSelectedInterval,iMiddle);
        T tB = holder->getByIndex<T>(iSelectedInterval,iBeg - 100/dHScale > 0        ? iBeg - 100/dHScale : 0);
        T tE = holder->getByIndex<T>(iSelectedInterval,iEnd + 100/dHScale < iMaxSize ? iEnd + 100/dHScale :iMaxSize-1);
//        {
//            ThreadFreeCout pcout;
//            pcout << "here 1\n";
//            pcout << "here 1\n";
//            pcout << "tB: "<<tB.Period()<<"\n";
//            pcout << "tE: "<<tB.Period()<<"\n";

//        }

        tStoredMiddlePointPosition = tM.Period();
        ////////////

        size_t j=0;
        while (j < vShowedGraphicsBars.size()){
            if (vShowedGraphicsBars[j]->Period() < tB.Period() ||
                vShowedGraphicsBars[j]->Period() > tE.Period() ){
               ui->grViewQuotes->scene()->removeItem(vShowedGraphicsBars[j]);
               delete (vShowedGraphicsBars[j]);
               vShowedGraphicsBars.erase(std::next(vShowedGraphicsBars.begin(),j));
            }
            else{
                ++j;
            }
        }

//        {
//            ThreadFreeCout pcout;
//            pcout << "here 2\n";
//        }

        ///////////
        auto It = holder->beginIteratorByDate<T>(iSelectedInterval,tB.Period(),bSuccess);
        auto ItEnd = holder->beginIteratorByDate<T>(iSelectedInterval,tE.Period()+1,bSuccess);
        auto ItTotalEnd = holder->end<T>();

//        {/// test case
//        if (It != ItEnd){
//            ThreadFreeCout pcout;
//            std::time_t tB{It->Period()};
//            std::string sB (threadfree_localtime_to_str(&tB));
//            pcout <<"adding bars from: "<<sB<<" {"<<It.realPosition()<<"}\n";
//            auto ItTheEnd = holder->end<T>();
//            if (ItEnd != ItTheEnd){
//                std::time_t tE{ItEnd->Period()};
//                std::string sE (threadfree_localtime_to_str(&tE));
//                pcout <<"adding bars to: "<<sE<<" {"<<ItEnd.realPosition()<<"}\n";
//            }
//            else{
//                pcout <<"adding bars to the end\n";
//            }
//        }
//        }/// test case end

//        {
//            ThreadFreeCout pcout;
//            pcout << "here 3\n";
//        }

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
                item->setPos((It.realPosition() + iLeftShift) * BarGraphicsItem::BarWidth , -dVScale * It->Close());
            }//- iLeftShift
            ++It;
        }
    }
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::resizeEvent(QResizeEvent *event)
{
    int iNewSize = iStoredMaxSize + iLeftShift + iRightShift - ui->grViewQuotes->width()/BarGraphicsItem::BarWidth;
    iNewSize = iNewSize < 0? 0 : iNewSize;
    ui->scrlBarHorizontal->setMaximum(iNewSize);

    QWidget::resizeEvent(event);
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::showEvent(QShowEvent *event)
{
    int iNewSize = iStoredMaxSize + iLeftShift + iRightShift - ui->grViewQuotes->width()/BarGraphicsItem::BarWidth;
    iNewSize = iNewSize < 0? 0 : iNewSize;
    ui->scrlBarHorizontal->setMaximum(iNewSize);

    QWidget::showEvent(event);
}
//---------------------------------------------------------------------------------------------------------------
