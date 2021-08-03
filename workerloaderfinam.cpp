#include "workerloaderfinam.h"
#include "threadfreecout.h"
#include "threadpool.h"



#include<sstream>
#include<chrono>

using namespace std::chrono_literals;


workerLoaderFinam::workerLoaderFinam()
{

}


void workerLoaderFinam::worker(BlockFreeQueue<dataFinamLoadTask> & queueFilamLoad,
                               BlockFreeQueue<dataBuckgroundThreadAnswer> &queueTrdAnswers)
{
    ThreadFreeCout fout;
    fout<<"workerLoaderFinam in\n";

    bool bSuccess{false};
    auto pdata = queueFilamLoad.Pop(bSuccess);
    while(bSuccess){
        dataFinamLoadTask data(*pdata.get());
        fout << "Loading <"<<data.sSign<<">\n";
        fout << "TickerID: <"<<data.TickerID<<">\n";
        fout << "Interval: <"<<data.iInterval<<">\n";
        fout << "file: <"<<data.pathFileName<<">\n";


        char buffer[100];
        std::tm * ptmB = std::localtime(&data.dtBegin);
        std::strftime(buffer, 100, "%Y/%m/%d %H:%M:%S", ptmB);
        std::string strB(buffer);
        fout << "Data begin: <"<<strB<<">\n";

        std::tm * ptmE = std::localtime(&data.dtEnd);
        std::strftime(buffer, 100, "%Y/%m/%d %H:%M:%S", ptmE);
        std::string strE(buffer);
        fout << "Data end: <"<<strE<<">\n";
        ////////////////////////////////////////////////////////////
        queueTrdAnswers.Push({dataBuckgroundThreadAnswer::eAnswerType::famLoadBegin,data.GetParentWnd()});
        for (int i =0; i<1000; ++i){
            if(this_thread_flagInterrup.isSet())
            {
                fout<<"exit on interrupt\n";
                break;
            }
            if (i %10 == 0){
                dataBuckgroundThreadAnswer dt(dataBuckgroundThreadAnswer::eAnswerType::famLoadCurrent,data.GetParentWnd());
                dt.SetPercent(i/10);
                queueTrdAnswers.Push(dt);
            }
            dataBuckgroundThreadAnswer dt(dataBuckgroundThreadAnswer::eAnswerType::LoadActivity,data.GetParentWnd());
            ///
            std::this_thread::sleep_for(2ms);

        }
        queueTrdAnswers.Push({dataBuckgroundThreadAnswer::eAnswerType::famLoadEnd,data.GetParentWnd()});
        //----------------------------------
        pdata = queueFilamLoad.Pop(bSuccess);
    }

    //    std::string str;
    //    std::ostringstream ss(str);

//    for ( int i=0; i < 50000000;++i){
//        if(this_thread_flagInterrup.isSet())
//        {
//            fout<<"exit on interrupt\n";
//            break;
//        }
//        ss<<i<<"ddd\n";
//    }

    //unsigned long t = (unsigned long)&this_thread_flagInterrup;
    //fout<<"got in thread ["<<t<<"]\n";
    fout<<"workerLoaderFinam out\n";
}


