#include "workerloader.h"
#include "threadfreecout.h"
#include "threadpool.h"



#include<sstream>
#include<fstream>
#include<ostream>
#include<chrono>

using namespace std::chrono_literals;
using seconds=std::chrono::duration<double>;
using milliseconds=std::chrono::duration<double,
    std::ratio_multiply<seconds::period,std::milli>
    >;


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
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// one task start

        queueTrdAnswers.Push({dataBuckgroundThreadAnswer::eAnswerType::famLoadBegin,data.GetParentWnd()});

        std::chrono::time_point dtStart(std::chrono::steady_clock::now());
        bool bWasSuccessfull{false};

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
        std::stringstream ssErr;
        if (!stStore.InitializeTicker(data.TickerID,ssErr)){
            dataBuckgroundThreadAnswer dt(dataBuckgroundThreadAnswer::eAnswerType::famLoadEnd,data.GetParentWnd());
            dt.SetSuccessfull(false);
            ssErr<<"\ncannot initialize stock quotes storage for ticker";
            dt.SetErrString(ssErr.str());
            queueTrdAnswers.Push(dt);
        }
        else{
            if (ssErr.str().size()>0){
                dataBuckgroundThreadAnswer dtT (dataBuckgroundThreadAnswer::eAnswerType::TextInfoMessage,data.GetParentWnd());
                dtT.SetTextInfo(ssErr.str());
                queueTrdAnswers.Push(dtT);
            }
            ////////////////////////////////////////////////////////////////////////
            //  4. Передача в write уже подготовленного буфера
            //  6. Проверка как будет быстрее, все сразу писать или частями
            //  7. Передача в wtite очериеди ответов и посылка currPercent оттуда
            //  8. добавить количество потоков в инициализацию threadpool
            ///////////////////////////////////////////////////////////////////////////

            std::istringstream issTmp;
            std::ostringstream ossErr;

            //dataFinQuotesParse parseDt(data.parseData,&issTmp,&ossErr);
            dataFinQuotesParse parseDt(&issTmp,&ossErr);
            parseDt = data.parseData;

            bool bFileOpened{false} ;

            if(std::filesystem::exists(data.pathFileName)    &&
               std::filesystem::is_regular_file(data.pathFileName)
                    ){
                std::ifstream file(data.pathFileName);

                if(file.good()){
                    bFileOpened = true;
                    ////
                    file.seekg(0,std::ios::end);
                    size_t filesize = file.tellg();
                    size_t currsize = file.tellg();
                    size_t iProgressSent{0};
                    size_t iCurrProgress{0};
                    file.seekg(0, std::ios::beg);
                    {
                        std::stringstream ss;
                        ss << "filesize: " << filesize<<"\n";
                        dataBuckgroundThreadAnswer dt (dataBuckgroundThreadAnswer::eAnswerType::TextInfoMessage,data.GetParentWnd());
                        dt.SetTextInfo(ss.str());
                        queueTrdAnswers.Push(dt);
                    }
                    /////////////////////////////
                    std::string sBuff;
                    std::istringstream iss;
                    Bar bb(0,0,0,0,0,0);

                    bWasSuccessfull = true;

                    if (parseDt.HasHeader()){ // skip header
                        std::getline(file,sBuff);
                    }

                    while (std::getline(file,sBuff)) {
                        // link stringstream
                        iss.clear();
                        iss.str(sBuff);
                        ////////////
                        if (!Storage::slotParseLine(parseDt, iss, bb)){
                            dataBuckgroundThreadAnswer dt(dataBuckgroundThreadAnswer::eAnswerType::famLoadEnd,data.GetParentWnd());
                            dt.SetSuccessfull(false);
                            ssErr<<iss.str();
                            ssErr<<ossErr.str();
                            dt.SetErrString(ssErr.str());
                            queueTrdAnswers.Push(dt);
                            bWasSuccessfull = false;
                            break;
                        }
                        ////////////////////////////////////////////////////////////////////////////////////////////////////////////
                        ///
                        /// storage work

                        ////////////////////////////////////////////////////////////////////////////////////////////////////////////
                        currsize = file.tellg();
                        iCurrProgress = int(100*(currsize/(double)filesize));
                        if(iProgressSent < iCurrProgress){
                            dataBuckgroundThreadAnswer dt(dataBuckgroundThreadAnswer::eAnswerType::famLoadCurrent,data.GetParentWnd());
                            dt.SetPercent(iCurrProgress);
                            queueTrdAnswers.Push(dt);
                        }
                        ////
                        if(this_thread_flagInterrup.isSet()){
                            //fout<<"exit on interrupt\n";
                            dataBuckgroundThreadAnswer dt(dataBuckgroundThreadAnswer::eAnswerType::famLoadEnd,data.GetParentWnd());
                            dt.SetSuccessfull(false);
                            dt.SetErrString("loading process interrupted");
                            queueTrdAnswers.Push(dt);
                            bWasBreaked = true;
                            bWasSuccessfull = false;
                            break;
                        }
                    }
                }
            }
            /////
            if (!bFileOpened){
                dataBuckgroundThreadAnswer dt(dataBuckgroundThreadAnswer::eAnswerType::famLoadEnd,data.GetParentWnd());
                dt.SetSuccessfull(false);
                ssErr<<"\ncannot open file :"<<data.pathFileName<<"\n";
                dt.SetErrString(ssErr.str());
                queueTrdAnswers.Push(dt);
            }

            ////////////////////////////////////////////////////////////////////////
            if(bWasSuccessfull){
                std::chrono::time_point dtStop(std::chrono::steady_clock::now());
                milliseconds tCount = dtStop - dtStart;
                {
                    std::stringstream ss;
                    ss << "Load time: "<<tCount.count()<<" milliseconds\n";
                    dataBuckgroundThreadAnswer dt (dataBuckgroundThreadAnswer::eAnswerType::TextInfoMessage,data.GetParentWnd());
                    dt.SetTextInfo(ss.str());
                    queueTrdAnswers.Push(dt);
                }

                dataBuckgroundThreadAnswer dt(dataBuckgroundThreadAnswer::eAnswerType::famLoadEnd,data.GetParentWnd());
                dt.SetSuccessfull(true);
                queueTrdAnswers.Push(dt);
            }
        }
        /// one task end
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        if (!bWasBreaked){
            pdata = queueFilLoad.Pop(bSuccess);
        }
        else{
            bSuccess = false; // to exit while
        }
        //----------------------------------
    }
    //fout<<"workerLoaderFinam out\n";
}


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

