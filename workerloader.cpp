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

        queueTrdAnswers.Push({data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famLoadBegin,data.GetParentWnd()});

        ////////////////////////////////////////////////////////////
        dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::TextInfoMessage,data.GetParentWnd());
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
                dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famLoadEnd,data.GetParentWnd());
                dt.SetSuccessfull(false);
                dt.SetErrString("loading process interrupted");
                queueTrdAnswers.Push(dt);
                bWasBreaked = true;
                break;
            }
            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //// work area
            if (i %10 == 0){
                dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famLoadCurrent,data.GetParentWnd());
                dt.SetPercent(i/10);
                queueTrdAnswers.Push(dt);
            }
            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::LoadActivity,data.GetParentWnd());
            ///
            std::this_thread::sleep_for(4ms);

        }
        if (!bWasBreaked){
            dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famLoadEnd,data.GetParentWnd());
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

/////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Main thread - dispatcher for database work
/// \param queueFilLoad
/// \param queueTrdAnswers
/// \param stStore
void workerLoader::workerDataBaseWork(BlockFreeQueue<dataFinLoadTask> & queueTasks,
                                      BlockFreeQueue<dataBuckgroundThreadAnswer> &queueTrdAnswers,
                                       Storage & stStore)
{
    bool bSuccess{false};
    auto pdata = queueTasks.Pop(bSuccess);
    while(bSuccess){

        dataFinLoadTask data(*pdata.get());
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// one task start

        switch (data.taskType) {
        case dataFinLoadTask::TaskType::finQuotesImport:
            workerFinQuotesLoad(queueTasks,queueTrdAnswers,stStore,data);
            break;
        case dataFinLoadTask::TaskType::finQuotesLoadFromStorage:
            {
            ThreadFreeCout pcout;
            pcout << "load from base task for TickerID: " << data.TickerID<<"\n";
            }
            break;
        case dataFinLoadTask::TaskType::storageOptimisation:
            {
            ThreadFreeCout pcout;
            pcout << "data optimisaion task for TickerID: " << data.TickerID<<"\n";
            }
            break;
        default:
            break;
        }
        /// one task end
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        if (!this_thread_flagInterrup.isSet()){
            pdata = queueTasks.Pop(bSuccess);
        }
        else{
            bSuccess = false; // to exit while
        }
        //----------------------------------
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Thread task for upload fin quotes data from file
/// \param queueFilLoad
/// \param queueTrdAnswers
/// \param stStore
///
void workerLoader::workerFinQuotesLoad(BlockFreeQueue<dataFinLoadTask> & queueTasks,
                                BlockFreeQueue<dataBuckgroundThreadAnswer> &queueTrdAnswers,
                                Storage & stStore,
                                dataFinLoadTask & data)

{

    {
        dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famLoadBegin,data.GetParentWnd());
        queueTrdAnswers.Push(dt);
    }

    std::chrono::time_point dtStart(std::chrono::steady_clock::now());
    std::chrono::time_point dtActivity(std::chrono::steady_clock::now());
    milliseconds tActivityCount = std::chrono::steady_clock::now() - dtActivity;
    bool bWasSuccessfull{false};

    ////////////////////////////////////////////////////////////
    dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::TextInfoMessage,data.GetParentWnd());
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
        dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famLoadEnd,data.GetParentWnd());
        dt.SetSuccessfull(false);
        ssErr<<"\ncannot initialize stock quotes storage for ticker";
        dt.SetErrString(ssErr.str());
        queueTrdAnswers.Push(dt);
    }
    else{
        if (ssErr.str().size()>0){
            dataBuckgroundThreadAnswer dtT (data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::TextInfoMessage,data.GetParentWnd());
            dtT.SetTextInfo(ssErr.str());
            queueTrdAnswers.Push(dtT);
        }
        ///////////////////////////////////////////////////////////////////////////

        std::istringstream issTmp;
        std::ostringstream ossErr;
        Bar bb(0,0,0,0,0,0);
        BarMemcopier bM(bb);

        //const int iOutBuffMax {1024};
        //const int iOutBuffMax {32768};
        const int iOutBuffMax {524288};
        //const int iOutBuffMax {2097152};
        //const int iOutBuffMax {3145727};

        char cOutBuff[iOutBuffMax];
        int iOutBuffPointer {0};
        //int iSecChangedPointer {0};
        int iBlockSize (  sizeof (Storage::data_type)
                        + sizeof (bb.Open())
                        + sizeof (bb.High())
                        + sizeof (bb.Low())
                        + sizeof (bb.Close())
                        + sizeof (bb.Volume())
                        + sizeof (bb.Period())
                        );

        std::time_t tMonth{0};
        std::time_t tSec{0};
        std::time_t tCurrentMonth{0};
        std::time_t tCurrentSec{0};

        Storage::data_type iState{Storage::data_type::new_sec};

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
                    dataBuckgroundThreadAnswer dt (data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::TextInfoMessage,data.GetParentWnd());
                    dt.SetTextInfo(ss.str());
                    queueTrdAnswers.Push(dt);
                }
                /////////////////////////////

