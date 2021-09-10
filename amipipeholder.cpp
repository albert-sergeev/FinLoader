#include "amipipeholder.h"

#include<filesystem>
#include<iostream>
#include<regex>
#include<iomanip>
#include<fstream>
#include<ostream>
#include<chrono>
#include<sstream>
#include "trimutils.h"
#include "storage.h"



using namespace std::chrono_literals;
using seconds=std::chrono::duration<double>;
using milliseconds=std::chrono::duration<double,
    std::ratio_multiply<seconds::period,std::milli>
    >;


//-------------------------------------------------------------------------------------------------
AmiPipeHolder::AmiPipeHolder():
    aiTaskCounter{1}
{
    std::call_once(AmiPipeHolder_call_once_flag,&AmiPipeHolder::initStartConst,this);

}
//-------------------------------------------------------------------------------------------------
void AmiPipeHolder::initStartConst(){
    std::tm t_tm;
    t_tm.tm_year   = 1971 - 1900;
    t_tm.tm_mon    = 01 - 1;
    t_tm.tm_mday   = 0;
    t_tm.tm_hour   = 0;
    t_tm.tm_min    = 0;
    t_tm.tm_sec    = 0;
    t_tm.tm_isdst  = 0;

    t1971_01_01_00_00_00 = mktime_gm(&t_tm);

    t_tm.tm_year   = 2100 - 1900;
    t_tm.tm_mon    = 01 - 1;
    t_tm.tm_mday   = 0;
    t_tm.tm_hour   = 0;
    t_tm.tm_min    = 0;
    t_tm.tm_sec    = 0;
    t_tm.tm_isdst  = 0;

    t2100_01_01_00_00_00 = mktime_gm(&t_tm);

    t_tm.tm_year   = 1970 - 1900;
    t_tm.tm_mon    = 01 - 1;
    t_tm.tm_mday   = 1;
    t_tm.tm_hour   = 4;
    t_tm.tm_min    = 0;
    t_tm.tm_sec    = 0;
    t_tm.tm_isdst  = 0;

    t1970_01_01_04_00_00 = mktime_gm(&t_tm);
};
//-------------------------------------------------------------------------------------------------
dataAmiPipeTask::pipes_type AmiPipeHolder::ScanActivePipes()
{
    dataAmiPipeTask::pipes_type mRet;

    std::string sPipeDir = "\\\\.\\pipe\\";
    std::stringstream ssFilePath("");


    //std::filesystem::path pathTickerDir = sPipeDir;
    std::filesystem::path pathTickerDir = std::filesystem::absolute(sPipeDir);
//    if(!std::filesystem::is_directory(pathTickerDir)){
//        ssOut <<" ./data/[TickerID] - is not directory";
//        return false;
//    }
    //////////////////
    std::stringstream ssReg;
    std::stringstream ssRegSign;

    //AmiBroker2QUIK_TQBR.SBER_TICKS
    ssReg <<"^AmiBroker2QUIK_(.*)_TICKS$";
    const std::regex reAmiPipe {ssReg.str()};

    ssRegSign <<"(?:(?![.]).)+$";
    const std::regex reAmiSign {ssRegSign.str()};

    std::vector<std::filesystem::directory_entry> vPipes;

    std::string sSign;
    std::string sBind;

#ifdef _WIN32
#elif
    if (std::filesystem::exists(pathTickerDir))
#endif
    {
        for (const std::filesystem::directory_entry &fl:std::filesystem::directory_iterator{pathTickerDir}){

            if ( fl.exists()
                 && fl.is_regular_file()
                 //&& fl.is_fifo()
                 ){
                std::string ss(fl.path().filename().string());
                const auto ItPipe = std::sregex_token_iterator(ss.begin(),ss.end(),reAmiPipe);
                if ( ItPipe != std::sregex_token_iterator()){
                    const auto ItQuik = std::sregex_token_iterator(ss.begin(),ss.end(),reAmiPipe,1);
                    sBind = *ItQuik;
                    if (ItQuik != std::sregex_token_iterator()){
                        const auto ItSign = std::sregex_token_iterator(sBind.begin(),sBind.end(),reAmiSign);
                        sSign = "";
                        if (ItSign != std::sregex_token_iterator()){
                            sSign = *ItSign;
                        }
                        ssFilePath.str("");
                        ssFilePath.clear();
                        ssFilePath <<sPipeDir<<(*ItPipe);
                        //ssFilePath <<fl.path().filename().string();


                        mRet[sBind] = {0,{*ItPipe,sSign,ssFilePath.str(),0,0}};
                    }
                }
            }
        }
    }
    return mRet;
}

