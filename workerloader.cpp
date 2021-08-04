#include "workerloader.h"
#include "threadfreecout.h"
#include "threadpool.h"



#include<sstream>
#include<chrono>

using namespace std::chrono_literals;


workerLoader::workerLoader()
{

}

void workerLoader::workerEtalon(BlockFreeQueue<dataFinLoadTask> & queueFilLoad,
                               BlockFreeQueue<dataBuckgroundThreadAnswer> &queueTrdAnswers)
{
    //ThreadFreeCout fout;
    //fout<<"workerLoaderFinam in\n";

    bool bWasBreaked{false};
    bool bSuccess{false};
    auto pdata = queueFilLoad.Pop(bSuccess);
    while(bSuccess){

        dataFinLoadTask data(*pdata.get());

        queueTrdAnswers.Push({dataBuckgroundThreadAnswer::eAnswerType::famLoadBegin,data.GetParentWnd()});

        ////////////////////////////////////////////////////////////
        dataBuckgroundThreadAnswer dt(dataBuckgroundThreadAnswer::eAnswerType::TextInfoMessage,data.GetParentWnd());
        std::stringstream ss;
        char buffer[100];
        std::tm * ptmB = std::localtime(&data.dtBegin);
        std::strftime(buffer, 100, "%Y/%m/%d %H:%M:%S", ptmB);
        std::string strB(buffer);

        std::tm * ptmE = std::localtime(&data.dtEnd);
        std::strftime(buffer, 100, "%Y/%m/%d %H:%M:%S", ptmE);
        std::string strE(buffer);

        ss << "Loading <"<<data.sSign<<">\n";
        ss << "TickerID: <"<<data.TickerID<<">\n";
        ss << "Interval: <"<<data.iInterval<<">\n";
        ss << "file: <"<<data.pathFileName<<">\n";
        ss << "Begin date: <"<<strB<<">\n";
        ss << "End date: <"<<strE<<">\n";
        dt.SetTextInfo(ss.str());
        queueTrdAnswers.Push(dt);
        ////////////////////////////////////////////////////////////
        for (int i =0; i<=1000; ++i){
            if(this_thread_flagInterrup.isSet())
            {
                //fout<<"exit on interrupt\n";
                dataBuckgroundThreadAnswer dt(dataBuckgroundThreadAnswer::eAnswerType::famLoadEnd,data.GetParentWnd());
                dt.SetSuccessfull(false);
                dt.SetErrString("loading process interrupted");
                queueTrdAnswers.Push(dt);
                bWasBreaked = true;
                break;
            }
            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //// work area
            if (i %10 == 0){
                dataBuckgroundThreadAnswer dt(dataBuckgroundThreadAnswer::eAnswerType::famLoadCurrent,data.GetParentWnd());
                dt.SetPercent(i/10);
                queueTrdAnswers.Push(dt);
            }
            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            dataBuckgroundThreadAnswer dt(dataBuckgroundThreadAnswer::eAnswerType::LoadActivity,data.GetParentWnd());
            ///
            std::this_thread::sleep_for(4ms);

        }
        if (!bWasBreaked){
            dataBuckgroundThreadAnswer dt(dataBuckgroundThreadAnswer::eAnswerType::famLoadEnd,data.GetParentWnd());
            dt.SetSuccessfull(true);
            queueTrdAnswers.Push(dt);
            pdata = queueFilLoad.Pop(bSuccess);
        }
        else{
            break;
        }
        //----------------------------------
    }
    //fout<<"workerLoaderFinam out\n";
}

void workerLoader::workerFinQuotesLoad(BlockFreeQueue<dataFinLoadTask> & queueFilLoad,
                                BlockFreeQueue<dataBuckgroundThreadAnswer> &queueTrdAnswers,
                                 Storage & stStore)
{
    //ThreadFreeCout fout;
    //fout<<"workerLoaderFinam in\n";

    bool bWasBreaked{false};
    bool bSuccess{false};
    auto pdata = queueFilLoad.Pop(bSuccess);
    while(bSuccess){

        dataFinLoadTask data(*pdata.get());

        queueTrdAnswers.Push({dataBuckgroundThreadAnswer::eAnswerType::famLoadBegin,data.GetParentWnd()});

        ////////////////////////////////////////////////////////////
        dataBuckgroundThreadAnswer dt(dataBuckgroundThreadAnswer::eAnswerType::TextInfoMessage,data.GetParentWnd());
        std::stringstream ss;
        char buffer[100];
        std::tm * ptmB = std::localtime(&data.dtBegin);
        std::strftime(buffer, 100, "%Y/%m/%d %H:%M:%S", ptmB);
        std::string strB(buffer);

        std::tm * ptmE = std::localtime(&data.dtEnd);
        std::strftime(buffer, 100, "%Y/%m/%d %H:%M:%S", ptmE);
        std::string strE(buffer);
        ss << "Loading <"<<data.sSign<<">\n";
        ss << "TickerID: <"<<data.TickerID<<">\n";
        ss << "Interval: <"<<data.iInterval<<">\n";
        ss << "file: <"<<data.pathFileName<<">\n";
        ss << "Begin date: <"<<strB<<">\n";
        ss << "End date: <"<<strE<<">\n";
        dt.SetTextInfo(ss.str());
        queueTrdAnswers.Push(dt);
        ////////////////////////////////////////////////////////////
        if (!stStore.InitializeTicker(data.TickerID)){
            dataBuckgroundThreadAnswer dt(dataBuckgroundThreadAnswer::eAnswerType::famLoadEnd,data.GetParentWnd());
            dt.SetSuccessfull(false);
            dt.SetErrString("cannot initialize stock quotes storage for ticker");
            queueTrdAnswers.Push(dt);
        }
        else{
            dataBuckgroundThreadAnswer dtT (dataBuckgroundThreadAnswer::eAnswerType::TextInfoMessage,data.GetParentWnd());
            dtT.SetTextInfo("Here we are!");
            queueTrdAnswers.Push(dt);


            dataBuckgroundThreadAnswer dt(dataBuckgroundThreadAnswer::eAnswerType::famLoadEnd,data.GetParentWnd());
            dt.SetSuccessfull(true);
            queueTrdAnswers.Push(dt);
        }
        ////////////////////////////////////////////////////////////



//        for (int i =0; i<=1000; ++i){
//            if(this_thread_flagInterrup.isSet())
//            {
//                //fout<<"exit on interrupt\n";
//                dataBuckgroundThreadAnswer dt(dataBuckgroundThreadAnswer::eAnswerType::famLoadEnd,data.GetParentWnd());
//                dt.SetSuccessfull(false);
//                dt.SetErrString("loading process interrupted");
//                queueTrdAnswers.Push(dt);
//                bWasBreaked = true;
//                break;
//            }
//            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//            //// work area
//            if (i %10 == 0){
//                dataBuckgroundThreadAnswer dt(dataBuckgroundThreadAnswer::eAnswerType::famLoadCurrent,data.GetParentWnd());
//                dt.SetPercent(i/10);
//                queueTrdAnswers.Push(dt);
//            }
//            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//            dataBuckgroundThreadAnswer dt(dataBuckgroundThreadAnswer::eAnswerType::LoadActivity,data.GetParentWnd());
//            ///
//            //std::this_thread::sleep_for(4ms);

//        }
        if (!bWasBreaked){
            pdata = queueFilLoad.Pop(bSuccess);
        }
        else{
            break;
        }
        //----------------------------------
    }
    //fout<<"workerLoaderFinam out\n";
}


