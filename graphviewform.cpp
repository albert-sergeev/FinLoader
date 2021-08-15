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
    ui(new Ui::GraphViewForm)
{
    ui->setupUi(this);

   // ui->grViewQuotes->setHorizontalScrollBar(ui->scrlBarHorizontal);
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
    tStartViewPosition  = 0;
    dHScale             = 20.0;
    dVScale             = 1;
    dTailFreeZone       = 0.1;
    bOHLC               = true;
    //------------------------------
    connect(ui->btnTestLoad,SIGNAL(clicked()),this,SLOT(slotLoadGraphButton()));

    //QGraphicsScene scene(QRectF(-100,-100,600,600));

    grScene = new QGraphicsScene(/*QRectF(-100,-100,600,600)*/);
    ui->grViewQuotes->setScene(grScene);


    if (iSelectedInterval  == Bar::eInterval::pTick)
    {

        // lock holder
        bool bSuccess{false};
        auto ItDefender(holder->beginIteratorByDate<BarTick>(iSelectedInterval,0,bSuccess));
        if (bSuccess){ // if locked

            size_t iSize =  holder->getViewGraphSize(iSelectedInterval);
            std::time_t tMin =  holder->getViewGraphDateMin(Bar::eInterval::pTick);
            std::time_t tMax =  holder->getViewGraphDateMax(Bar::eInterval::pTick);

            auto p = holder->getMinMax(tMin,tMax);
            double dLowMin  = p.first;
            double dHighMax = p.second;


             //-dHScale * It->Close()
//            QRectF newRec(0, -dHScale * dLowMin  ,iSize * BarGraphicsItem::BarWidth  , dHScale * (dHighMax - dLowMin));
//            ui->grViewQuotes->setSceneRect(newRec);

//            x	-4
//            y	-5821.8
//            width	707
//            height	168.6

             QRectF newRec(0, -dHScale * dLowMin ,iSize * BarGraphicsItem::BarWidth  ,- dHScale * (dHighMax - dLowMin));
             ui->grViewQuotes->setSceneRect(newRec);



//            {
//                QRectF currRectF = ui->grViewQuotes->sceneRect();
//                ThreadFreeCout pcout;
//                pcout <<"constructor:\n";
//                pcout <<"x\t"<<currRectF.x()<<"\n";
//                pcout <<"y\t"<<currRectF.y()<<"\n";
//                pcout <<"width\t"<<currRectF.width()<<"\n";
//                pcout <<"height\t"<<currRectF.height()<<"\n";
//            }

            auto It(holder->beginIteratorByDate<BarTick>(iSelectedInterval,tMin,bSuccess));
            if (bSuccess){
                auto ItEnd(holder->beginIteratorByDate<BarTick>(iSelectedInterval,tMax+1,bSuccess));
                if (bSuccess){
                    slotAddBarTicksToView(It,ItEnd);
                }
            }
            {
                QRectF currRectF = ui->grViewQuotes->sceneRect();
                ThreadFreeCout pcout;
                pcout <<"constructor:\n";
                pcout <<"x\t"<<currRectF.x()<<"\n";
                pcout <<"y\t"<<currRectF.y()<<"\n";
                pcout <<"width\t"<<currRectF.width()<<"\n";
                pcout <<"height\t"<<currRectF.height()<<"\n";
            }




//            auto strMin = threadfree_localtime_to_str(&tMin);
//            auto strMax = threadfree_localtime_to_str(&tMax);
//            ThreadFreeCout pcout;
//            std::stringstream ss;
//            ss << "date from: "<< strMin<<"\n";
//            ss << "date to: "<< strMax<<"\n";
//            ss << "{Min:Max}: <"<<dLowMin<<":"<<dHighMax<<">\n";
//            pcout <<ss.str();
        }
    }
    else{
        ThreadFreeCout pcout;
        pcout <<"GraphViewForm::GraphViewForm: No handler for other intervals!!!!!!!!:\n";
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
        ///
        if (iSelectedInterval == Bar::eInterval::pTick) {
            auto It (holder->beginIteratorByDate<BarTick>(iSelectedInterval,data.dtStart,bSuccess));
            if (bSuccess){
                auto ItEnd (holder->beginIteratorByDate<BarTick>(iSelectedInterval,data.dtEnd,bSuccess));
                if (bSuccess){

//                    std::time_t tMin =  holder->getViewGraphDateMin(Bar::eInterval::pTick);
//                    std::time_t tMax =  holder->getViewGraphDateMax(Bar::eInterval::pTick);

                    auto p = holder->getMinMax(data.dtStart,data.dtEnd);
//                    qreal dLowMin  = -p.first;
//                    qreal dHighMax = -p.second;


//                    QRectF currRectF = ui->grViewQuotes->sceneRect();
//                    if (currRectF.y() !=0 && dLowMin < currRectF.y()) {
//                        dLowMin = currRectF.y();
//                    }
//                    if (dHighMax > currRectF.y() - currRectF.height()) {
//                        dHighMax = currRectF.y() - currRectF.height();
//                    }


//                    QRectF newRec(tMin,
//                                  dLowMin,
//                                  tMax  - tMin,
//                                  -(dHighMax - dLowMin));

//                    ui->grViewQuotes->setSceneRect(newRec);

//                    std::string strStart    = threadfree_localtime_to_str(&data.dtStart);
//                    std::string strEnd      = threadfree_localtime_to_str(&data.dtEnd);

//                    emit SendToLog("invalidate from: " + QString::fromStdString(strStart));
//                    emit SendToLog("invalidate to: " + QString::fromStdString(strEnd));

//                    currRectF = ui->grViewQuotes->sceneRect();
                    {
                    ThreadFreeCout pcout;
                    pcout <<"invalidate:\n";
                    pcout << "{Min:Max}: <"<<p.first<<":"<<p.second<<">\n";
//                    pcout <<"x\t"<<currRectF.x()<<"\n";
//                    pcout <<"y\t"<<currRectF.y()<<"\n";
//                    pcout <<"width\t"<<currRectF.width()<<"\n";
//                    pcout <<"height\t"<<currRectF.height()<<"\n";
                    }
                }
            }
        }
        else{
            auto It (holder->beginIteratorByDate<Bar>(iSelectedInterval,data.dtStart,bSuccess));
            if (bSuccess){
                auto ItEnd (holder->beginIteratorByDate<Bar>(iSelectedInterval,data.dtEnd,bSuccess));
                if (bSuccess){
                    std::string strStart    = threadfree_localtime_to_str(&data.dtStart);
                    std::string strEnd      = threadfree_localtime_to_str(&data.dtEnd);

                    emit SendToLog("invalidate from: " + QString::fromStdString(strStart));
                    emit SendToLog("invalidate to: " + QString::fromStdString(strEnd));
                }
            }
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
void GraphViewForm::slotLoadGraphButton2()
{

}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::slotAddBarTicksToView(GraphHolder::Iterator<BarTick> It, GraphHolder::Iterator<BarTick> ItEnd)
{
    int iCount{0};
//TODO: std::distance
    //auto iDist = std::distance(ItEnd,It);
    auto iDist = holder->getViewGraphSize(Bar::eInterval::pTick);
    auto parts(iDist/1000);
    while(It != ItEnd){
        BarGraphicsItem *item = new BarGraphicsItem(*It,3);
        ui->grViewQuotes->scene()->addItem(item);
        item->setPos(iCount * BarGraphicsItem::BarWidth , -dHScale * It->Close());

        It = std::next(It,parts);

        iCount++;
//        if (iCount>100) break;
    }
    ThreadFreeCout pcout;
    pcout<<"added {"<<iCount<< "} Ticks to BarGraphicsItem\n";
    pcout<<"iDist {"<<iDist<< "} Ticks to BarGraphicsItem\n";

}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::slotAddBarsToView(GraphHolder::Iterator<Bar> /*It*/, GraphHolder::Iterator<Bar> /*ItEnd*/)
{
    int iCount{0};
//    while(It != ItEnd){
//        BarGraphicsItem *item = new BarGraphicsItem(*It,3);
//        ui->grViewQuotes->scene()->addItem(item);
//        item->setPos(iCount * BarGraphicsItem::BarWidth , -dHScale * It->Close());
//        It++;iCount++;
//    }
    ThreadFreeCout pcout;
    pcout<<"added {"<<iCount<< "} Bars to BarGraphicsItem\n";

}
//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------

