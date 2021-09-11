#include "workerloader.h"
#include "threadfreecout.h"
#include "threadpool.h"

#include<sstream>
#include<fstream>
#include<ostream>
#include<chrono>

#include "graph.h"
#include "threadfreelocaltime.h"


using namespace std::chrono_literals;
using seconds=std::chrono::duration<double>;
using milliseconds=std::chrono::duration<double,
    std::ratio_multiply<seconds::period,std::milli>
    >;

//------------------------------------------------------------------------------------------------------------------------------------------
workerLoader::workerLoader()
{

}

//------------------------------------------------------------------------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Main thread - dispatcher for database work
/// \param queueFilLoad
/// \param queueTrdAnswers
/// \param stStore
void workerLoader::workerDataBaseWork(BlockFreeQueue<dataFinLoadTask> & queueTasks,
                                      BlockFreeQueue<dataBuckgroundThreadAnswer> &queueTrdAnswers,
                                       Storage & stStore)
{

//    int iID = WorkerThreadCounter.load();
//    while (!WorkerThreadCounter.compare_exchange_weak(iID,iID + 1)) {;}
//    WorkerThreadID = iID;

    try{
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
            case dataFinLoadTask::TaskType::finQuotesCheck:
                workerFinQuotesCheck(queueTasks,queueTrdAnswers,stStore,data);
                break;
            case dataFinLoadTask::TaskType::finQuotesLoadFromStorage:
                workerLoadFromStorage(queueTasks,queueTrdAnswers,stStore,data);
                break;
            case dataFinLoadTask::TaskType::LoadIntoGraph:
                workerLoadIntoGraph(queueTasks,queueTrdAnswers,stStore,data);
                break;
            case dataFinLoadTask::TaskType::storageOptimisation:
                workerOptimizeStorage(queueTasks,queueTrdAnswers,stStore,data);
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
    catch (std::exception &ex) {
        ThreadFreeCout pcout;
        pcout <<"crash ecxeption" <<ex.what()<<"\n";
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
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

        queueTrdAnswers.Push({data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famImportBegin,data.GetParentWnd()});

        ////////////////////////////////////////////////////////////
        dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::TextInfoMessage,data.GetParentWnd());
        std::stringstream ss;
        char buffer[100];
        std::tm * ptmB = threadfree_gmtime(&data.dtBegin);
        std::strftime(buffer, 100, "%Y/%m/%d %H:%M:%S", ptmB);
        std::string strB(buffer);

        std::tm * ptmE = threadfree_gmtime(&data.dtEnd);
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
                dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famImportEnd,data.GetParentWnd());
                dt.SetSuccessfull(false);
                dt.SetErrString("loading process interrupted");
                queueTrdAnswers.Push(dt);
                bWasBreaked = true;
                break;
            }
            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //// work area
            if (i %10 == 0){
                dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famImportCurrent,data.GetParentWnd());
                dt.SetPercent(i/10);
                queueTrdAnswers.Push(dt);
            }
            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::LoadActivity,data.GetParentWnd());
            ///
            std::this_thread::sleep_for(4ms);

        }
        if (!bWasBreaked){
            dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famImportEnd,data.GetParentWnd());
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


//------------------------------------------------------------------------------------------------------------------------------------------
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
        dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famImportBegin,data.GetParentWnd());
        queueTrdAnswers.Push(dt);
    }
    {
        dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famImportCurrent,data.GetParentWnd());
        dt.SetPercent(0);
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
    std::tm * ptmB = threadfree_gmtime(&data.dtBegin);
    std::strftime(buffer, 100, "%Y/%m/%d %H:%M:%S", ptmB);
    std::string strB(buffer);

    std::tm * ptmE = threadfree_gmtime(&data.dtEnd);
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
    if (Bar::castInterval(data.iInterval)  != Bar::eInterval::pTick){
        dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famImportEnd,data.GetParentWnd());
        dt.SetSuccessfull(false);
        ssErr<<"\nincorrect interval: {"<<data.iInterval<<"}\n";
        ssErr<<"only tick data can be loaded\n";
        dt.SetErrString(ssErr.str());
        queueTrdAnswers.Push(dt);
    }
    else if (!stStore.InitializeTicker(data.TickerID,ssErr)){
        dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famImportEnd,data.GetParentWnd());
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
        const int iOutBuffMax {65535};
        //const int iOutBuffMax {524288};
        //const int iOutBuffMax {2097152};
        //const int iOutBuffMax {3145727};

        char cOutBuff[iOutBuffMax];
        int iOutBuffPointer {0};
        //int iSecChangedPointer {0};
        int iBlockSize (  sizeof (Storage::data_type)
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
                size_t currsize{0};
                int iProgressSent{0};
                int iCurrProgress{0};
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

                Storage::MutexDefender<std::shared_lock<std::shared_mutex>> defSlk;
                Storage::MutexDefender<std::unique_lock<std::shared_mutex>> defUlk;

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
                        dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famImportEnd,data.GetParentWnd());
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
                            if(!stStore.WriteMemblockToStore(defSlk,defUlk,data.TickerID, tCurrentMonth, cOutBuff,iOutBuffPointer, ssErr)){
                                dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famImportEnd,data.GetParentWnd());
                                dt.SetSuccessfull(false);
                                ssErr<<"\n\rError writing file.\n";
                                dt.SetErrString(ssErr.str());
                                queueTrdAnswers.Push(dt);
                                bWasSuccessfull = false;
                                break;
                            }
                        }
                        tCurrentMonth   = tMonth;
                        iOutBuffPointer = createCleanUpHeader(tCurrentMonth, cOutBuff,data.dtBegin, data.dtEnd);
                    }
                    else if( iOutBuffPointer + iBlockSize*4 > iOutBuffMax){ // do write to iSecChangedPointer

                        if(!stStore.WriteMemblockToStore(defSlk,defUlk,data.TickerID, tMonth, cOutBuff,iOutBuffPointer/*iSecChangedPointer*/, ssErr)){
                            dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famImportEnd,data.GetParentWnd());
                            dt.SetSuccessfull(false);
                            ssErr<<"\n\rError writing file.\n";
                            dt.SetErrString(ssErr.str());
                            queueTrdAnswers.Push(dt);
                            bWasSuccessfull = false;
                            break;
                        }

                        iOutBuffPointer = 0;
                    }
                    //
                    if (tSec != tCurrentSec){
                        iState = Storage::data_type::new_sec;
                        tCurrentSec = tSec;
                    }
                    else{
                        iState = Storage::data_type::usual;
                    }

