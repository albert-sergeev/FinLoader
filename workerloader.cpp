/****************************************************************************
*  This is part of FinLoader
*  Copyright (C) 2021  Albert Sergeyev
*  Contact: albert.s.sergeev@mail.ru
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <https://www.gnu.org/licenses/>.
****************************************************************************/

#include "workerloader.h"
#include "threadfreecout.h"
#include "threadpool.h"

#include<sstream>
#include<fstream>
#include<ostream>
#include<chrono>

#include "graph.h"
#include "threadfreelocaltime.h"


//using namespace std::chrono_literals;
using seconds=std::chrono::duration<double>;
using milliseconds=std::chrono::duration<double,
    std::ratio_multiply<seconds::period,std::milli>
    >;


//------------------------------------------------------------------------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief workerDataBaseWork Base process for database works like importing/check/optimisation data tasks.
/// Process have limited lifetime (until complite all tasks). If needed so, environment must run more instances of the process
/// \param queueFinQuotesLoad queue for task from wich process pull next task until queue will become empty, then exit.
/// \param queueTrdAnswers queue answer results
/// \param stStore partal threadsafe class for work with database
///
void workerLoader::workerDataBaseWork(BlockFreeQueue<dataFinLoadTask> & queueTasks,
                                      BlockFreeQueue<dataBuckgroundThreadAnswer> &queueTrdAnswers,
                                       Storage & stStore)
{

    try{
        bool bSuccess{false};
        auto pdata = queueTasks.Pop(bSuccess);
        while(bSuccess){
            ActiveProcessCounter counter;

            // pop next task to process
            dataFinLoadTask data(*pdata.get());
            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            /// one task start

            // select function to process data
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
/////////////////
/// \brief workerEtalon typical structure of process function (for develop purpouse)
///
void workerLoader::workerEtalon(BlockFreeQueue<dataFinLoadTask> & queueFilLoad,
                               BlockFreeQueue<dataBuckgroundThreadAnswer> &queueTrdAnswers)
{
    {
        ThreadFreeCout pcout;
        pcout<<"workerEtalon in\n";
    }

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
            std::this_thread::sleep_for(milliseconds(4));//4ms

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
    {
        ThreadFreeCout pcout;
        pcout<<"workerEtalon out\n";
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief workerFinQuotesLoad process to load history data from file. Used by workerDataBaseWork
/// \param queueTasks queue of tasks
/// \param queueTrdAnswers queue for answers
/// \param stStore partal threadsafe class for work with database
/// \param data data task
///
void workerLoader::workerFinQuotesLoad(BlockFreeQueue<dataFinLoadTask> & queueTasks,
                                BlockFreeQueue<dataBuckgroundThreadAnswer> &queueTrdAnswers,
                                Storage & stStore,
                                dataFinLoadTask & data)

{
    // algorithm:
    // 1. check if all correct with database
    // 2. open file to import
    // 3. read and parse file line by line
    // 4. push data line (i.e. trade Bar) to buffer with control chunks if needed:
    //     a. clear import range
    //     b. check current month (SS-tables divided by month)
    //     c. since the primary index in the SS table is <second interval> and the data arrives at millisecond intervals, we create new_second escape sequences
    // !uses MutexDefender class to correctly set mutex on SS-table
    // 5. write buffer to SS-table when it is full or month changed or tail at the end
    // 6. sending activity events wherever possible

    {//activity event
        dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famImportBegin,data.GetParentWnd());
        queueTrdAnswers.Push(dt);
    }
    {//activity event
        dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famImportCurrent,data.GetParentWnd());
        dt.SetPercent(0);
        queueTrdAnswers.Push(dt);
    }

    std::chrono::time_point dtStart(std::chrono::steady_clock::now());
    std::chrono::time_point dtActivity(std::chrono::steady_clock::now());
    milliseconds tActivityCount = std::chrono::steady_clock::now() - dtActivity;
    bool bWasSuccessfull{false};

    ////////////////////////////////////////////////////////////
    //activity event
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
    /// 1. check if all correct with database

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
        if (ssErr.str().size()>0){ //activity event
            dataBuckgroundThreadAnswer dtT (data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::TextInfoMessage,data.GetParentWnd());
            dtT.SetTextInfo(ssErr.str());
            queueTrdAnswers.Push(dtT);
        }
        ///////////////////////////////////////////////////////////////////////////
        /// 2. open file to import

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
                {//activity event
                    std::stringstream ss;
                    ss << "filesize: " << filesize<<"\n";
                    dataBuckgroundThreadAnswer dt (data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::TextInfoMessage,data.GetParentWnd());
                    dt.SetTextInfo(ss.str());
                    queueTrdAnswers.Push(dt);
                }
                /////////////////////////////
                std::string sBuff;
                std::istringstream iss;

                Storage::MutexDefender<std::shared_lock<std::shared_mutex>> defSlk;
                Storage::MutexDefender<std::unique_lock<std::shared_mutex>> defUlk;

                bWasSuccessfull = true;

                ///////////////////////////////////////////////////////////////////////////
                /// 3. read and parse file line by line

                if (parseDt.HasHeader()){ // skip header
                    std::getline(file,sBuff);
                }                
                while (std::getline(file,sBuff)) {
                    // link stringstream
                    iss.clear();
                    iss.str(sBuff);
                    ////////////
                    // 3.1 parse line
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

                    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
                    /// 5. write buffer to SS-table when it is full or month changed

                    /// 5.1  write buffer if month changed
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

                        /// 4.a. clear import range (on start too, becouse tCurrentMonth on start is 0)
                        iOutBuffPointer = createCleanUpHeader(tCurrentMonth, cOutBuff,data.dtBegin, data.dtEnd);
                    }
                    /// 5.1  write buffer if is full
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
                    //////////////
                    /// 4.c. since the primary index in the SS table is <second interval> and the data arrives at millisecond intervals,
                    ///      we create new_second escape sequences
                    if (tSec != tCurrentSec){
                        iState = Storage::data_type::new_sec;
                        tCurrentSec = tSec;
                    }
                    else{
                        iState = Storage::data_type::usual;
                    }

                    //////////////
                    /// 4. push data line (i.e. trade Bar) to buffer:
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
                    if(iProgressSent < iCurrProgress){ //activity event
                        dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famImportCurrent,data.GetParentWnd());
                        dt.SetPercent(iCurrProgress);
                        queueTrdAnswers.Push(dt);
                        iProgressSent = iCurrProgress;
                    }
                    //// send every 0.5 sec activity
                    ///
                    tActivityCount = std::chrono::steady_clock::now() - dtActivity;
                    if (tActivityCount > milliseconds(500)){ //activity event
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
                if (bWasSuccessfull && iOutBuffPointer>0){
                    ////////////////////////
                    /// 5.1  write tail of buffer
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
        if (!bFileOpened){ //activity event
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
            { //activity event
                std::stringstream ss;
                ss << "Import time: "<<tCount.count()<<" seconds\n";
                dataBuckgroundThreadAnswer dt (data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::TextInfoMessage,data.GetParentWnd());
                dt.SetTextInfo(ss.str());
                queueTrdAnswers.Push(dt);
            }

            //activity event
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

            { // order the application to create new worker thread if needed
                dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::cloneThread,data.GetParentWnd());
                queueTrdAnswers.Push(dt);
            }
            { // order the application to create new worker thread if needed
                dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::cloneThread,data.GetParentWnd());
                queueTrdAnswers.Push(dt);
            }
        }
    }

}
//------------------------------------------------------------------------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief createCleanUpHeader Utility to form control title packet in SS-table. Used by workerFinQuotesLoad
///
int workerLoader::createCleanUpHeader(std::time_t tMonth, char* cBuff,std::time_t tBegin, std::time_t tEnd)
{
    // algorithm
    // uses two consecutive bars to store start and end of deleted range
    // the time range cut to the current month
    // return new buffer shift

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
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief workerLoadFromStorage  process to load data from SS-table part of database. Used by workerDataBaseWork
/// \param queueTasks queue of tasks
/// \param queueTrdAnswers queue for answers
/// \param stStore partal threadsafe class for work with database
/// \param data data task
///
void workerLoader::workerLoadFromStorage(BlockFreeQueue<dataFinLoadTask> & queueTasks,
                                BlockFreeQueue<dataBuckgroundThreadAnswer> &queueTrdAnswers,
                                Storage & stStore,
                                dataFinLoadTask & data)

{
    // algorithm
    // 1. init data storage
    // 2. calculate month sequence (time range can has more then one)
    // 3. consistently load data, month by month
    // 4. form tasks to make LSM-tree or merge and seal SS-tables
    // 5. sending activity events wherever possible
    { //activity event
        dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::storagLoadFromStorageGraphBegin,data.GetParentWnd());
        queueTrdAnswers.Push(dt);
    }
    //
    std::chrono::time_point dtStart(std::chrono::steady_clock::now());
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool bSuccessfull{true};

    ///////////////////////////////////
    // 1. init data storage
    std::stringstream ssErr;
    if (!stStore.InitializeTicker(data.TickerID,ssErr)){
        dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famImportEnd,data.GetParentWnd());
        dt.SetSuccessfull(false);
        ssErr<<"\ncannot initialize stock quotes storage for ticker";
        dt.SetErrString(ssErr.str());
        queueTrdAnswers.Push(dt);
    }
    else{

        ///////////////////////////////////
        // 2. calculate month sequence (time range can has more then one)

        std::vector<std::time_t> vMonthToLoad;

        std::time_t dtMonthBegin = Storage::dateCastToMonth(data.dtBegin);
        std::time_t dtMonthEnd   = Storage::dateAddMonth(data.dtEnd);

        while (dtMonthBegin < dtMonthEnd) {
            vMonthToLoad.push_back(dtMonthBegin);
            dtMonthBegin = Storage::dateAddMonth(dtMonthBegin);
        }
        //
        size_t iCollisionsCount{0};
        std::stringstream ssOut;

        std::shared_ptr<std::vector<std::vector<BarTick>>> pvBars{std::make_shared<std::vector<std::vector<BarTick>>>()};

        ///////////////////////////////////
        // 3. consistently load data, month by month
        for (const std::time_t &t:vMonthToLoad){
            ssOut.str("");
            ssOut.clear();
            iCollisionsCount = 0;
            //
            pvBars->push_back({});
            //
            if (!stStore.ReadFromStore(data.TickerID, t, pvBars->back(), data.dtBegin,data.dtEnd,true,data.vRepoTable,false,data.vSessionTable,ssOut,iCollisionsCount)){
                bSuccessfull = false;
                dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::storagLoadFromStorageGraphEnd,data.GetParentWnd());
                dt.SetSuccessfull(false);
                ssOut<<"\n\rError reading file.\n";
                dt.SetErrString(ssOut.str());
                queueTrdAnswers.Push(dt);

                break;
            }
            else{
                if (ssOut.str().size()>0){ //activity event
                    dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::logText,data.GetParentWnd());
                    dt.SetTextInfo(ssOut.str());
                    queueTrdAnswers.Push(dt);
                }
                if(iCollisionsCount > 1000){

                    ///////////////////////////////////
                    // 4.1 form tasks to  merge and seal SS-tables

                    /// if too many collisions - do optimisations
                    dataFinLoadTask optTask(data);// copying data range and tickerID
                    optTask.taskType = dataFinLoadTask::TaskType::storageOptimisation;
                    optTask.dtBegin = t;
                    optTask.dtEnd = t;
                    queueTasks.Push(optTask);

                    //activity event
                    dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::logText,data.GetParentWnd());
                    std::stringstream ss;
                    ss <<"{"<< data.TickerID<<"} ";
                    ss << "To many collisions: "<<iCollisionsCount;
                    ss << " Request optimisation for: "<<threadfree_gmtime_to_str(&t)<<"\n";
                    dt.SetTextInfo(ss.str());
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
        ///////////////////////////////////
        // 4.2 form tasks to make LSM-tree
        dataFinLoadTask optTask(data);// copying data range and tickerID
        optTask.taskType = dataFinLoadTask::TaskType::LoadIntoGraph;
        optTask.pvBars =pvBars;
        queueTasks.Push(optTask);
        { // ordering the application to create a new worker process
            dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::cloneThread,data.GetParentWnd());
            queueTrdAnswers.Push(dt);
        }
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if (bSuccessfull){
        std::chrono::time_point dtStop(std::chrono::steady_clock::now());
        //milliseconds tCount = dtStop - dtStart;
        seconds tCount = dtStop - dtStart;
        { //activity event
            std::stringstream ss;
            ss <<"{"<< data.TickerID<<"} ";
            ss << "Import time: "<<tCount.count()<<" seconds\n";
            dataBuckgroundThreadAnswer dt (data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::logText,data.GetParentWnd());
            dt.SetTextInfo(ss.str());
            queueTrdAnswers.Push(dt);
        }
        //activity event
        dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::storagLoadFromStorageGraphEnd,data.GetParentWnd());
        dt.SetSuccessfull(true);
        queueTrdAnswers.Push(dt);
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------
///////////////
/// \brief workerLoadIntoGraph process the data read from the SS-table by the workerLoadFromStorage and form correct LSM-tree.  Used by workerDataBaseWork
/// \param queueTrdAnswers queue for answers
/// \param data data task
///
void workerLoader::workerLoadIntoGraph(BlockFreeQueue<dataFinLoadTask> & /*queueTasks*/,
                                BlockFreeQueue<dataBuckgroundThreadAnswer> &queueTrdAnswers,
                                Storage & /*stStore*/,
                                dataFinLoadTask & data)
{
    // algorithm
    // 1.take data task and check if all fields filled correctly
    // 2. run AddBarsLists from GraphHolder object
    // 3. for tests - run CheckMap
    // 4. emit event with result

    {
        dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::storagLoadToGraphBegin,data.GetParentWnd());
        queueTrdAnswers.Push(dt);
    }
    std::chrono::time_point dtStart(std::chrono::steady_clock::now());
    bool bSuccessfull{true};

    //////////////////////////////////////////////////////////////////////////////////
    // 1.take data task and check if all fields filled correctly

    if (data.holder != std::shared_ptr<GraphHolder>{}){

        if(std::shared_ptr<std::vector<std::vector<BarTick>>>{} != data.pvBars){

            //////////////////////////////////////////////
            // 2. run AddBarsLists from GraphHolder object

            if(data.holder->AddBarsLists(*data.pvBars.get(),data.dtBegin,data.dtEnd)){
                ////////////////////////////////////////////////
                //  3. for tests - run CheckMap
                //
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
    // 4. emit event with result

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
}
//------------------------------------------------------------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////
/// \brief workerOptimizeStorage optimise phisical structure of SS-tables (merge and seal process). Used by workerDataBaseWork
/// \param queueTasks queue of tasks
/// \param queueTrdAnswers queue for answers
/// \param stStore partal threadsafe class for work with database
/// \param data task
///
void workerLoader::workerOptimizeStorage(BlockFreeQueue<dataFinLoadTask> & queueTasks,
                                BlockFreeQueue<dataBuckgroundThreadAnswer> &queueTrdAnswers,
                                Storage & stStore,
                                dataFinLoadTask & data)
{
    // algorithm
    // 1. check if all correct with database
    // 2. calculate month sequence (time range can has more then one)
    // 3. consistently run optimizing task, month by month
    // 3.1 if needed, order next shift of SS-files
    // 4. sending activity events wherever possible

        { //activity event
            dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::storageOptimisationBegin,data.GetParentWnd());
            queueTrdAnswers.Push(dt);
        }
        //
        std::chrono::time_point dtStart(std::chrono::steady_clock::now());
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // 1. check if all correct with database

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

            //////////////////////////////////////////////////////////////////
            // 2. calculate month sequence (time range can has more then one)

            std::time_t dtMonthBegin = Storage::dateCastToMonth(data.dtBegin);
            std::time_t dtMonthEnd   = Storage::dateAddMonth(data.dtEnd);

            while (dtMonthBegin < dtMonthEnd) {
                vMonthToLoad.push_back(dtMonthBegin);
                dtMonthBegin = Storage::dateAddMonth(dtMonthBegin);
            }
            //
            std::stringstream ssOut;

            bool bToPlanNextShift{false};

            //////////////////////////////////////////////////////////////////
            // 3. consistently run optimizing task, month by month

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
                    //////////////////////////////////////////////////////////////////
                    // 3.1 if needed, order next shift of SS-files

                    if (bToPlanNextShift){// plan next optimization task
                        dataFinLoadTask optTask(data);// copying data range and tickerID
                        optTask.dtBegin = t;    // only current month
                        optTask.dtEnd   = t;    // only current month
                        optTask.taskType = dataFinLoadTask::TaskType::storageOptimisation;
                        queueTasks.Push(optTask);
                    }
                    if (ssOut.str().size()>0){ //activity event
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
            { //activity event
                std::stringstream ss;
                ss << "Optimization time: "<<tCount.count()<<" seconds\n";
                dataBuckgroundThreadAnswer dt (data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::logText,data.GetParentWnd());
                dt.SetTextInfo(ss.str());
                queueTrdAnswers.Push(dt);
            }
            //activity event
            dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::storageOptimisationEnd,data.GetParentWnd());
            dt.SetSuccessfull(true);
            queueTrdAnswers.Push(dt);
        }
}
//------------------------------------------------------------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief workerFinQuotesCheck  process to check history data from file whith database. Used by workerDataBaseWork
/// \param queueTrdAnswers queue for answers
/// \param stStore partal threadsafe class for work with database
/// \param data task
///
void workerLoader::workerFinQuotesCheck(BlockFreeQueue<dataFinLoadTask> & /*queueTasks*/,
                                BlockFreeQueue<dataBuckgroundThreadAnswer> &queueTrdAnswers,
                                Storage & stStore,
                                dataFinLoadTask & data)

{
    // algorithm
    // 1. check if all correct with database
    // 2. open file to import
    // 3. read and parse file line by line and push_back to vector with data
    // 4.1 if it is not check in memory - load SS-tables
    // 4.2 if it is not check in memory - convert loaded SS-tables to temporary LSM-tree
    // 4.3 if it is check in memory - just use current main data holder
    // 5. compare data in LSM-tree (temporary or current) with prepared early vector witn data
    // 6. sending activity events wherever possible

    { //activity event
        dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famImportBegin,data.GetParentWnd());
        queueTrdAnswers.Push(dt);
    }
    { //activity event
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
    //activity event
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
    // 1. check if all correct with database

    std::stringstream ssErr;
    if (!stStore.InitializeTicker(data.TickerID,ssErr)){
        dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famImportEnd,data.GetParentWnd());
        dt.SetSuccessfull(false);
        ssErr<<"\ncannot initialize stock quotes storage for ticker";
        dt.SetErrString(ssErr.str());
        queueTrdAnswers.Push(dt);
    }
    else{
        if (ssErr.str().size()>0){ //activity event
            dataBuckgroundThreadAnswer dtT (data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::TextInfoMessage,data.GetParentWnd());
            dtT.SetTextInfo(ssErr.str());
            queueTrdAnswers.Push(dtT);
        }
        ///////////////////////////////////////////////////////////////////////////
        // 2. open file to import

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
                {//activity event
                    std::stringstream ss;
                    ss << "filesize: " << filesize<<"\n";
                    dataBuckgroundThreadAnswer dt (data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::TextInfoMessage,data.GetParentWnd());
                    dt.SetTextInfo(ss.str());
                    queueTrdAnswers.Push(dt);
                }
                /////////////////////////////
                std::string sBuff;
                std::istringstream iss;

                {//activity event
                    dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::TextInfoMessage,data.GetParentWnd());
                    dt.SetTextInfo("parsing check file...");
                    queueTrdAnswers.Push(dt);
                }

                ////////////////////////////////////////////////////////////////////////
                // 3. read and parse file line by line and push_back to vector with data


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
                    // activity event
                    // send completion percentage
                    currsize = file.tellg();
                    iCurrProgress = int(50*(currsize/(double)filesize));
                    if(iProgressSent < iCurrProgress){
                        dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famImportCurrent,data.GetParentWnd());
                        dt.SetPercent(iCurrProgress);
                        queueTrdAnswers.Push(dt);
                        iProgressSent = iCurrProgress;
                    }
                    // activity event
                    // send every 0.5 sec activity
                    tActivityCount = std::chrono::steady_clock::now() - dtActivity;
                    if (tActivityCount > milliseconds(500)){//500ms
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
                {// activity event
                    dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::TextInfoMessage,data.GetParentWnd());
                    dt.SetTextInfo("loading data from storage...");
                    queueTrdAnswers.Push(dt);
                }
                std::shared_ptr<GraphHolder> ptrHolder = std::make_shared<GraphHolder>(GraphHolder{data.TickerID});

                /////////////////////////////////////////////////////
                // 4.1 if it is not check in memory - load SS-tables

                if (!data.bCheckInMemory){
                    // load data from own storage:
                    // filling data from store

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
                        size_t iCollisionsCount;

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
                                                       ssOut,iCollisionsCount)){
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
                                                        ssOut,iCollisionsCount)){
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
                    if(iProgressSent < iCurrProgress){ // activity event
                        dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famImportCurrent,data.GetParentWnd());
                        dt.SetPercent(iCurrProgress);
                        queueTrdAnswers.Push(dt);
                        iProgressSent = iCurrProgress + 5;
                    }
                    // activity event
                    // send every 0.5 sec activity
                    tActivityCount = std::chrono::steady_clock::now() - dtActivity;
                    if (tActivityCount > milliseconds(500)){//500ms
                        dtActivity = std::chrono::steady_clock::now();
                        dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::LoadActivity,data.GetParentWnd());
                        queueTrdAnswers.Push(dt);
                    }
                    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                    // 4.2 if it is not check in memory - convert loaded SS-tables to temporary LSM-tree
                    /// load data to holder

                    if (bWasSuccessfull){
                        if(ptrHolder->AddBarsLists(v_vStoredBars,data.dtBegin,tEnd)){
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
                }
                else{
                    ///////////////////////////////////////////////////////////////////
                    // 4.3 if it is check in memory - just use current main data holder
                    // using contained in memory data
                    ptrHolder = data.holder;
                }
                iCurrProgress = 80;
                if(iProgressSent < iCurrProgress){// activity event
                    dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::famImportCurrent,data.GetParentWnd());
                    dt.SetPercent(iCurrProgress);
                    queueTrdAnswers.Push(dt);
                   // iProgressSent = iCurrProgress + 5;
                }
                // send every 0.5 sec activity
                tActivityCount = std::chrono::steady_clock::now() - dtActivity;
                if (tActivityCount > milliseconds(500)){// activity event
                    dtActivity = std::chrono::steady_clock::now();
                    dataBuckgroundThreadAnswer dt(data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::LoadActivity,data.GetParentWnd());
                    queueTrdAnswers.Push(dt);
                }
                //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                // 5. compare data in LSM-tree (temporary or current) with prepared early vector witn data
                if (bWasSuccessfull){
                    bIsEqual = compareBars(*ptrHolder,data,vBars,queueTrdAnswers,dtActivity);
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
            {// activity event
                std::stringstream ss;
                ss << "Check time: "<<tCount.count()<<" seconds\n";
                dataBuckgroundThreadAnswer dt (data.TickerID,dataBuckgroundThreadAnswer::eAnswerType::TextInfoMessage,data.GetParentWnd());
                dt.SetTextInfo(ss.str());
                queueTrdAnswers.Push(dt);
            }

            // send back results
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
///////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief compareBars utility function to compare data storage with data set. Used by workerFinQuotesCheck
///
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
    // algorithm
    // two ranges standard compare by iterating throught two iterators and retun false if not equal or has different size
    // check differ and send back with log events

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
                if (tActivityCount > milliseconds(500)){//500ms
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
/// \brief workerAmiClient Client for pipes with trade data sources (fast time work).
/// Process has !infinite! lifetime. Works until this_thread_flagInterrup is set
/// !Warning! Only one instanse of process must be run, or you must divide serviced pipes between instances.
/// \param queueFinQuotesLoad queue for database work tasks (need to give orders for optimisation and etc.)
/// \param queueTrdAnswers  queue for answers
/// \param queuePipeTasks queue for receive main task or process
/// \param queuePipeAnswers queue for answers
/// \param queueFastTasks queue for setting tasks to process received packets with data
/// \param pipesHolder partal threadsafe class for work with connections (pipes)
///
void workerLoader::workerAmiClient(BlockFreeQueue<dataFinLoadTask> & /*queueFinQuotesLoad*/,
                            BlockFreeQueue<dataBuckgroundThreadAnswer> &queueTrdAnswers,
                            BlockFreeQueue<dataAmiPipeTask> &queuePipeTasks,
                            BlockFreeQueue<dataAmiPipeAnswer> &queuePipeAnswers,
                            BlockFreeQueue<dataFastLoadTask> &queueFastTasks,
                            AmiPipeHolder& pipesHolder)
{   
    // algorithm
    // 1. infinite loop until this_thread_flagInterrup is set
    // 2. process two type of task:
    //      2.1. receive list of pipes to get ticker name from them (not for work)
    //      2.2. receive list of work pipes with ones to connect or disconnect
    // 3. read the pipes for which we want to know the name of the ticker
    // 4. read the pipes we actually work with

        try{
            ///////////////////////////////////////////////////////////////////////////////
            // 1. infinite loop until this_thread_flagInterrup is set
            bool bSuccess{false};
            bool bLoop{true};
            int iBytesRead{0};
            bool bWasFullBuffers{false};
            while(bLoop){
                ///////////////////////////////////////////////////////////////////////////////
                /// pipe task check block
                ///
                auto pdata =queuePipeTasks.Pop(bSuccess);
                bool bWasRefresh{false};
                while(bSuccess){
                    ActiveProcessCounter counter;
                    dataAmiPipeTask data(*pdata.get());


                    iBytesRead = 0;

                    switch (data.Type()) {
                    case dataAmiPipeTask::eTask_type::Nop:
                        break;
                    case dataAmiPipeTask::eTask_type::RefreshPipeList:
                        ////////////////////////////////////////////////////////////////////////
                        // 2.2. receive list of work pipes with ones to connect or disconnect

                        if (!bWasRefresh){//only once at loop
                            pipesHolder.RefreshActiveSockets(data.pipesBindedActive,data.pipesBindedOff,queuePipeAnswers);
                        }
                        bWasRefresh = true;
                        break;
                    case dataAmiPipeTask::eTask_type::AskPipesNames:
                        ////////////////////////////////////////////////////////////////////////
                        //2.1. receive list of pipes to get ticker name from them (not for work)

                        pipesHolder.AskPipesNames(data.pipesFree,queuePipeAnswers);
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

                ////////////////////////////////////////////
                // 4. read the pipes we actually work with

                {
                    ActiveProcessCounter counter;
                    pipesHolder.ReadConnectedPipes(queueFastTasks,queuePipeAnswers,queueTrdAnswers,false,iBytesRead,bWasFullBuffers);
                }

                ////////////////////////////////////////////
                // 3. read the pipes for which we want to know the name of the ticker

                {
                    ActiveProcessCounter counter;
                    int iBytesRead;
                    bool bWasFullBuffers;
                    pipesHolder.ReadConnectedPipes(queueFastTasks,queuePipeAnswers,queueTrdAnswers,true,iBytesRead,bWasFullBuffers);
                }

                ///////////////////////////////////////////////////////////////////////////////
                //----------------------------------
                if (this_thread_flagInterrup.isSet()){
                    bLoop = false; // to exit while
                }
                else
                {
                    if (!bWasFullBuffers)
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
}
//------------------------------------------------------------------------------------------------------------------------------------------
/// \brief workerFastDataWork process for parallel processing receiving data packets (long time work).
/// Inserting into LSM-tree database, forming data chunk to didplay by GUI and etc.
/// Process has !infinite! lifetime. Works until this_thread_flagInterrup is set.
/// You can run several instances of process - the received chunks of data are correctly ordered and processed.
/// \param queueFastTasks queue for main task
/// \param queuePipeAnswers queue for answer
/// \param fastHolder partal threadsafe class for work with data chunks
/// \param stStore stStore partal threadsafe class for work with database
/// \param Holders main in-memory storage of trade data
///
void workerLoader::workerFastDataWork( BlockFreeQueue<dataFastLoadTask>     &queueFastTasks,
                                //BlockFreeQueue<dataBuckgroundThreadAnswer>  &queueTrdAnswers,
                                BlockFreeQueue<dataAmiPipeAnswer>           &queuePipeAnswers,
                                FastTasksHolder &fastHolder,
                                Storage &stStore,
                                std::map<int,std::shared_ptr<GraphHolder>> &Holders)
{

    // algorithm
    // 1. waiting on condition variable loop. wait_for 1 sec and exit if this_thread_flagInterrup was set
    // 2. queueFastTasks is not empty, pop task and run processer
    // 3. if queueFastTasks has another task - do it (without waiting on condition variable)

    bool bSuccess{false};
    try{
        while(true){
            ///////////////////////////////////////////////////////////////////////////////
            if (this_thread_flagInterrup.isSet()){
                break;// to exit while
            }
            ///////////////////////////////////////////////////////////////////////////////
            // 1. waiting on condition variable loop. wait_for 1 sec and exit if this_thread_flagInterrup was set

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
                ActiveProcessCounter counter;
                ///////////////////////////////////////////////////////////////////
                // 2. queueFastTasks is not empty, pop task and run processer

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
                    // 3. if queueFastTasks has another task - do it (without waiting on condition variable)

                    pdata =queueFastTasks.Pop(bSuccess);
                }
            }
        }
    }
    catch (std::exception &ex) {
        ThreadFreeCout pcout;
        pcout <<"crash ecxeption" <<ex.what()<<"\n";
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