//-------------------------------------------------------------------------------------------------
void AmiPipeHolder::CheckPipes(std::vector<Ticker> &vT,
                               dataAmiPipeTask::pipes_type & mBindedPipesActive,
                               dataAmiPipeTask::pipes_type & mBindedPipesOff,
                               dataAmiPipeTask::pipes_type &mFreePipes,
                               std::vector<int> &vUnconnectedPipes,
                               std::vector<int> &vInformantsPipes
                               )
{
    mBindedPipesActive.clear();
    mBindedPipesOff.clear();
    mFreePipes.clear();
    //
    dataAmiPipeTask::pipes_type mPipes = AmiPipeHolder::ScanActivePipes();
    //
    std::string sTmp;
    for(const auto &t:vT){
        sTmp = trim(t.TickerSignQuik());
        if(sTmp.size() > 0 && mPipes.find(sTmp) != mPipes.end()){
            if (t.AutoLoad()){
                mPipes[sTmp].second = {std::get<0>(mPipes[sTmp].second),
                                       std::get<1>(mPipes[sTmp].second),
                                       std::get<2>(mPipes[sTmp].second),
                                       t.TickerID(),
                                       1};
            }
            else{
                mPipes[sTmp].second = {std::get<0>(mPipes[sTmp].second),
                                       std::get<1>(mPipes[sTmp].second),
                                       std::get<2>(mPipes[sTmp].second),
                                       t.TickerID(),
                                       2};
            }

        }
        else{
            if (t.AutoLoad()){
                vUnconnectedPipes.push_back(t.TickerID());
            }
            else{
                vInformantsPipes.push_back(t.TickerID());
            }
        }
    }
    ///
    for(const auto &p:mPipes){
        if (std::get<4>(p.second.second) == 1){
            mBindedPipesActive[p.first] = p.second;
        }
        else if (std::get<4>(p.second.second) == 2){
            mBindedPipesOff[p.first] = p.second;
        }
        else{
            mFreePipes[p.first] = p.second;
        }
    }
}
//-------------------------------------------------------------------------------------------------
void AmiPipeHolder::RefreshActiveSockets(dataAmiPipeTask::pipes_type& pActive,
                          dataAmiPipeTask::pipes_type& pOff,
                          BlockFreeQueue<dataAmiPipeAnswer>&queuePipeAnswers)
{
    for (auto &m:mPipesHalted){m.second.first =0;}
    for (auto &m:mPipesConnected){m.second.first =0;}
    //
    for(const auto & p:pActive){
        auto ItHalted (mPipesHalted.find(p.first));
        if ( ItHalted == mPipesHalted.end()){
            auto ItConnected (mPipesConnected.find(p.first));
            if ( ItConnected == mPipesConnected.end()){
//                {
//                    ThreadFreeCout pcout;
//                    pcout <<"pipename: "<<std::get<2>(p.second.second)<<"\n";
//                    if(std::filesystem::is_fifo(std::get<2>(p.second.second))){
//                        pcout <<"is fifo\n";
//                    }
//                    else{
//                        pcout <<"is regular file\n";
//                    }
//                }
                bool bOpend{false};
                try{
#ifdef _WIN32
                    mPipesConnected[p.first].first  = 1;
                    mPipesConnected[p.first].second = {std::get<3>(p.second.second),{}};
                    mPipesConnected[p.first].second.second.setPipePath(std::get<2>(p.second.second));

                    mPipesConnected[p.first].second.second.open();

                    if (mPipesConnected[p.first].second.second.good()){
#else
                    std::ifstream file(std::get<2>(p.second.second),std::ios::in);
                    if (file.good()){
                        mPipesConnected[p.first].second = {std::get<3>(p.second.second),
                                                            std::move(file)};
#endif
                        AddUtilityMapEntry(std::get<3>(p.second.second), p.first);
                        //
                        dataAmiPipeAnswer answ;
                        answ.Type = dataAmiPipeAnswer::PipeConnected;
                        answ.iTickerID = std::get<3>(p.second.second);
                        queuePipeAnswers.Push(answ);
                        bOpend = true;
                    }
#ifdef _WIN32
                    else{
                        auto ItD (mPipesConnected.find(p.first));
                        if (ItD != mPipesConnected.end()){
                            mPipesConnected.erase(ItD);
                        }
                    }
#endif
                }
                catch(std::exception &){
                    ;
                }
                if (!bOpend){
                    mPipesHalted[p.first].first  = 1;
#ifdef _WIN32
                    Win32NamedPipe file("");
                    mPipesHalted[p.first].second = {std::get<3>(p.second.second),file};
#else
                    mPipesHalted[p.first].second = {std::get<3>(p.second.second),
                                                    std::ifstream{}
                                                   };
#endif
                    //
                    dataAmiPipeAnswer answ;
                    answ.Type = dataAmiPipeAnswer::PipeHalted;
                    answ.iTickerID = std::get<3>(p.second.second);
                    queuePipeAnswers.Push(answ);
                }
            }
            else{
                ItConnected->second.first = 1;
            }
        }
        else{
            ItHalted->second.first = 1;
        }
    }
    //////////////////////////////////////////////////////////
    auto ItConnected = mPipesConnected.begin();
    while (ItConnected != mPipesConnected.end()){
        if (ItConnected->second.first == 0){
            /// TODO: disconnect;
            ItConnected->second.second.second.close();
            ///
            dataAmiPipeAnswer answ;
            if(pOff.find(ItConnected->first)!=pOff.end())    answ.Type = dataAmiPipeAnswer::PipeOff;
            else                                            answ.Type = dataAmiPipeAnswer::PipeDisconnected;

            answ.iTickerID = ItConnected->second.second.first;
            queuePipeAnswers.Push(answ);
            //
            RemoveUtilityMapEntry(ItConnected->second.second.first, ItConnected->first);
            //
            auto ItNext = std::next(ItConnected);
            mPipesConnected.erase(ItConnected);
            ItConnected = ItNext;
        }
        else{
            ItConnected++;
        }
    }
    //
    auto ItHulted = mPipesHalted.begin();
    while (ItHulted != mPipesHalted.end()){
        if (ItHulted->second.first == 0){

            auto ItNext = std::next(ItHulted);
            mPipesHalted.erase(ItHulted);
            ItHulted = ItNext;
        }
        else{
            ItHulted++;
        }
    }
}
//-------------------------------------------------------------------------------------------------
void AmiPipeHolder::ReadConnectedPipes(BlockFreeQueue<dataFastLoadTask>                    &queueFastTasks,
                                       BlockFreeQueue<dataAmiPipeAnswer>                   &queuePipeAnswers,
                                       BlockFreeQueue<dataBuckgroundThreadAnswer>          &queueTrdAnswers)
{
    milliseconds tActivityCount;

    int iTickerID{0};

    int iBytesToRead{0};
    int iWriteStart{0};

    bool bInStream{false};


    BarTick b(0,0,0);
    BarTickMemcopier bM(b);



    auto ItConnected = mPipesConnected.begin();
    while (ItConnected != mPipesConnected.end()){

        iTickerID = ItConnected->second.second.first;
        dataFastLoadTask task(dataFastLoadTask::NewTicks);

        if (ItConnected->second.second.second.good()){

#ifdef _WIN32
            int filesize = iBlockMaxSize;
#else
            ItConnected->second.second.second.seekg(0,std::ios::end);
            int filesize = ItConnected->second.second.second.tellg();
            ItConnected->second.second.second.seekg(0, std::ios::cur);
#endif

//            int filesize = 1;

            if (filesize > 0){
                iWriteStart = mPointerToWrite[ItConnected->first];
                iBytesToRead = iWriteStart + (int)filesize < iBlockMaxSize ?
                            filesize : iBlockMaxSize - iWriteStart;
                //
                char *buff = mBuffer[ItConnected->first].data();
#ifdef _WIN32
                //Read(char * buff, size_t buffsize, size_t &bytesRead);
                if(ItConnected->second.second.second.read (buff + iWriteStart,iBytesToRead,filesize)){
                    mPointerToWrite[ItConnected->first] +=  filesize;
#else
                if(ItConnected->second.second.second.read(buff + iWriteStart,iBytesToRead)){
                    mPointerToWrite[ItConnected->first] +=  iBytesToRead;
#endif

                    ////-------
//                    if (iTicketID == 1)
//                    {
//                        if(!fileW.write(buff,iBytesToRead)){
//                            ThreadFreeCout pcout;
//                            pcout <<"Unsuccessful write operation\n";
//                        }
//                        else{
//                            ThreadFreeCout pcout;
//                            pcout <<"Writing buff {"<<iBytesToRead<<"}\n";
//                        }
//                    }
                    ////-------
                    bInStream = false;
                    int iReadStart = mPointerToRead[ItConnected->first];
                    int iBlockStart = 0;
                    int i = iReadStart;
                    unsigned long long longDate{0};
                    double volume{0};

                    double vO{0};
                    double vH{0};
                    double vL{0};
                    double vC{0};

                    task.vV.reserve(sizeof (b) * (1 + ((mPointerToWrite[ItConnected->first] - iReadStart)/48)));

                    while(i < mPointerToWrite[ItConnected->first]){
                        if(i >= 2 && !bInStream){
                            if(buff[i - 2] == 8 && buff[i - 1] == 0){
                                bInStream = true;
                                iBlockStart = 0;
                            }
                        }
                        if (bInStream && iBlockStart >= 48){
                            bInStream = false;
                            iReadStart = i - iBlockStart;
//                            if (bFirst && iReadStart != 2){
//                                ThreadFreeCout pcout;
//                                pcout <<"reading not from start: {"<<iReadStart<<"}\n";
//                            }

                            memcpy(&longDate,buff + iReadStart, 8);          iReadStart += 8;
                            memcpy(&vO,buff  + iReadStart, 8);       iReadStart += 8;
                            memcpy(&vH,buff  + iReadStart, 8);       iReadStart += 8;
                            memcpy(&vL,buff  + iReadStart, 8);       iReadStart += 8;
                            memcpy(&vC,buff  + iReadStart, 8);       iReadStart += 8;
                            memcpy(&volume,buff + iReadStart, 8);       iReadStart += 8;

                                                                                   // 13275568800 179 000 1
                            bM.Period() = t1970_01_01_04_00_00 + (longDate/10000000 - 11644488000);
                            bM.Close() = vC;
                            bM.Volume() = (long)volume;

                            if(b.Period() > t1971_01_01_00_00_00 &&
                               b.Period() < t2100_01_01_00_00_00
                                    ){

                                task.vV.push_back(b);

//                                if (iTickerID == 1){
//                                    ThreadFreeCout pcout;
//                                    pcout <<"{"<<threadfree_gmtime_to_str(&bM.Period())<<"} ";
//                                    pcout <<"{"<<b.Close()<<"} ";
//                                    pcout <<"{"<<volume<<"}\n";
//                                }
                            }
                            else
                            {
                                //TODO: dump!!!
                                ThreadFreeCout pcout;
                                pcout <<"error in time during import from pipe: "<<iTickerID<<"\n";
                            }

                        }
                        iBlockStart++;
                        i++;
                    }

                    if (iReadStart > 0){

                        int iDelta = mPointerToWrite[ItConnected->first] - iReadStart;

//                        {
//                            ThreadFreeCout pcout;
//                            pcout <<"iDelta: "<<iDelta<<"\n";
//                        }

                        memcpy(buff,buff + iReadStart, iDelta);
                        mPointerToWrite[ItConnected->first] -= iReadStart;
                        mPointerToRead[ItConnected->first] = 0;

                        task.iTickerID       = iTickerID;
                        task.lTask           = mTask[iTickerID];
                        task.llPackesCounter = mPacketsCounter[iTickerID]++;
                        queueFastTasks.Push(task);
                        conditionFastData.notify_one();


//                        {
//                            ThreadFreeCout pcout;
//                            pcout <<"mems: {"<<iTicketID<<"} write_pointer: {"<<mPointerToWrite[ItConnected->first]<<"}\n";
//                        }

                        tActivityCount = std::chrono::steady_clock::now() - mDtActivity[iTickerID];
                        if (tActivityCount > 1800ms){
                            mDtActivity[iTickerID] = std::chrono::steady_clock::now();
                            dataBuckgroundThreadAnswer dt(iTickerID,dataBuckgroundThreadAnswer::eAnswerType::LoadActivity,nullptr);
                            queueTrdAnswers.Push(dt);
                        }
                    }
                }
            }
            ++ItConnected;
        }
        else{

#ifdef _WIN32
            if (!ItConnected->second.second.second.good()){
#else
            if (!(ItConnected->second.second.second.rdstate() & std::ios_base::eofbit)){
                ThreadFreeCout pcout;
                pcout << "not good\n";

                if (ItConnected->second.second.second.rdstate() & std::ios_base::goodbit) { pcout << "stream state is goodbit\n";}
                if (ItConnected->second.second.second.rdstate() & std::ios_base::badbit) { pcout << "stream state is badbit\n";}
                if (ItConnected->second.second.second.rdstate() & std::ios_base::failbit) { pcout << "stream state is failbit\n";}

#endif
                try{
                    ItConnected->second.second.second.close();
                }
                catch (...) {;}
                //
                dataAmiPipeAnswer answ;
                answ.Type = dataAmiPipeAnswer::PipeDisconnected;
                answ.iTickerID = ItConnected->second.second.first;
                queuePipeAnswers.Push(answ);
                //
                RemoveUtilityMapEntry(ItConnected->second.second.first, ItConnected->first);
                //
                auto ItNext = std::next(ItConnected);
                mPipesConnected.erase(ItConnected);
                ItConnected = ItNext;
            }
            else{
                ++ItConnected;
            }
        }
    }

}
//-------------------------------------------------------------------------------------------------
//std::map<std::string,std::vector<char>> mBuffer;

//std::map<std::string,int> mPointerToWrite;
//std::map<std::string,int> mPointerToRead;

//std::map<int,std::chrono::time_point<std::chrono::steady_clock>> mDtActivity;
//std::map<int,std::time_t> mCurrentSecond;
//-------------------------------------------------------------------------------------------------
void AmiPipeHolder::AddUtilityMapEntry(int iTickerID, std::string sBind)
{
    std::unique_lock lk(mutUtility);

    long lTask = aiTaskCounter.load();
    while(!aiTaskCounter.compare_exchange_weak(lTask,lTask + 1)){;}


    mBuffer[sBind].resize(iBlockMaxSize);
    mPointerToWrite[sBind] = 0;
    mPointerToRead[sBind] = 0;

    mDtActivity[iTickerID] = std::chrono::steady_clock::now();
    mCurrentSecond[iTickerID] = 0;

    mTask[iTickerID] = lTask;
    mPacketsCounter[iTickerID] = 1;

}
//-------------------------------------------------------------------------------------------------
void AmiPipeHolder::RemoveUtilityMapEntry(int iTickerID, std::string sBind)
{
    std::unique_lock lk(mutUtility);

    auto ItBuff = mBuffer.find(sBind);
    if (ItBuff != mBuffer.end()){
        mBuffer.erase(ItBuff);
    }
    auto ItPW = mPointerToWrite.find(sBind);
    if (ItPW != mPointerToWrite.end()){
        mPointerToWrite.erase(ItPW);
    }
    auto ItPR = mPointerToRead.find(sBind);
    if (ItPR != mPointerToRead.end()){
        mPointerToRead.erase(ItPR);
    }
    auto ItAct = mDtActivity.find(iTickerID);
    if (ItAct != mDtActivity.end()){
        mDtActivity.erase(ItAct);
    }
    auto ItSec = mCurrentSecond.find(iTickerID);
    if (ItSec != mCurrentSecond.end()){
        mCurrentSecond.erase(ItSec);
    }

    auto ItTask = mTask.find(iTickerID);
    if (ItTask != mTask.end()){
        mTask.erase(ItTask);
    }

    auto ItPackets = mPacketsCounter.find(iTickerID);
    if (ItPackets != mPacketsCounter.end()){
        mPacketsCounter.erase(ItPackets);
    }
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------