//                    if (Market::IsInSessionTabe(data.vSessionTable,bb.Period()) ||
//                        Market::IsInSessionTabe(data.vRepoTable,bb.Period())
//                            )
                    {
                        memcpy(cOutBuff + iOutBuffPointer,&iState,   sizeof (Storage::data_type));      iOutBuffPointer += sizeof (Storage::data_type);
                        memcpy(cOutBuff + iOutBuffPointer,&bM.Close(),  sizeof (bM.Close()));           iOutBuffPointer += sizeof (bM.Close());
                        memcpy(cOutBuff + iOutBuffPointer,&bM.Volume(), sizeof (bM.Volume()));          iOutBuffPointer += sizeof (bM.Volume());
                        memcpy(cOutBuff + iOutBuffPointer,&bM.Period(), sizeof (bM.Period()));          iOutBuffPointer += sizeof (bM.Period());
                    }

                    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
                    //// send completion percentage
                    ///
                    currsize = file.tellg();
                    iCurrProgress = int(100*(currsize/(double)filesize));
                    if(iProgressSent < iCurrProgress){
                        dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famImportCurrent,data.GetParentWnd());
                        dt.SetPercent(iCurrProgress);
                        queueTrdAnswers.Push(dt);
                        iProgressSent = iCurrProgress;
                    }
                    //// send every 0.5 sec activity
                    ///
                    tActivityCount = std::chrono::steady_clock::now() - dtActivity;
                    if (tActivityCount > 500ms){
                        dtActivity = std::chrono::steady_clock::now();
                        dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::LoadActivity,data.GetParentWnd());
                        queueTrdAnswers.Push(dt);
                    }

                    //}
                    //// check thread interrupt
                    ///
                    if(this_thread_flagInterrup.isSet()){
                        //fout<<"exit on interrupt\n";
                        dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famImportEnd,data.GetParentWnd());
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
                    if(!stStore.WriteMemblockToStore(defSlk,defUlk,data.TickerID, tMonth, cOutBuff,iOutBuffPointer, ssErr)){
                        dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famImportEnd,data.GetParentWnd());
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
            dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famImportEnd,data.GetParentWnd());
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
                ss << "Import time: "<<tCount.count()<<" seconds\n";
                dataBuckgroundThreadAnswer dt (data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::TextInfoMessage,data.GetParentWnd());
                dt.SetTextInfo(ss.str());
                queueTrdAnswers.Push(dt);
            }

            dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famImportEnd,data.GetParentWnd());
            dt.SetSuccessfull(true);
            queueTrdAnswers.Push(dt);


            ///////
            //  adding load data task for viewers:
            dataFinLoadTask loadTask(data);// copying data range and tickerID
            loadTask.taskType = dataFinLoadTask::TaskType::finQuotesLoadFromStorage;
            queueTasks.Push(loadTask);
            ///////
            //  adding task for data optimisation:
            dataFinLoadTask optTask(data);// copying data range and tickerID
            optTask.taskType = dataFinLoadTask::TaskType::storageOptimisation;
            queueTasks.Push(optTask);
            //
            {
                dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::cloneThread,data.GetParentWnd());
                queueTrdAnswers.Push(dt);
            }
            {
                dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::cloneThread,data.GetParentWnd());
                queueTrdAnswers.Push(dt);
            }
        }
    }

}
//------------------------------------------------------------------------------------------------------------------------------------------
int workerLoader::createCleanUpHeader(std::time_t tMonth, char* cBuff,std::time_t tBegin, std::time_t tEnd)
{
    if (tBegin > tEnd) return 0;
    if (tBegin < tMonth){
        tBegin = tMonth;
    }
    if (tEnd >= Storage::dateAddMonth(tMonth)){
        tEnd = Storage::dateAddMonth(tMonth)-1;
    }


    Bar bb(0,0,0,0,0,0);
    BarMemcopier bM(bb);


    int iBuffPointer{0};
    Storage::data_type iState;
    iState = Storage::data_type::del_from;

    bb.setPeriod(tBegin);

    memcpy(cBuff + iBuffPointer,&iState,   sizeof (Storage::data_type));      iBuffPointer += sizeof (Storage::data_type);
//    memcpy(cBuff + iBuffPointer,&bM.Open(),   sizeof (bM.Open()));            iBuffPointer += sizeof (bM.Open());
//    memcpy(cBuff + iBuffPointer,&bM.High(),   sizeof (bM.High()));            iBuffPointer += sizeof (bM.High());
//    memcpy(cBuff + iBuffPointer,&bM.Low(),    sizeof (bM.Low()));             iBuffPointer += sizeof (bM.Low());
    memcpy(cBuff + iBuffPointer,&bM.Close(),  sizeof (bM.Close()));           iBuffPointer += sizeof (bM.Close());
    memcpy(cBuff + iBuffPointer,&bM.Volume(), sizeof (bM.Volume()));          iBuffPointer += sizeof (bM.Volume());
    memcpy(cBuff + iBuffPointer,&bM.Period(), sizeof (bM.Period()));          iBuffPointer += sizeof (bM.Period());

    iState = Storage::data_type::del_to;
    bb.setPeriod(tEnd);

    memcpy(cBuff + iBuffPointer,&iState,   sizeof (Storage::data_type));      iBuffPointer += sizeof (Storage::data_type);
//    memcpy(cBuff + iBuffPointer,&bM.Open(),   sizeof (bM.Open()));            iBuffPointer += sizeof (bM.Open());
//    memcpy(cBuff + iBuffPointer,&bM.High(),   sizeof (bM.High()));            iBuffPointer += sizeof (bM.High());
//    memcpy(cBuff + iBuffPointer,&bM.Low(),    sizeof (bM.Low()));             iBuffPointer += sizeof (bM.Low());
    memcpy(cBuff + iBuffPointer,&bM.Close(),  sizeof (bM.Close()));           iBuffPointer += sizeof (bM.Close());
    memcpy(cBuff + iBuffPointer,&bM.Volume(), sizeof (bM.Volume()));          iBuffPointer += sizeof (bM.Volume());
    memcpy(cBuff + iBuffPointer,&bM.Period(), sizeof (bM.Period()));          iBuffPointer += sizeof (bM.Period());



    return iBuffPointer;
}
//------------------------------------------------------------------------------------------------------------------------------------------
void workerLoader::workerLoadFromStorage(BlockFreeQueue<dataFinLoadTask> & queueTasks,
                                BlockFreeQueue<dataBuckgroundThreadAnswer> &queueTrdAnswers,
                                Storage & stStore,
                                dataFinLoadTask & data)

