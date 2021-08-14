#include "graphviewform.h"
#include "ui_graphviewform.h"
#include "storage.h"

#include<sstream>

//---------------------------------------------------------------------------------------------------------------
GraphViewForm::GraphViewForm(const int TickerID, std::vector<Ticker> &v, QWidget *parent) :
    QWidget(parent),
    iTickerID{TickerID},
    tTicker{0,"","",1},
    vTickersLst{v},
    ui(new Ui::GraphViewForm)
{
    ui->setupUi(this);
    ///----------------------------
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

    connect(ui->btnTestLoad,SIGNAL(clicked()),this,SLOT(slotLoadGraphButton()));

}
//---------------------------------------------------------------------------------------------------------------
GraphViewForm::~GraphViewForm()
{
    delete ui;
}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::slotLoadGraphButton()
{
//    std::tm tmSt;
//    tmSt.tm_year    = 2021       - 1900;
//    tmSt.tm_mon     = 1      - 1;
//    tmSt.tm_mday    = 15;
//    tmSt.tm_hour    = 10;
//    tmSt.tm_min     = 45;
//    tmSt.tm_sec     = 12;
//    tmSt.tm_isdst   = 0;

//    std::time_t tBegin  = std::mktime(&tmSt);

//    tmSt.tm_mon     = 8      - 1;
//    tmSt.tm_mday    = 5;
//    tmSt.tm_hour    = 18;
//    tmSt.tm_min     = 31;
//    tmSt.tm_sec     = 45;

//    std::time_t tEnd    = std::mktime(&tmSt);

    std::time_t tNow =std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::time_t tBegin = Storage::dateAddMonth(tNow,-12);
    std::time_t tEnd = tNow;

    emit NeedLoadGraph(iTickerID, tBegin, tEnd);
}
//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------

