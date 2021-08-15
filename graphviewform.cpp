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

    connect(ui->btnTestLoad,SIGNAL(clicked()),this,SLOT(slotLoadGraphButton()));
    connect(ui->btnTest2,SIGNAL(clicked()),this,SLOT(slotLoadGraphButton2()));


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
void GraphViewForm::slotInvalidateGraph(std::time_t dtBegin, std::time_t dtEnd)
{
    {
        ThreadFreeCout pcout;

        char buffer[100];
        std::tm * ptb = threadfree_localtime(&dtBegin);
        std::strftime(buffer, 100, "%Y/%m/%d %H:%M:%S", ptb);
        std::string strb(buffer);

        std::tm * pte = threadfree_localtime(&dtEnd);
        std::strftime(buffer, 100, "%Y/%m/%d %H:%M:%S", pte);
        std::string stre(buffer);

        pcout<< "GraphViewForm:\n";
        pcout<< "invalidate from: "<<strb<<"\n";
        pcout<< "invalidate to: "<<stre<<"\n";

        Bar::eInterval it{Bar::eInterval::pTick};

        pcout<< " elements total: "<<holder->getViewGraphSize(it)<<"\n";

        std::time_t dtMaxBegin  = holder->getViewGraphDateMin(it);
        std::time_t dtMaxEnd    = holder->getViewGraphDateMax(it);

        std::tm * ptbM = threadfree_localtime(&dtMaxBegin);
        std::strftime(buffer, 100, "%Y/%m/%d %H:%M:%S", ptbM);
        std::string strbM(buffer);

        std::tm * pteM = threadfree_localtime(&dtMaxEnd);
        std::strftime(buffer, 100, "%Y/%m/%d %H:%M:%S", pteM);
        std::string streM(buffer);

        pcout<< "has data from: "<<strbM<<"\n";
        pcout<< "has data to: "<<streM<<"\n";



    }
    {
        //std::unique_lock lk(holder->mutexHolder);;

        ThreadFreeCout pcout;
        bool bSuccess;
        It =  holder->beginIteratorByDate<BarTick>(Bar::eInterval::pTick, dtBegin, bSuccess);
        auto ItEndT = holder->end<BarTick>();
        auto ItEnd(ItEndT);
        if (bSuccess)
        {

            bool bPlainIncremented{true};
            int iCount{0};
            std::time_t tTmp{0};

            auto ItNew (std::next(It,537410));
            //auto ItNew (std::next(It,637410));
            while(ItNew != ItEnd){
                if ((*It).Period() < tTmp
                        || (*It).Period() > dtEnd
                        || (*It).Period() < dtBegin
                        ){
                    bPlainIncremented   = false;
                }
                else{
                    tTmp  = (*It).Period();
                }
                pcout<< "close: "<<(*It).Close()<<"\n";

                iCount++;

                ++ItNew;
            }

            pcout<< "total iterator: "<<iCount<<"\n";
            pcout<< "bPlainIncremented: "<<bPlainIncremented<<"\n";
            pcout<< "It owns_lock: "<<It.owns_lock()<<"\n";
            pcout<< "ItNew owns_lock: "<<ItNew.owns_lock()<<"\n";
//            It.ulock();
//            ItNew.ulock();
//            pcout<< "do unlock\n";
//            pcout<< "It owns_lock: "<<It.owns_lock()<<"\n";
//            pcout<< "ItNew owns_lock: "<<ItNew.owns_lock()<<"\n";
        }
        else{
            pcout<< "cannot lock mutex.\n";
        }

    }

}
//---------------------------------------------------------------------------------------------------------------
void GraphViewForm::slotLoadGraphButton2()
{
    {
        ThreadFreeCout pcout;
        pcout<< "\n\rIt owns_lock: "<<It.owns_lock()<<"\n";
    }

    It = holder->end<BarTick>();
    It.ulock();

    {
        ThreadFreeCout pcout;
        pcout <<"trying unlock\n";
        pcout<< "It owns_lock: "<<It.owns_lock()<<"\n";

//        std::unique_lock lk(holder->mutexHolder, std::defer_lock);
//        bool locked = !lk.try_lock();
//        pcout<< "realy locked: "<<locked<<"\n";
//        holder->mutexHolder.unlock();
    }

}
//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------