{
//                if (data.TickerID == 1){
//                ThreadFreeCout pcout;
//                pcout << "load from base task for TickerID: " << data.TickerID<<"\n";
//                }
    {
        dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::storagLoadFromStorageGraphBegin,data.GetParentWnd());
        queueTrdAnswers.Push(dt);
    }
    //
    std::chrono::time_point dtStart(std::chrono::steady_clock::now());
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool bSuccessfull{true};

    std::stringstream ssErr;
    if (!stStore.InitializeTicker(data.TickerID,ssErr)){
        dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famImportEnd,data.GetParentWnd());
        dt.SetSuccessfull(false);
        ssErr<<"\ncannot initialize stock quotes storage for ticker";
        dt.SetErrString(ssErr.str());
        queueTrdAnswers.Push(dt);
    }
    else{

        std::vector<std::time_t> vMonthToLoad;

        std::time_t dtMonthBegin = Storage::dateCastToMonth(data.dtBegin);
        std::time_t dtMonthEnd   = Storage::dateAddMonth(data.dtEnd);

        while (dtMonthBegin < dtMonthEnd) {
            vMonthToLoad.push_back(dtMonthBegin);
            dtMonthBegin = Storage::dateAddMonth(dtMonthBegin);
        }
        //
        std::stringstream ssOut;

        std::shared_ptr<std::vector<std::vector<BarTick>>> pvBars{std::make_shared<std::vector<std::vector<BarTick>>>()};

        for (const std::time_t &t:vMonthToLoad){
            ssOut.str("");
            ssOut.clear();
            //
            pvBars->push_back({});
            //
            if (!stStore.ReadFromStore(data.TickerID, t, pvBars->back(), data.dtBegin,data.dtEnd,true,data.vRepoTable,false,data.vSessionTable,ssOut)){
                bSuccessfull = false;
                dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::storagLoadFromStorageGraphEnd,data.GetParentWnd());
                dt.SetSuccessfull(false);
                ssOut<<"\n\rError reading file.\n";
                dt.SetErrString(ssOut.str());
                queueTrdAnswers.Push(dt);

                break;
            }
            else{
                if (ssOut.str().size()>0){
                    dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::logText,data.GetParentWnd());
                    dt.SetTextInfo(ssOut.str());
                    queueTrdAnswers.Push(dt);
                }
            }
            if(this_thread_flagInterrup.isSet()){
                //fout<<"exit on interrupt\n";
                dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::storagLoadFromStorageGraphEnd,data.GetParentWnd());
                dt.SetSuccessfull(false);
                dt.SetErrString("loading process interrupted");
                queueTrdAnswers.Push(dt);
                bSuccessfull = false;
                break;
            }
        }
        /////
        {
            // TODO: remove
            //for tests
//            if (data.TickerID == 9){
//                dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::testPvBars,data.GetParentWnd());
//                dt.pvBars = pvBars;
//                queueTrdAnswers.Push(dt);
//            }
        }
        dataFinLoadTask optTask(data);// copying data range and tickerID
        optTask.taskType = dataFinLoadTask::TaskType::LoadIntoGraph;
        optTask.pvBars =pvBars;
        queueTasks.Push(optTask);
        {
            dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::cloneThread,data.GetParentWnd());
            queueTrdAnswers.Push(dt);
        }
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if (bSuccessfull){
        std::chrono::time_point dtStop(std::chrono::steady_clock::now());
        //milliseconds tCount = dtStop - dtStart;
        seconds tCount = dtStop - dtStart;
        {
            std::stringstream ss;
            ss <<"{"<< data.TickerID<<"} ";
            ss << "Import time: "<<tCount.count()<<" seconds\n";
            dataBuckgroundThreadAnswer dt (data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::logText,data.GetParentWnd());
            dt.SetTextInfo(ss.str());
            queueTrdAnswers.Push(dt);
        }
        //
        dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::storagLoadFromStorageGraphEnd,data.GetParentWnd());
        dt.SetSuccessfull(true);
        queueTrdAnswers.Push(dt);
    }