//            std::tm tmT;
//            tmT.tm_year = 2020  - 1900;
//            tmT.tm_mon  = 03 - 1;
//            tmT.tm_mday = 1;
//            tmT.tm_hour = 0;
//            tmT.tm_min = 0;
//            tmT.tm_sec = 0;
//            tmT.tm_isdst = 0;
//            std::time_t t = std::mktime(&tmT);

//            std::stringstream ssOut;
//            stStore.CreateAndGetFileStageForTicker(data.TickerID, t, ssOut);

//            {
//                dataBuckgroundThreadAnswer dt (dataBuckgroundThreadAnswer::eAnswerType::TextInfoMessage,data.GetParentWnd());
//                dt.SetTextInfo(ssOut.str());
//                queueTrdAnswers.Push(dt);
//            }

//            ssOut.str("");
//            ssOut.clear();

//            std::tm tmT1;
//            tmT1.tm_year = 2021  - 1900;
//            tmT1.tm_mon  = 02 - 1;
//            tmT1.tm_mday = 15;
//            tmT1.tm_hour = 12;
//            tmT1.tm_min = 46;
//            tmT1.tm_sec = 33;
//            tmT1.tm_isdst = 0;
//            std::time_t t1 = std::mktime(&tmT1);

//            Bar b(1,2,3,4,100,t1);
//            if (stStore.WriteBarToStore(data.TickerID, b, ssOut)){
//                ssOut<<"write successful.\n";
//            }
//            else{
//                ssOut<<"write fail.\n";
//            }

//            {
//                dataBuckgroundThreadAnswer dt (dataBuckgroundThreadAnswer::eAnswerType::TextInfoMessage,data.GetParentWnd());
//                dt.SetTextInfo(ssOut.str());
//                queueTrdAnswers.Push(dt);
//            }
