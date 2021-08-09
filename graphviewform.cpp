#include "graphviewform.h"
#include "ui_graphviewform.h"

#include<sstream>

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

}

GraphViewForm::~GraphViewForm()
{
    delete ui;
}