//    if (data.TickerID == 1){
//    ThreadFreeCout pcout;
//    pcout << "out load from base {" << data.TickerID<<"}\n";
//    }

}
//------------------------------------------------------------------------------------------------------------------------------------------
void workerLoader::workerLoadIntoGraph(BlockFreeQueue<dataFinLoadTask> & /*queueTasks*/,
                                BlockFreeQueue<dataBuckgroundThreadAnswer> &queueTrdAnswers,
                                Storage & /*stStore*/,
                                dataFinLoadTask & data)
{
//    if (data.TickerID == 1){
//        ThreadFreeCout pcout;
//        pcout << "in load into Graph for TickerID: " << data.TickerID<<"\n";
//    }
    {
        dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::storagLoadToGraphBegin,data.GetParentWnd());
        queueTrdAnswers.Push(dt);
    }
    std::chrono::time_point dtStart(std::chrono::steady_clock::now());
    bool bSuccessfull{true};
    //////////////////////////////////////////////////////////////////////////////////
    if (data.holder != std::shared_ptr<GraphHolder>{}){

        if(std::shared_ptr<std::vector<std::vector<BarTick>>>{} != data.pvBars){

            if(data.holder->AddBarsLists(*data.pvBars.get(),data.dtBegin,data.dtEnd)){
                //                if (data.holder->CheckMap()){
                //                    dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::logText,data.GetParentWnd());
                //                    dt.SetTextInfo("map consistensy is good");
                //                    queueTrdAnswers.Push(dt);
                //                }
                //                else{
                //                    dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::logCriticalError,data.GetParentWnd());
                //                    dt.SetErrString("map consistensy is broken");
                //                    queueTrdAnswers.Push(dt);
                //                    bSuccessfull = false;
                //                }
            }
            if(this_thread_flagInterrup.isSet()){
                //fout<<"exit on interrupt\n";
                dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::storagLoadToGraphEnd,data.GetParentWnd());
                dt.SetSuccessfull(false);
                dt.SetErrString("loading process interrupted");
                queueTrdAnswers.Push(dt);
                bSuccessfull = false;
            }
        }
        else{
            dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::storagLoadToGraphEnd,data.GetParentWnd());
            dt.SetErrString("Empty incoming vVector<...>");
            dt.SetSuccessfull(false);
            queueTrdAnswers.Push(dt);
            bSuccessfull = false;
        }
    }
    else{
        dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::storagLoadToGraphEnd,data.GetParentWnd());
        dt.SetErrString("Empty graph holder ptr");
        dt.SetSuccessfull(false);
        queueTrdAnswers.Push(dt);
        bSuccessfull = false;
    }
    //////////////////////////////////////////////////////////////////////////////////
    if (bSuccessfull){
        std::chrono::time_point dtStop(std::chrono::steady_clock::now());
        //milliseconds tCount = dtStop - dtStart;
        seconds tCount = dtStop - dtStart;
        {
            std::stringstream ss;
            ss <<"{"<< data.TickerID<<"} ";
            ss << "Graph filling time: "<<tCount.count()<<" seconds\n";
            dataBuckgroundThreadAnswer dt (data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::logText,data.GetParentWnd());
            dt.SetTextInfo(ss.str());
            queueTrdAnswers.Push(dt);
        }
        //
        dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::storagLoadToGraphEnd,data.GetParentWnd());
        dt.SetBeginDate(data.dtBegin);
        dt.SetEndDate(data.dtEnd);
        dt.SetSuccessfull(true);
        queueTrdAnswers.Push(dt);
    }
//    if (data.TickerID == 1){
//        ThreadFreeCout pcout;
//        pcout << "out load into Graph {" << data.TickerID<<"}\n";
//    }
}
//------------------------------------------------------------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////
void workerLoader::workerOptimizeStorage(BlockFreeQueue<dataFinLoadTask> & queueTasks,
                                BlockFreeQueue<dataBuckgroundThreadAnswer> &queueTrdAnswers,
                                Storage & stStore,
                                dataFinLoadTask & data)
{
        {

            ThreadFreeCout pcout;
            pcout << "storage optimization task for TickerID: " << data.TickerID<<"\n";
        }
        {
            dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::storageOptimisationBegin,data.GetParentWnd());
            queueTrdAnswers.Push(dt);
        }
        //
        std::chrono::time_point dtStart(std::chrono::steady_clock::now());
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        bool bSuccessfull{true};

        std::stringstream ssErr;
        if (!stStore.InitializeTicker(data.TickerID,ssErr)){
            dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famImportEnd,data.GetParentWnd());
            dt.SetSuccessfull(false);
            ssErr<<"\ncannot initialize stock quotes storage for ticker";
            dt.SetErrString(ssErr.str());
            queueTrdAnswers.Push(dt);
        }
        else{
            std::vector<std::time_t> vMonthToLoad;

            std::time_t dtMonthBegin = Storage::dateCastToMonth(data.dtBegin);
            std::time_t dtMonthEnd   = Storage::dateAddMonth(data.dtEnd);

            while (dtMonthBegin < dtMonthEnd) {
                vMonthToLoad.push_back(dtMonthBegin);
                dtMonthBegin = Storage::dateAddMonth(dtMonthBegin);
            }
            //
            std::stringstream ssOut;

            bool bToPlanNextShift{false};

            for (const std::time_t &t:vMonthToLoad){
                ssOut.str("");
                ssOut.clear();
                bToPlanNextShift = false;
                //

                if (!stStore.OptimizeStore( data.TickerID, t, bToPlanNextShift,ssOut))
                {
                    bSuccessfull = false;
                    dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::storageOptimisationEnd,data.GetParentWnd());
                    dt.SetSuccessfull(false);
                    ssOut<<"\n\rError optimizing file.\n";
                    dt.SetErrString(ssOut.str());
                    queueTrdAnswers.Push(dt);
                    break;
                }
                else{
                    if (bToPlanNextShift){// plan next optimization task
                        dataFinLoadTask optTask(data);// copying data range and tickerID
                        optTask.dtBegin = t;    // only current month
                        optTask.dtEnd   = t;    // only current month
                        optTask.taskType = dataFinLoadTask::TaskType::storageOptimisation;
                        queueTasks.Push(optTask);
                    }
                    if (ssOut.str().size()>0){
                        dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::logText,data.GetParentWnd());
                        dt.SetTextInfo(ssOut.str());
                        queueTrdAnswers.Push(dt);
                    }
                }
                if(this_thread_flagInterrup.isSet()){
                    //fout<<"exit on interrupt\n";
                    dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::storageOptimisationEnd,data.GetParentWnd());
                    dt.SetSuccessfull(false);
                    dt.SetErrString("loading process interrupted");
                    queueTrdAnswers.Push(dt);
                    bSuccessfull = false;
                    break;
                }
            }
        }
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        if (bSuccessfull){
            std::chrono::time_point dtStop(std::chrono::steady_clock::now());
            //milliseconds tCount = dtStop - dtStart;
            seconds tCount = dtStop - dtStart;
            {
                std::stringstream ss;
                ss << "Optimization time: "<<tCount.count()<<" seconds\n";
                dataBuckgroundThreadAnswer dt (data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::logText,data.GetParentWnd());
                dt.SetTextInfo(ss.str());
                queueTrdAnswers.Push(dt);
            }
            //
            dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::storageOptimisationEnd,data.GetParentWnd());
            dt.SetSuccessfull(true);
            queueTrdAnswers.Push(dt);
        }