//                    std::vector<char> vBigBuff(filesize+1);
//                    fileR.read(vBigBuff.data(), filesize);
//                    std::istringstream file(vBigBuff.data());

                /////////////////////////////
                std::string sBuff;
                std::istringstream iss;


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
                        dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famLoadEnd,data.GetParentWnd());
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
                    tMonth  = Storage::dateCastToMonth(bb.Period());
                    tSec    = bb.Period();

                    /// init new storage file or buffer is full
                    if (tMonth !=  tCurrentMonth){
                        if (iOutBuffPointer >0){
                            // do write
                            if(!stStore.WriteMemblockToStore(data.TickerID, tMonth, cOutBuff,iOutBuffPointer, ssErr)){
                                dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famLoadEnd,data.GetParentWnd());
                                dt.SetSuccessfull(false);
                                ssErr<<"\n\rError writing file.\n";
                                dt.SetErrString(ssErr.str());
                                queueTrdAnswers.Push(dt);
                                bWasSuccessfull = false;
                                break;
                            }
                        }
                        tCurrentMonth   = tMonth;
                        iOutBuffPointer = 0;
                        //iSecChangedPointer = 0;
                    }
                    else if( iOutBuffPointer + iBlockSize*4 > iOutBuffMax){ // do write to iSecChangedPointer

//                        if (iSecChangedPointer == 0){
//                            // if everything is within one second, copy the entire block
//                            iSecChangedPointer = iOutBuffPointer;
//                            // TODO: lock database file to correct continue
//                        }
                        if(!stStore.WriteMemblockToStore(data.TickerID, tMonth, cOutBuff,iOutBuffPointer/*iSecChangedPointer*/, ssErr)){
                            dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famLoadEnd,data.GetParentWnd());
                            dt.SetSuccessfull(false);
                            ssErr<<"\n\rError writing file.\n";
                            dt.SetErrString(ssErr.str());
                            queueTrdAnswers.Push(dt);
                            bWasSuccessfull = false;
                            break;
                        }

                        iOutBuffPointer = 0;
                        //memcpy(cOutBuff, cOutBuff + iSecChangedPointer,   iOutBuffPointer - iSecChangedPointer);
                        //iOutBuffPointer = iOutBuffPointer - iSecChangedPointer;
                        //iSecChangedPointer = 0;
                    }
                    //
                    if (tSec != tCurrentSec){
                        iState = Storage::data_type::new_sec;
                        tCurrentSec = tSec;
                        //iSecChangedPointer = iOutBuffPointer;
                    }
                    else{
                        iState = Storage::data_type::usual;
                    }

                    memcpy(cOutBuff + iOutBuffPointer,&iState,   sizeof (Storage::data_type));      iOutBuffPointer += sizeof (Storage::data_type);
                    memcpy(cOutBuff + iOutBuffPointer,&bM.Open(),   sizeof (bM.Open()));            iOutBuffPointer += sizeof (bM.Open());
                    memcpy(cOutBuff + iOutBuffPointer,&bM.High(),   sizeof (bM.High()));            iOutBuffPointer += sizeof (bM.High());
                    memcpy(cOutBuff + iOutBuffPointer,&bM.Low(),    sizeof (bM.Low()));             iOutBuffPointer += sizeof (bM.Low());
                    memcpy(cOutBuff + iOutBuffPointer,&bM.Close(),  sizeof (bM.Close()));           iOutBuffPointer += sizeof (bM.Close());
                    memcpy(cOutBuff + iOutBuffPointer,&bM.Volume(), sizeof (bM.Volume()));          iOutBuffPointer += sizeof (bM.Volume());
                    memcpy(cOutBuff + iOutBuffPointer,&bM.Period(), sizeof (bM.Period()));          iOutBuffPointer += sizeof (bM.Period());

                    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
                    //// send completion percentage
                    ///
                    currsize = file.tellg();
                    iCurrProgress = int(100*(currsize/(double)filesize));
                    if(iProgressSent < iCurrProgress){
                        dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famLoadCurrent,data.GetParentWnd());
                        dt.SetPercent(iCurrProgress);
                        queueTrdAnswers.Push(dt);
                    }
                    //// send every 0.5 sec activity
                    ///
                    tActivityCount = std::chrono::steady_clock::now() - dtActivity;
                    if (tActivityCount > 500ms){
                        dtActivity = std::chrono::steady_clock::now();
                        dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::LoadActivity,data.GetParentWnd());
                        queueTrdAnswers.Push(dt);
                    }
                    //// check thread interrupt
                    ///
                    if(this_thread_flagInterrup.isSet()){
                        //fout<<"exit on interrupt\n";
                        dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famLoadEnd,data.GetParentWnd());
                        dt.SetSuccessfull(false);
                        dt.SetErrString("loading process interrupted");
                        queueTrdAnswers.Push(dt);
                        //bWasBreaked = true;
                        bWasSuccessfull = false;
                        break;
                    }
                }
                if (bWasSuccessfull && iOutBuffPointer>0){
                    ////////////////////////
                    /// write tail
                    if(!stStore.WriteMemblockToStore(data.TickerID, tMonth, cOutBuff,iOutBuffPointer, ssErr)){
                        dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famLoadEnd,data.GetParentWnd());
                        dt.SetSuccessfull(false);
                        ssErr<<"\n\rError writing file.\n";
                        dt.SetErrString(ssErr.str());
                        queueTrdAnswers.Push(dt);
                        bWasSuccessfull = false;
                    }
                    /// ////////////////////////
                }
            }
        }
        /////
        if (!bFileOpened){
            dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famLoadEnd,data.GetParentWnd());
            dt.SetSuccessfull(false);
            ssErr<<"\n\rcannot open file :"<<data.pathFileName<<"\n";
            dt.SetErrString(ssErr.str());
            queueTrdAnswers.Push(dt);
        }

        ////////////////////////////////////////////////////////////////////////
        if(bWasSuccessfull){
            std::chrono::time_point dtStop(std::chrono::steady_clock::now());
            //milliseconds tCount = dtStop - dtStart;
            seconds tCount = dtStop - dtStart;
            {
                std::stringstream ss;
                //ss << "Load time: "<<tCount.count()<<" milliseconds\n";
                ss << "Load time: "<<tCount.count()<<" seconds\n";
                dataBuckgroundThreadAnswer dt (data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::TextInfoMessage,data.GetParentWnd());
                dt.SetTextInfo(ss.str());
                queueTrdAnswers.Push(dt);
            }

            dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famLoadEnd,data.GetParentWnd());
            dt.SetSuccessfull(true);
            queueTrdAnswers.Push(dt);

            ///////
            //  adding task for data optimisation:
            dataFinLoadTask optTask(data);// copying data range and tickerID
            optTask.taskType = dataFinLoadTask::TaskType::storageOptimisation;
            queueTasks.Push(optTask);
        }
    }

}

////////////////////////////////////////////////////////////////////////
//  9. добавить очередь задач на оптимизацию данных