//        {

//            ThreadFreeCout pcout;
//            pcout << "storage optimization task out {" << data.TickerID<<"}\n";
//        }
}
//------------------------------------------------------------------------------------------------------------------------------------------
/// \brief Thread task for check fin quotes data from file
/// \param queueFilLoad
/// \param queueTrdAnswers
/// \param stStore
///
void workerLoader::workerFinQuotesCheck(BlockFreeQueue<dataFinLoadTask> & /*queueTasks*/,
                                BlockFreeQueue<dataBuckgroundThreadAnswer> &queueTrdAnswers,
                                Storage & stStore,
                                dataFinLoadTask & data)

{

    {
        dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famImportBegin,data.GetParentWnd());
        queueTrdAnswers.Push(dt);
    }
    {
        dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famImportCurrent,data.GetParentWnd());
        dt.SetPercent(0);
        queueTrdAnswers.Push(dt);
    }

    std::chrono::time_point dtStart(std::chrono::steady_clock::now());
    std::chrono::time_point dtActivity(std::chrono::steady_clock::now());
    milliseconds tActivityCount = std::chrono::steady_clock::now() - dtActivity;
    bool bWasSuccessfull{false};
    bool bIsEqual{true};

    ////////////////////////////////////////////////////////////
    dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::TextInfoMessage,data.GetParentWnd());
    std::stringstream ss;
    char buffer[100];
    std::tm * ptmB = threadfree_gmtime(&data.dtBegin);
    std::strftime(buffer, 100, "%Y/%m/%d %H:%M:%S", ptmB);
    std::string strB(buffer);

    std::tm * ptmE = threadfree_gmtime(&data.dtEnd);
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
        dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famImportEnd,data.GetParentWnd());
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
        std::vector<Bar> vBars;

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
                size_t currsize{0};
                int iProgressSent{0};
                int iCurrProgress{0};
                file.seekg(0, std::ios::beg);
                {
                    std::stringstream ss;
                    ss << "filesize: " << filesize<<"\n";
                    dataBuckgroundThreadAnswer dt (data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::TextInfoMessage,data.GetParentWnd());
                    dt.SetTextInfo(ss.str());
                    queueTrdAnswers.Push(dt);
                }
                /////////////////////////////

                /////////////////////////////
                std::string sBuff;
                std::istringstream iss;

//                Storage::MutexDefender<std::shared_lock<std::shared_mutex>> defSlk;
//                Storage::MutexDefender<std::unique_lock<std::shared_mutex>> defUlk;

                {
                    dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::TextInfoMessage,data.GetParentWnd());
                    dt.SetTextInfo("parsing check file...");
                    queueTrdAnswers.Push(dt);
                }


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
                        dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famImportEnd,data.GetParentWnd());
                        dt.SetSuccessfull(false);
                        ssErr<<iss.str();
                        ssErr<<ossErr.str();
                        dt.SetErrString(ssErr.str());
                        queueTrdAnswers.Push(dt);
                        bWasSuccessfull = false;
                        break;
                    }
                    //if (!Market::IsInSessionTabe(data.vRepoTable,bb.Period())){
                        vBars.push_back(bb);
                    //}

                    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
                    //// send completion percentage
                    ///
                    currsize = file.tellg();
                    iCurrProgress = int(50*(currsize/(double)filesize));
                    if(iProgressSent < iCurrProgress){
                        dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famImportCurrent,data.GetParentWnd());
                        dt.SetPercent(iCurrProgress);
                        queueTrdAnswers.Push(dt);
                        iProgressSent = iCurrProgress;
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
                        dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famImportEnd,data.GetParentWnd());
                        dt.SetSuccessfull(false);
                        dt.SetErrString("loading process interrupted");
                        queueTrdAnswers.Push(dt);
                        //bWasBreaked = true;
                        bWasSuccessfull = false;
                        break;
                    }
                }
                //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                // load data from own storage:
                {
                    dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::TextInfoMessage,data.GetParentWnd());
                    dt.SetTextInfo("loading data from storage...");
                    queueTrdAnswers.Push(dt);
                }
                std::vector<std::vector<BarTick>> v_vStoredBars;

                std::time_t tEnd = Bar::DateAccommodate(data.dtEnd,data.iInterval,true) - 1;

                if (bWasSuccessfull){
                    std::vector<std::time_t> vMonthToLoad;



                    std::time_t dtMonthBegin = Storage::dateCastToMonth(data.dtBegin);
                    std::time_t dtMonthEnd   = Storage::dateAddMonth(tEnd);

                    while (dtMonthBegin < dtMonthEnd) {
                        vMonthToLoad.push_back(dtMonthBegin);
                        dtMonthBegin = Storage::dateAddMonth(dtMonthBegin);
                    }
                    //
                    std::stringstream ssOut;

                    for (const std::time_t &t:vMonthToLoad){
                        ssOut.str("");
                        ssOut.clear();
                        //
                        v_vStoredBars.push_back({});
                        //
                        if (data.iInterval == Bar::eInterval::pTick &&
                                !stStore.ReadFromStore(data.TickerID, t, v_vStoredBars.back(), data.dtBegin,tEnd,
                                                   false,data.vRepoTable,
                                                   false,data.vSessionTable,
                                                   ssOut)){
                            bWasSuccessfull = false;
                            dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famImportEnd,data.GetParentWnd());
                            dt.SetSuccessfull(false);
                            ssOut<<"\n\rError reading file.\n";
                            dt.SetErrString(ssOut.str());
                            queueTrdAnswers.Push(dt);

                            break;
                        }
                        else if (data.iInterval != Bar::eInterval::pTick &&
                                 !stStore.ReadFromStore(data.TickerID, t, v_vStoredBars.back(), data.dtBegin,tEnd,
                                                    false,data.vRepoTable,
                                                    true,data.vSessionTable,
                                                    ssOut)){
                             bWasSuccessfull = false;
                             dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famImportEnd,data.GetParentWnd());
                             dt.SetSuccessfull(false);
                             ssOut<<"\n\rError reading file.\n";
                             dt.SetErrString(ssOut.str());
                             queueTrdAnswers.Push(dt);

                             break;
                         }
                        else{
                            if (ssOut.str().size()>0){
                                dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::logText,data.GetParentWnd());
                                dt.SetTextInfo(ssOut.str());
                                queueTrdAnswers.Push(dt);
                            }
                        }
                        if(this_thread_flagInterrup.isSet()){
                            //fout<<"exit on interrupt\n";
                            dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famImportEnd,data.GetParentWnd());
                            dt.SetSuccessfull(false);
                            dt.SetErrString("loading process interrupted");
                            queueTrdAnswers.Push(dt);
                            bWasSuccessfull = false;
                            break;
                        }
                    }
                }
                iCurrProgress = 60;
                if(iProgressSent < iCurrProgress){
                    dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famImportCurrent,data.GetParentWnd());
                    dt.SetPercent(iCurrProgress);
                    queueTrdAnswers.Push(dt);
                    iProgressSent = iCurrProgress + 5;
                }
                //// send every 0.5 sec activity
                ///
                tActivityCount = std::chrono::steady_clock::now() - dtActivity;
                if (tActivityCount > 500ms){
                    dtActivity = std::chrono::steady_clock::now();
                    dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::LoadActivity,data.GetParentWnd());
                    queueTrdAnswers.Push(dt);
                }
                //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                /// load data to holder
//                {
//                    dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::TextInfoMessage,data.GetParentWnd());
//                    dt.SetTextInfo("parsing storage data to memory structures...");
//                    queueTrdAnswers.Push(dt);
//                }
                GraphHolder hl {data.TickerID};

                if (bWasSuccessfull){

                    if(hl.AddBarsLists(v_vStoredBars,data.dtBegin,tEnd)){
                        ;
                    }
                    if(this_thread_flagInterrup.isSet()){
                        //fout<<"exit on interrupt\n";
                        dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famImportEnd,data.GetParentWnd());
                        dt.SetSuccessfull(false);
                        dt.SetErrString("loading process interrupted");
                        queueTrdAnswers.Push(dt);
                        bWasSuccessfull = false;
                    }
                }
                iCurrProgress = 80;
                if(iProgressSent < iCurrProgress){
                    dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famImportCurrent,data.GetParentWnd());
                    dt.SetPercent(iCurrProgress);
                    queueTrdAnswers.Push(dt);
                    iProgressSent = iCurrProgress + 5;
                }
                //// send every 0.5 sec activity
                ///
                tActivityCount = std::chrono::steady_clock::now() - dtActivity;
                if (tActivityCount > 500ms){
                    dtActivity = std::chrono::steady_clock::now();
                    dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::LoadActivity,data.GetParentWnd());
                    queueTrdAnswers.Push(dt);
                }
                //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                // compare vectors
                if (bWasSuccessfull){
                    bIsEqual = compareBars(hl,data,vBars,queueTrdAnswers,dtActivity);
                }
                //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                if(this_thread_flagInterrup.isSet()){
                    //fout<<"exit on interrupt\n";
                    dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famImportEnd,data.GetParentWnd());
                    dt.SetSuccessfull(false);
                    dt.SetErrString("loading process interrupted");
                    queueTrdAnswers.Push(dt);
                    bWasSuccessfull = false;
                }
            }
        }
        /////
        if (!bFileOpened){
            dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famImportEnd,data.GetParentWnd());
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
                ss << "Check time: "<<tCount.count()<<" seconds\n";
                dataBuckgroundThreadAnswer dt (data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::TextInfoMessage,data.GetParentWnd());
                dt.SetTextInfo(ss.str());
                queueTrdAnswers.Push(dt);
            }

            dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famImportEnd,data.GetParentWnd());
            dt.SetSuccessfull(bIsEqual);
            if (!bIsEqual){
                dt.SetErrString("checkfile and stored data are not equal!");
            }
            queueTrdAnswers.Push(dt);
        }
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------
bool workerLoader::compareBars(GraphHolder &hl,dataFinLoadTask & data,std::vector<Bar> &vBars,
                               BlockFreeQueue<dataBuckgroundThreadAnswer> &queueTrdAnswers,
                               std::chrono::time_point<std::chrono::steady_clock> dtActivity)
{
    if (data.iInterval == Bar::eInterval::pTick){
        return compareBarsT<BarTick>(hl,data,vBars,queueTrdAnswers,dtActivity);
    }
    else{
        return compareBarsT<Bar>(hl,data,vBars,queueTrdAnswers,dtActivity);
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------
template<typename T>
bool workerLoader::compareBarsT(GraphHolder &hl, dataFinLoadTask & data,std::vector<Bar> &vBars,
                                BlockFreeQueue<dataBuckgroundThreadAnswer> &queueTrdAnswers,
                                std::chrono::time_point<std::chrono::steady_clock> dtActivity)
{
    {
        dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::TextInfoMessage,data.GetParentWnd());
        dt.SetTextInfo("begin checking data...");
        queueTrdAnswers.Push(dt);
    }

    milliseconds tActivityCount = std::chrono::steady_clock::now() - dtActivity;


    int iCurrProgress{0};
    int iProgressSent{0};

    std::time_t t1{0};
    std::time_t t2{0};

    bool bSuccess{false};

    auto ItTotalEnd (hl.end<T>());
    auto It  (hl.beginIteratorByDate<T>(Bar::castInterval(data.iInterval),data.dtBegin,bSuccess));
    if (bSuccess){
        auto ItEnd  (hl.beginIteratorByDate<T>(Bar::castInterval(data.iInterval),
                                                         Bar::DateAccommodate(data.dtEnd,Bar::castInterval(data.iInterval),true)
                                                         ,bSuccess));
        if (bSuccess){
            //if (ItEnd != ItTotalEnd) ++ItEnd;
            auto ItChck (vBars.begin());
            auto ItChckEnd (vBars.end());

            ///////////////////////////////////////////////////////////////////////////////
            while(It != ItEnd && ItChck != ItChckEnd){
                if (It->Period() > ItChck->Period()){
                    t1 = ItChck->Period();
                    t2 = It->Period();
                    ItChck = std::lower_bound( ItChck,ItChckEnd,It->Period());

                    bSuccess = false;
                    // send text
                    std::stringstream ss;
                    ss <<"storage has no data from: " <<threadfree_gmtime_to_str(&t1) <<" prior to: " <<threadfree_gmtime_to_str(&t2)<<"";
                    dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::TextInfoMessage,data.GetParentWnd());
                    dt.SetTextInfo(ss.str());
                    queueTrdAnswers.Push(dt);
                }
                else if (It->Period() < ItChck->Period()){
                    t1 = It->Period();
                    t2 = ItChck->Period();

                    It = hl.beginIteratorByDate<T>(Bar::castInterval(data.iInterval),ItChck->Period(),bSuccess);
                    if (!bSuccess){
                        dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::TextInfoMessage,data.GetParentWnd());
                        dt.SetTextInfo("cannot get data from holder {3}");
                        queueTrdAnswers.Push(dt);
                        bSuccess = false;
                        break;
                    }
                    bSuccess = false;
                    // send text
                    std::stringstream ss;
                    ss <<"no data in checkfile from: " <<threadfree_gmtime_to_str(&t1) <<" prior to: " <<threadfree_gmtime_to_str(&t2)<<"";
                    dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::TextInfoMessage,data.GetParentWnd());
                    dt.SetTextInfo(ss.str());
                    queueTrdAnswers.Push(dt);
                }
                else if (!ItChck->equal(*It)){
                    // send text
                    t1 = ItChck->Period();
                    std::stringstream ss;
                    ss <<"not equal data at: " <<threadfree_gmtime_to_str(&t1)<<"\n";
                    ss <<"store OHLCV:\t{"<<It->Open()<<","<<It->High()<<","<<It->Low()<<","<<It->Close()<<","<<It->Volume()<<"}\n";
                    ss <<"check OHLCV:\t{"<<ItChck->Open()<<","<<ItChck->High()<<","<<ItChck->Low()<<","<<ItChck->Close()<<","<<ItChck->Volume()<<"}";
                    dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::TextInfoMessage,data.GetParentWnd());
                    dt.SetTextInfo(ss.str());
                    queueTrdAnswers.Push(dt);
                    ///
                    bSuccess = false;
                    It++;ItChck++;
                }
                else{
                    It++;ItChck++;
                }

                //////////////////
                iCurrProgress = 80 + 20 *  ( std::distance(vBars.begin(),ItChck)/(double)vBars.size());
                if(iProgressSent < iCurrProgress){
                    dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famImportCurrent,data.GetParentWnd());
                    dt.SetPercent(iCurrProgress);
                    queueTrdAnswers.Push(dt);
                    iProgressSent = iCurrProgress + 5;
                }
                //// send every 0.5 sec activity
                ///
                tActivityCount = std::chrono::steady_clock::now() - dtActivity;
                if (tActivityCount > 500ms){
                    dtActivity = std::chrono::steady_clock::now();
                    dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::LoadActivity,data.GetParentWnd());
                    queueTrdAnswers.Push(dt);
                }
                //////////////////
                if(this_thread_flagInterrup.isSet()){
                    //fout<<"exit on interrupt\n";
                    dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famImportEnd,data.GetParentWnd());
                    dt.SetSuccessfull(false);
                    dt.SetErrString("loading process interrupted");
                    queueTrdAnswers.Push(dt);
                    bSuccess = false;
                    break;
                }
            }
            if (It != ItEnd || ItChck != ItChckEnd){
                bSuccess = false;
                if(It != ItEnd && ItChck == ItChckEnd){
                    // send text
                    t1 = It->Period();
                    std::stringstream ss;
                    ss <<"no tail data in checkfile from: " << threadfree_gmtime_to_str(&t1) <<" to: " <<threadfree_gmtime_to_str(&data.dtEnd);
                    dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::TextInfoMessage,data.GetParentWnd());
                    dt.SetTextInfo(ss.str());
                    queueTrdAnswers.Push(dt);
                }
                else if(It == ItEnd && ItChck != ItChckEnd){
                    // send text
                    t1 = ItChck->Period();
                    std::stringstream ss;
                    ss <<"no tail data in storage from: " <<threadfree_gmtime_to_str(&t1) <<" to: " <<threadfree_gmtime_to_str(&data.dtEnd);
                    dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::TextInfoMessage,data.GetParentWnd());
                    dt.SetTextInfo(ss.str());

                    queueTrdAnswers.Push(dt);
                }
                else{
                    // was break;
                }
            }

            if(iProgressSent < iCurrProgress){
                dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famImportCurrent,data.GetParentWnd());
                dt.SetPercent(100);
                queueTrdAnswers.Push(dt);
            }
            ///////////////////////////////////////////////////////////////////////////////
        }
        else{
            dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::TextInfoMessage,data.GetParentWnd());
            dt.SetTextInfo("cannot get data from holder {2}");
            queueTrdAnswers.Push(dt);
        }
    }
    else{
        dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::TextInfoMessage,data.GetParentWnd());
        dt.SetTextInfo("cannot get data from holder {1}");
        queueTrdAnswers.Push(dt);

    }
    {
        dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::TextInfoMessage,data.GetParentWnd());
        dt.SetTextInfo("data checking finished.");
        queueTrdAnswers.Push(dt);
    }
    return bSuccess;
}
//------------------------------------------------------------------------------------------------------------------------------------------
//////********************************************************************************************************************************//////
//------------------------------------------------------------------------------------------------------------------------------------------
void workerLoader::workerAmiClient(BlockFreeQueue<dataFinLoadTask> & /*queueFinQuotesLoad*/,
                            BlockFreeQueue<dataBuckgroundThreadAnswer> &queueTrdAnswers,
                            BlockFreeQueue<dataAmiPipeTask> &queuePipeTasks,
                            BlockFreeQueue<dataAmiPipeAnswer> &queuePipeAnswers,
                            BlockFreeQueue<dataFastLoadTask> &queueFastTasks,
                            AmiPipeHolder& pipesHolder)
{
    //    int iID = WorkerThreadCounter.load();
    //    while (!WorkerThreadCounter.compare_exchange_weak(iID,iID + 1)) {;}
    //    WorkerThreadID = iID;
    {
        ThreadFreeCout pcout;
        pcout <<"workerAmiClient in\n";
    }

//    std::string sPath = "\\\\.\\pipe\\AmiBroker2QUIK_TQBR.SBER_TICKS";
//    pipesHolder.testPipe.setPipePath(sPath);
//    pipesHolder.testPipe.open();
//    char testbuffer[1024];
//    int iBytesRead{0};

//    std::this_thread::sleep_for(std::chrono::microseconds(10));
//    pipesHolder.testPipe.read(testbuffer,1024,iBytesRead);

        try{
            bool bSuccess{false};
            bool bLoop{true};
            size_t iBytesRead{0};
            while(bLoop){
                ///////////////////////////////////////////////////////////////////////////////
                /// pipe task check block
                ///
                auto pdata =queuePipeTasks.Pop(bSuccess);
                bool bWasRefresh{false};
                while(bSuccess){
                    dataAmiPipeTask data(*pdata.get());

                    iBytesRead = 0;

                    switch (data.Type()) {
                    case dataAmiPipeTask::eTask_type::Nop:
                        break;
                    case dataAmiPipeTask::eTask_type::RefreshPipeList:
                        if (!bWasRefresh){

                            pipesHolder.RefreshActiveSockets(data.pipesBindedActive,data.pipesBindedOff,queuePipeAnswers);
                        }
                        bWasRefresh = true;
                        break;

                    }
                    ///
                    if (!this_thread_flagInterrup.isSet()){
                        pdata =queuePipeTasks.Pop(bSuccess);
                    }
                    else{
                        bSuccess = false; // to exit while
                    }
                }
                ///////////////////////////////////////////////////////////////////////////////
                // socket work block

                pipesHolder.ReadConnectedPipes(queueFastTasks,queuePipeAnswers,queueTrdAnswers,iBytesRead);

                ///////////////////////////////////////////////////////////////////////////////
                //----------------------------------
                if (this_thread_flagInterrup.isSet()){
                    bLoop = false; // to exit while
                }
                else
                {
                    // TODO: to fast read. if uncomment there will be one tick packets.
                    // Do some buffering maybe?
                    // or read incoming message whole at once?
                    //if (iBytesRead == 0)
                    {
                        std::this_thread::sleep_for(std::chrono::microseconds(100));
                    }

                    if (this_thread_flagInterrup.isSet()){
                        bLoop = false; // to exit while
                    }
                }
            }
        }
        catch (std::exception &ex) {
            ThreadFreeCout pcout;
            pcout <<"crash ecxeption" <<ex.what()<<"\n";
        }
    {
        ThreadFreeCout pcout;
        pcout <<"workerAmiClient out\n";
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------
void workerLoader::workerFastDataWork( BlockFreeQueue<dataFastLoadTask>     &queueFastTasks,
                                //BlockFreeQueue<dataBuckgroundThreadAnswer>  &queueTrdAnswers,
                                BlockFreeQueue<dataAmiPipeAnswer>           &queuePipeAnswers,
                                FastTasksHolder &fastHolder,
                                Storage &stStore,
                                std::map<int,std::shared_ptr<GraphHolder>> &Holders)
{

    {
        ThreadFreeCout pcout;
        pcout <<"workerFastDataWork in\n";
    }
    bool bSuccess{false};
    try{
        while(true){
            ///////////////////////////////////////////////////////////////////////////////
            if (this_thread_flagInterrup.isSet()){
                break;// to exit while
            }
            ///////////////////////////////////////////////////////////////////////////////
            std::unique_lock lk(mutexConditionFastData);
            conditionFastData.wait_for (lk,milliseconds(1000),[&]{
                        return !queueFastTasks.empty();});
            lk.unlock();
            ///////////////////////////////////////////////////////////////////////////////
            if (this_thread_flagInterrup.isSet()){
                break;// to exit while
            }
            ///////////////////////////////////////////////////////////////////////////////
            if(!queueFastTasks.empty()){
                auto pdata =queueFastTasks.Pop(bSuccess);
                while(bSuccess){
                    dataFastLoadTask &data(*pdata.get());
                    ////////////////////////////////////////
                    switch (data.TaskType()) {
                    case dataFastLoadTask::eTaskType::NewTicks:
                        fastHolder.PacketReceived(data,stStore,Holders,queueFastTasks,queuePipeAnswers);
                        break;
                    case dataFastLoadTask::eTaskType::Nop:
                        break;
                    }
                    ////////////////////////////////////////
                    if (this_thread_flagInterrup.isSet()){
                        break;// to exit while
                    }
                    ////////////////////////////////////////
                    pdata =queueFastTasks.Pop(bSuccess);
                }
            }
        }
    }
    catch (std::exception &ex) {
        ThreadFreeCout pcout;
        pcout <<"crash ecxeption" <<ex.what()<<"\n";
    }

    {
        ThreadFreeCout pcout;
        pcout <<"workerFastDataWork out\n";
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------
