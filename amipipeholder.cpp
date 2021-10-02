#include "amipipeholder.h"

#include<filesystem>
#include<iostream>
#include<regex>
#include<iomanip>
#include<fstream>
#include<ostream>
#include<chrono>
#include<sstream>
#include "utilites.h"
#include "storage.h"

#include "threadfreelocaltime.h"



//using namespace std::chrono_literals;
using seconds=std::chrono::duration<double>;
using milliseconds=std::chrono::duration<double,
    std::ratio_multiply<seconds::period,std::milli>
    >;


//-------------------------------------------------------------------------------------------------
AmiPipeHolder::AmiPipeHolder():
    iMode{Byte_Nonblocking},
    //iMode{Message_Nonblocking},
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
std::string AmiPipeHolder::getSignFromBind(std::string sBind)
{
    std::stringstream ssRegSign;
    ssRegSign <<"(?:(?![.]).)+$";
    const std::regex reAmiSign {ssRegSign.str()};
    const auto ItSign = std::sregex_token_iterator(sBind.begin(),sBind.end(),reAmiSign);

    if (ItSign != std::sregex_token_iterator()){
        return *ItSign;
    }
    else{
        return "";
    }
}
//-------------------------------------------------------------------------------------------------
std::string AmiPipeHolder::getNameFromRaw(std::string sRaw)
{
    std::stringstream ssRegName;
    ssRegName <<"^[^\[]*";
    const std::regex reAmiSign {ssRegName.str()};
    const auto ItSign = std::sregex_token_iterator(sRaw.begin(),sRaw.end(),reAmiSign);

    if (ItSign != std::sregex_token_iterator()){
        return *ItSign;
    }
    else{
        return "";
    }
}
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
#else
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
//                        const auto ItSign = std::sregex_token_iterator(sBind.begin(),sBind.end(),reAmiSign);
//                        sSign = "";
//                        if (ItSign != std::sregex_token_iterator()){
//                            sSign = *ItSign;
//                        }
                        sSign = getSignFromBind(sBind);
                        ssFilePath.str("");
                        ssFilePath.clear();
                        ssFilePath <<sPipeDir<<(*ItPipe);
                        //ssFilePath <<fl.path().filename().string();


                        mRet[sBind] = {0,{*ItPipe,sSign,ssFilePath.str(),0,0,sSign}};
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
                                       1,
                                       std::get<5>(mPipes[sTmp].second)
                                      };
            }
            else{
                mPipes[sTmp].second = {std::get<0>(mPipes[sTmp].second),
                                       std::get<1>(mPipes[sTmp].second),
                                       std::get<2>(mPipes[sTmp].second),
                                       t.TickerID(),
                                       2,
                                       std::get<5>(mPipes[sTmp].second)
                                      };
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
        if (this_thread_flagInterrup.isSet()){
            return;
        }
        auto ItHalted (mPipesHalted.find(p.first));
        if ( ItHalted == mPipesHalted.end()){
            auto ItConnected (mPipesConnected.find(p.first));
            if ( ItConnected == mPipesConnected.end()){
                bool bOpend{false};
                try{
#ifdef _WIN32
                    mPipesConnected[p.first].first  = 1;
                    mPipesConnected[p.first].second = {std::get<3>(p.second.second),{}};
                    mPipesConnected[p.first].second.second.setPipePath(std::get<2>(p.second.second));

                    if (iMode == Byte_Nonblocking){
                        mPipesConnected[p.first].second.second.setMode(Win32NamedPipe::ePipeMode_type::Byte_Nonblocking);
                    }
                    else{
                        mPipesConnected[p.first].second.second.setMode(Win32NamedPipe::ePipeMode_type::Message_Nonblocking);
                    }
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
                        answ.SetType(dataAmiPipeAnswer::PipeConnected);
                        answ.SetTickerID(std::get<3>(p.second.second));
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
                    answ.SetType(dataAmiPipeAnswer::PipeHalted);
                    answ.SetTickerID(std::get<3>(p.second.second));
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
        if (this_thread_flagInterrup.isSet()){
            return;
        }
    }
    //////////////////////////////////////////////////////////
    auto ItConnected = mPipesConnected.begin();
    while (ItConnected != mPipesConnected.end()){
        if (ItConnected->second.first == 0){
            ///
            ItConnected->second.second.second.close();
            ///
            dataAmiPipeAnswer answ;
            if(pOff.find(ItConnected->first)!=pOff.end())    answ.SetType(dataAmiPipeAnswer::PipeOff);
            else                                             answ.SetType(dataAmiPipeAnswer::PipeDisconnected);

            answ.SetTickerID(ItConnected->second.second.first);
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
                                       BlockFreeQueue<dataBuckgroundThreadAnswer>          &queueTrdAnswers,
                                       bool bCheckMode,
                                       int &BytesRead,
                                       bool & bWasFullBuffers)
{
#ifdef _WIN32
    if (iMode == AmiPipeHolder::ePipeMode_type::Byte_Nonblocking){
        return ReadConnectedPipes_bytemode_win32(queueFastTasks,queuePipeAnswers,queueTrdAnswers,bCheckMode,BytesRead,bWasFullBuffers);
    }
    else{
        return ReadConnectedPipes_bytemode_win32(queueFastTasks,queuePipeAnswers,queueTrdAnswers,bCheckMode,BytesRead,bWasFullBuffers);
        //return ReadConnectedPipes_messagemode_win32(queueFastTasks,queuePipeAnswers,queueTrdAnswers,BytesRead,bWasFullBuffers);
    }
#else
    if (iMode == AmiPipeHolder::ePipeMode_type::Byte_Nonblocking){
        return ReadConnectedPipes_bytemode_linux(queueFastTasks,queuePipeAnswers,queueTrdAnswers,bCheckMode,BytesRead,bWasFullBuffers);
    }
    else{
        return ReadConnectedPipes_messagemode_linux(queueFastTasks,queuePipeAnswers,queueTrdAnswers,bCheckMode,BytesRead,bWasFullBuffers);
    }
#endif
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

    mPaperName[sBind] = "";//getSignFromBind(sBind);
    mCheckTime[sBind] = std::chrono::steady_clock::now();

}
//-------------------------------------------------------------------------------------------------
void AmiPipeHolder::RemoveUtilityMapEntry(int iTickerID, std::string sBind, bool bCheckMode)
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

    if (!(bCheckMode && iTickerID == 0)){
        auto ItTask = mTask.find(iTickerID);
        if (ItTask != mTask.end()){
            mTask.erase(ItTask);
        }

        auto ItPackets = mPacketsCounter.find(iTickerID);
        if (ItPackets != mPacketsCounter.end()){
            mPacketsCounter.erase(ItPackets);
        }
    }

    auto ItName = mPaperName.find(sBind);
    if (ItName != mPaperName.end()){
        mPaperName.erase(ItName);
    }

    auto ItCheckTime = mCheckTime.find(sBind);
    if (ItCheckTime != mCheckTime.end()){
        mCheckTime.erase(ItCheckTime);
    }
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
#ifdef _WIN32
//-------------------------------------------------------------------------------------------------
int AmiPipeHolder::ProcessReceivedBuffer(BlockFreeQueue<dataFastLoadTask>                   &queueFastTasks,
                                         BlockFreeQueue<dataAmiPipeAnswer>                  &queuePipeAnswers,
                                         BlockFreeQueue<dataBuckgroundThreadAnswer>         &queueTrdAnswers,
                                         const int iTickerID,
                                         const std::string strBind,
                                         bool bCheckMode,
                                         dataFastLoadTask &task,
                                         char *buff,
                                         int &ptrToRead,
                                         int &ptrToWrite
                                         )
{
    assert(iTickerID  > 0 || bCheckMode);
    assert(ptrToRead  >= 0);
    assert(ptrToWrite >= 0);
    assert(ptrToWrite >= ptrToRead);

    BarTick b(0,0,0);
    BarTickMemcopier bM(b);
    unsigned long long longDate{0};
    double volume{0};
    double vC{0};
    std::string strName;

    bool bInStream{false};
    enum eStream:int {s0,s1,s2,sN,s6,s7,s8};
    eStream streamNum{s0};
    int16_t iStreamNameLen;

    int iBlockStart{0};
    int iReadStart{0};

    const int iExpectedBuff (sizeof (b) * (1 + ((ptrToWrite - ptrToRead)/48)));
    if (task.vV.capacity() - task.vV.size() < iExpectedBuff){
        task.vV.reserve(task.vV.size() + iExpectedBuff);
    }


    int i = ptrToRead;
    while(i < ptrToWrite){
        if(i >= 2 && !bInStream){
            if(buff[i - 2] == 0 && buff[i - 1] == 0){ iReadStart = i;}
            if(buff[i - 2] == 1 && buff[i - 1] == 0){ bInStream = true; streamNum = s1;}
            if(buff[i - 2] == 2 && buff[i - 1] == 0){ bInStream = true; streamNum = s2;}
            if(buff[i - 2] == 6 && buff[i - 1] == 0){ bInStream = true; streamNum = s6;}
            if(buff[i - 2] == 7 && buff[i - 1] == 0){ bInStream = true; streamNum = s7;}
            if(buff[i - 2] == 8 && buff[i - 1] == 0){ bInStream = true; streamNum = s8;}
            iBlockStart = 0;
        }
        //----------------------------------------------------------------------------
        if (bInStream && streamNum == s1 && iBlockStart >= 0){
            bInStream = false;
            iReadStart = i - iBlockStart;
        }
        else if (bInStream && streamNum == s2 && iBlockStart >= 4){
            bInStream = false;
            iReadStart = i - iBlockStart;
            iReadStart+=2;
            memcpy(&iStreamNameLen,buff + iReadStart, 2);     iReadStart += 2;

            if(iStreamNameLen > 0){
                bInStream = true;
                streamNum = sN;
                iBlockStart = 0;
            }
        }
        else if (bInStream && streamNum == sN && iBlockStart >= iStreamNameLen){

            bInStream = false;
            iReadStart = i - iBlockStart;
            strName.resize(iStreamNameLen+1);
            memcpy(strName.data(),buff + iReadStart, iStreamNameLen);     iReadStart += iStreamNameLen;

            if (strName[iStreamNameLen] == '\0'){
                strName.resize(iStreamNameLen);
            }
            else{
                strName[iStreamNameLen+1] = '\0';
            }

            mPaperName[strBind] = getNameFromRaw(QString::fromLocal8Bit(strName.data(),(int)strName.size()).toStdString());
        }
        else if (bInStream && (streamNum == s6 || streamNum == s7) && iBlockStart >= 4){
            bInStream = false;
            iReadStart = i - iBlockStart;
            iReadStart += 4;
        }
        else if (bInStream && iBlockStart >= 48){
            bInStream = false;
            iReadStart = i - iBlockStart;

            memcpy(&longDate,buff + iReadStart, 8);     iReadStart += 8;
            /*memcpy(&vO,buff  + iReadStart, 8);*/       iReadStart += 8;
            /*memcpy(&vH,buff  + iReadStart, 8);*/       iReadStart += 8;
            /*memcpy(&vL,buff  + iReadStart, 8);*/       iReadStart += 8;
            memcpy(&vC,buff  + iReadStart, 8);          iReadStart += 8;
            memcpy(&volume,buff + iReadStart, 8);       iReadStart += 8;
                                                                   // 13275568800 179 000 1
            bM.Period() = t1970_01_01_04_00_00 + (longDate/10000000 - 11644488000);
            bM.Close() = vC;
            bM.Volume() = (long)volume;

            if(b.Period() > t1971_01_01_00_00_00 &&
               b.Period() < t2100_01_01_00_00_00){
                // writing tick to vector
                task.vV.push_back(b);
            }
            else{
                std::stringstream ss;
                ss <<"error in time during import from pipe: "<<iTickerID<<"\n";
                SendToErrorLog(queuePipeAnswers, iTickerID,ss.str());
            }
        }
        ++i;
        ++iBlockStart;
    }

    if(iReadStart > 0 && !task.vV.empty()){
        ptrToRead = iReadStart;

        int iDelta = ptrToWrite - iReadStart;

        memcpy(buff,buff + iReadStart, iDelta);
        ptrToWrite -= iReadStart;
        ptrToRead = 0;

        if (!bCheckMode){
            task.iTickerID       = iTickerID;
            task.lTask           = mTask[iTickerID];
            task.llPackesCounter = mPacketsCounter[iTickerID]++;
            queueFastTasks.Push(task);
            conditionFastData.notify_one();


            milliseconds tActivityCount = std::chrono::steady_clock::now() - mDtActivity[iTickerID];
            if (tActivityCount > milliseconds(1800)/*ms*/){
                mDtActivity[iTickerID] = std::chrono::steady_clock::now();
                dataBuckgroundThreadAnswer dt(iTickerID,dataBuckgroundThreadAnswer::eAnswerType::LoadActivity,nullptr);
                queueTrdAnswers.Push(dt);
            }
        }
    }

    if(ptrToWrite + 48 > iBlockMaxSize/2){

        memcpy(buff,buff + ptrToWrite - 48, iBlockMaxSize/2 - ptrToWrite + 48);
        ptrToRead = 0;
        ptrToWrite = 48;
    }

    return 0;
}
//-------------------------------------------------------------------------------------------------
bool AmiPipeHolder::ReadPipe_bytemode_win32(Win32NamedPipe &pip,
                                            bool bCheckMode,
                                            const int iTickerID,
                                            const std::string strBind,
                                            BlockFreeQueue<dataFastLoadTask>        &queueFastTasks,
                                            BlockFreeQueue<dataAmiPipeAnswer>                   &queuePipeAnswers,
                                            BlockFreeQueue<dataBuckgroundThreadAnswer>          &queueTrdAnswers,
                                            int & BytesRead,
                                            bool & bWasFullBuffers)
{

    dataFastLoadTask task(dataFastLoadTask::NewTicks);

    int iWriteStart{0};
    int iReadCount{0};
    int iBytesToRead{0};
    static const int iMaxReadCount{30};
    bool bLastReadMoreData{false};
    int iTotalBytesRead{0};
    bool bSuccessfullRead{false};

    iWriteStart = mPointerToWrite[strBind];
    //iReadStart = mPointerToRead[strBind];
    iBytesToRead = iBlockMaxSize  - iWriteStart;
    iReadCount = 0;
    iTotalBytesRead = 0;
    char *buff = mBuffer[strBind].data();
    //bLastReadMoreData = true;

    while(true){
        if (pip.read (buff + iWriteStart,iBytesToRead,iTotalBytesRead) || GetLastError() == ERROR_MORE_DATA){

            bSuccessfullRead = true;
            mPointerToWrite[strBind] = iWriteStart + iTotalBytesRead;
            BytesRead += iTotalBytesRead;
            if (GetLastError() == ERROR_MORE_DATA)  {bLastReadMoreData = true;  bWasFullBuffers = true;}
            else                                    {bLastReadMoreData = false;}
            iReadCount++;
            ////
            ProcessReceivedBuffer(queueFastTasks,queuePipeAnswers,queueTrdAnswers,
                        iTickerID,strBind,bCheckMode,task,buff,mPointerToRead[strBind],mPointerToWrite[strBind]);

            iWriteStart = mPointerToWrite[strBind];
            iBytesToRead = iBlockMaxSize  - iWriteStart;
        }
        else{
            if (GetLastError() == ERROR_NO_DATA)  {bSuccessfullRead = true;}
        }
        if (!bLastReadMoreData || iReadCount >= iMaxReadCount) break;
    }

    return bSuccessfullRead;
}
//-------------------------------------------------------------------------------------------------
void AmiPipeHolder::ReadConnectedPipes_bytemode_win32(BlockFreeQueue<dataFastLoadTask>     &queueFastTasks,
                            BlockFreeQueue<dataAmiPipeAnswer>                   &queuePipeAnswers,
                            BlockFreeQueue<dataBuckgroundThreadAnswer>          &queueTrdAnswers,
                            bool bCheckMode,
                            int & BytesRead,
                            bool & bWasFullBuffers
                            )
{


    bWasFullBuffers = false;
    BytesRead = 0;

    int iTickerID{0};
    std::string strBind;

    bool bSuccessfullRead{false};
    milliseconds tActivityCount;

    ////////////////////////////////////////////////
    internal_pipes_type &mMap = !bCheckMode  ? mPipesConnected : mPipesFree;
    ////////////////////////////////////////////////

    auto ItConnected = mMap.begin();

    while (ItConnected != mMap.end()){


        iTickerID   = ItConnected->second.second.first;
        strBind     = ItConnected->first;
        Win32NamedPipe &pip = ItConnected->second.second.second;
        bSuccessfullRead = false;

        if (pip.good()){
            bSuccessfullRead = ReadPipe_bytemode_win32(pip,bCheckMode,iTickerID,strBind,
                                                       queueFastTasks,
                                                       queuePipeAnswers,
                                                       queueTrdAnswers,
                                                       BytesRead,
                                                       bWasFullBuffers);
            if(bCheckMode){
                tActivityCount = std::chrono::steady_clock::now() - mCheckTime[strBind];
                if (mPaperName[strBind].size() > 0 || tActivityCount > milliseconds(2000)){//2000ms

                    dataAmiPipeAnswer answ;
                    answ.SetType(dataAmiPipeAnswer::AskNameAnswer);
                    answ.SetTickerID(iTickerID);
                    answ.SetBind(strBind);

                    if (mPaperName[strBind].size() > 0) answ.SetPipeName(mPaperName[strBind]);
                    else                                answ.SetPipeName(getSignFromBind(strBind));
                    queuePipeAnswers.Push(answ);

                    try{
                        ItConnected->second.second.second.close();
                    }
                    catch (...) {;}

                    RemoveUtilityMapEntry(iTickerID, strBind);
                    //
                    auto ItNext = std::next(ItConnected);
                    mMap.erase(ItConnected);
                    ItConnected = ItNext;
                    continue;
                }
            }
        }
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        if(!bSuccessfullRead){

            if (GetLastError() != 233){ // server close connection
                /////////////////////////////////////////////////////////////////////
                std::stringstream ss;
                ss <<"-----------------------------------------------------\n";
                ss <<"halt dump:\n";
                if (mBuffer.find(strBind) != mBuffer.end()){ss <<"mBuffer[strBind].size(): "<<mBuffer[strBind].size()<<"\n";}
                else                                        {ss <<"mBuffer[strBind] not found!!!\n";}
                ss <<"strBind: {"<<strBind<<"}\n";
                SendToErrorLog(queuePipeAnswers, iTickerID, ss.str());
                /////////////////////////////////////////////////////////////////////
            }
            else{
                std::stringstream ss;
                ss <<"server close connection on {"<<iTickerID<<"} {"<<strBind<<"}";
                SendToErrorLog(queuePipeAnswers, iTickerID, ss.str());
            }
            try{
                ItConnected->second.second.second.close();
            }
            catch (...) {;}
            //
            //
            if (!bCheckMode){
                dataAmiPipeAnswer answ;
                answ.SetType(dataAmiPipeAnswer::PipeDisconnected);
                answ.SetTickerID(ItConnected->second.second.first);
                queuePipeAnswers.Push(answ);
            }
            else{
                dataAmiPipeAnswer answ;
                answ.SetType(dataAmiPipeAnswer::AskNameAnswer);
                answ.SetTickerID(ItConnected->second.second.first);
                answ.SetBind(ItConnected->first);
                answ.SetPipeName(getSignFromBind(ItConnected->first)+"_t2");
                queuePipeAnswers.Push(answ);
            }
            //
            RemoveUtilityMapEntry(iTickerID, strBind);
            //
            auto ItNext = std::next(ItConnected);
            mMap.erase(ItConnected);
            ItConnected = ItNext;
            /////////////////////////////////////////////////////////////////////
        }
        else{
            ++ItConnected;
        }
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    }
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
//-------------------------------------------------------------------------------------------------
void AmiPipeHolder::ReadConnectedPipes_messagemode_win32(BlockFreeQueue<dataFastLoadTask>        &/*queueFastTasks*/,
                            BlockFreeQueue<dataAmiPipeAnswer>                   &/*queuePipeAnswers*/,
                            BlockFreeQueue<dataBuckgroundThreadAnswer>          &/*queueTrdAnswers*/,
                            bool /*bCheckMode*/,
                            int & BytesRead,
                            bool & bWasFullBuffers
                            )
{
    {
        BytesRead = 0;
        bWasFullBuffers = false;
        return;
    }
}
//-------------------------------------------------------------------------------------------------
#else
//-------------------------------------------------------------------------------------------------
void AmiPipeHolder::ReadConnectedPipes_bytemode_linux(BlockFreeQueue<dataFastLoadTask>     &queueFastTasks,
                                                      BlockFreeQueue<dataAmiPipeAnswer>                   &queuePipeAnswers,
                                                      BlockFreeQueue<dataBuckgroundThreadAnswer>          &queueTrdAnswers,
                                                      bool /*bCheckMode*/,
                                                      int & BytesRead,
                                                      bool & bWasFullBuffers
                            )
{
    milliseconds tActivityCount;

    bWasFullBuffers = false;
    BytesRead = 0;

    int iTickerID{0};
    std::string strBind;

    int iBytesToRead{0};
    int iTotalBytesRead{0};
    int iWriteStart{0};
    //int iReadCount{0};

    //const int iMaxReadCounts {30};
    //const int iMaxReadCounts {1};
    //bool bLastReadMoreData;

    bool bInStream{false};


    BarTick b(0,0,0);
    BarTickMemcopier bM(b);

    bool bSuccessfullRead{false};


    auto ItConnected = mPipesConnected.begin();
    while (ItConnected != mPipesConnected.end()){


        iTickerID   = ItConnected->second.second.first;
        strBind     = ItConnected->first;

        dataFastLoadTask task(dataFastLoadTask::NewTicks);


        bSuccessfullRead = true;
        if (ItConnected->second.second.second.good()){

#ifdef _WIN32
            if(true){
#else
            int filesize{0};
            ItConnected->second.second.second.seekg(0,std::ios::end);
            filesize = ItConnected->second.second.second.tellg();
            ItConnected->second.second.second.seekg(0, std::ios::cur);
            if (filesize > 0){
#endif

                iWriteStart = mPointerToWrite[strBind];
                iBytesToRead = iBlockMaxSize  - iWriteStart;

//                iBytesToRead = iWriteStart + (int)filesize < iBlockMaxSize ?
//                            filesize : iBlockMaxSize - iWriteStart;
                //
                char *buff = mBuffer[strBind].data();
#ifdef _WIN32

                iReadCount = 0;
                iTotalBytesRead = 0;
                bLastReadMoreData = true;

                while( bLastReadMoreData && iReadCount < iMaxReadCounts){

                    bSuccessfullRead = ItConnected->second.second.second.read (buff + iWriteStart,iBytesToRead,iTotalBytesRead);

                    if (GetLastError() == ERROR_MORE_DATA)  {bLastReadMoreData = true;}
                    else                                    {bLastReadMoreData = false;}

                    if(!bSuccessfullRead){
                        break;
                    }

                    iReadCount++;

                    mPointerToWrite[strBind] +=  iTotalBytesRead;
                    BytesRead += iTotalBytesRead;

                    if (bLastReadMoreData) bWasFullBuffers = true;
#else
                int iToRead = iBytesToRead;
                if(ItConnected->second.second.second.read(buff + iWriteStart,iBytesToRead)){
                    iTotalBytesRead = iBytesToRead;
                    bSuccessfullRead = true;
                    mPointerToWrite[strBind] +=  iTotalBytesRead;
                    BytesRead += iBytesToRead;
                    if (iToRead == iBytesToRead) bWasFullBuffers = true;
#endif

                    ////-------
                    bInStream = false;
                    int iReadStart = mPointerToRead[strBind];
                    int iBlockStart = 0;
                    int i = iReadStart;
                    int iPacketStart = iReadStart;
                    unsigned long long longDate{0};
                    double volume{0};
                    double vC{0};

                    task.vV.reserve(sizeof (b) * (1 + ((mPointerToWrite[strBind] - iReadStart)/48)));

                    while(i < mPointerToWrite[strBind]){
                        if(i >= 2 && !bInStream){
                            if(buff[i - 2] == 8 && buff[i - 1] == 0){
                                bInStream = true;
                                iBlockStart = 0;
                            }
                        }
                        if (bInStream && iBlockStart >= 48){
                            bInStream = false;
                            iReadStart = i - iBlockStart;

                            memcpy(&longDate,buff + iReadStart, 8);     iReadStart += 8;
                            /*memcpy(&vO,buff  + iReadStart, 8);*/       iReadStart += 8;
                            /*memcpy(&vH,buff  + iReadStart, 8);*/       iReadStart += 8;
                            /*memcpy(&vL,buff  + iReadStart, 8);*/       iReadStart += 8;
                            memcpy(&vC,buff  + iReadStart, 8);          iReadStart += 8;
                            memcpy(&volume,buff + iReadStart, 8);       iReadStart += 8;

                                                                                   // 13275568800 179 000 1
                            bM.Period() = t1970_01_01_04_00_00 + (longDate/10000000 - 11644488000);
                            bM.Close() = vC;
                            bM.Volume() = (long)volume;

                            if(b.Period() > t1971_01_01_00_00_00 &&
                               b.Period() < t2100_01_01_00_00_00){
                                // writing tick to vector
                                task.vV.push_back(b);
                            }
                            else
                            {
                                //TODO: this is dump!!! delete on release
                                ThreadFreeCout pcout;
                                pcout <<"error in time during import from pipe: "<<iTickerID<<"\n";

                                ////-----
                                std::stringstream ss;
                                ss <<"errdump_" << iTickerID <<".txt";
                                std::ofstream fileW(ss.str(),std::ios_base::app);
                                if (fileW.good()){
                                    fileW <<"====================================================================\n";
                                    fileW.close();
                                    //
                                    std::ofstream fileW(ss.str(),std::ios_base::app | std::ios_base::binary);
                                    if (fileW.good()){
                                        if(!fileW.write(buff + iPacketStart,i - iPacketStart)){
                                            ThreadFreeCout pcout;
                                            pcout <<"Unsuccessful write buff dump operation\n";
                                        }
                                        else{
                                            ThreadFreeCout pcout;
                                            pcout <<"Writing buff dump {"<<iBytesToRead<<"}\n";
                                        }
                                    }
                                }
                            }
                        }
                        iBlockStart++;
                        i++;
                    }

                    if (iReadStart > 0){

                        int iDelta = mPointerToWrite[strBind] - iReadStart;


                        memcpy(buff,buff + iReadStart, iDelta);
                        mPointerToWrite[strBind] -= iReadStart;
                        mPointerToRead[strBind] = 0;

                        task.iTickerID       = iTickerID;
                        task.lTask           = mTask[iTickerID];
                        task.llPackesCounter = mPacketsCounter[iTickerID]++;
                        queueFastTasks.Push(task);
                        conditionFastData.notify_one();

                        tActivityCount = std::chrono::steady_clock::now() - mDtActivity[iTickerID];
                        if (tActivityCount > milliseconds(1800)){//1800ms
                            mDtActivity[iTickerID] = std::chrono::steady_clock::now();
                            dataBuckgroundThreadAnswer dt(iTickerID,dataBuckgroundThreadAnswer::eAnswerType::LoadActivity,nullptr);
                            queueTrdAnswers.Push(dt);
                        }
                    }
                    if(mPointerToWrite[strBind] + 48 > iBlockMaxSize){
                        //TODO: this is dump!!! delete on release
                        ThreadFreeCout pcout;
                        pcout <<"Buffer overflow on pipe: "<<iTickerID<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";

                        ////-----
                        std::stringstream ss;
                        ss <<"overlowdump_" << iTickerID <<".txt";
                        std::ofstream fileW(ss.str(),std::ios_base::app);
                        if (fileW.good()){
                            fileW <<"====================================================================\n";
                            fileW.close();
                            //
                            std::ofstream fileW(ss.str(),std::ios_base::app | std::ios_base::binary);
                            if (fileW.good()){
                                if(!fileW.write(buff,mPointerToWrite[strBind])){
                                    ThreadFreeCout pcout;
                                    pcout <<"Unsuccessful write buff dump operation\n";
                                }
                                else{
                                    ThreadFreeCout pcout;
                                    pcout <<"Writing buff dump {"<<iBytesToRead<<"}\n";
                                }
                            }
                        }
                        //-----------------------------------------------------------------------------
                        mPointerToWrite[strBind] = 0;
                        mPointerToRead[strBind] = 0;
                    }
                    iWriteStart = mPointerToWrite[strBind];
                    iBytesToRead = iBlockMaxSize  - iWriteStart;
                }
            }
        }
        //// looper
        if (!bSuccessfullRead){
            {
                // (bSuccessfullRead = ItConnected->second.second.second.read (buff + iWriteStart,iBytesToRead,filesize))){
                ThreadFreeCout pcout;
                pcout <<"-----------------------------------------------------\n";
                pcout <<"halt dump:\n";
                if (mBuffer.find(strBind) != mBuffer.end()){
                    ;
                    pcout <<"mBuffer[strBind].size(): "<<mBuffer[strBind].size()<<"\n";
                }
                else{
                    pcout <<"mBuffer[strBind] not found!!!\n";
                }
                pcout <<"iWriteStart: "<<iWriteStart<<"\n";
                pcout <<"iBytesToRead: "<<iBytesToRead<<"\n";
                pcout <<"iTotalBytesRead: "<<iTotalBytesRead<<"\n";
                pcout <<"-----------------------------------------------------\n";

            }
                try{
                    ItConnected->second.second.second.close();
                }
                catch (...) {;}
                //
                dataAmiPipeAnswer answ;
                answ.SetType(dataAmiPipeAnswer::PipeDisconnected);
                answ.SetTickerID(ItConnected->second.second.first);
                queuePipeAnswers.Push(answ);
                //
                RemoveUtilityMapEntry(iTickerID, strBind);
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

//-------------------------------------------------------------------------------------------------
void AmiPipeHolder::ReadConnectedPipes_messagemode_linux(BlockFreeQueue<dataFastLoadTask>        &/*queueFastTasks*/,
                                                         BlockFreeQueue<dataAmiPipeAnswer>                   &/*queuePipeAnswers*/,
                                                         BlockFreeQueue<dataBuckgroundThreadAnswer>          &/*queueTrdAnswers*/,
                                                         bool /*bCheckMode*/,
                                                         int & BytesRead,
                                                         bool & bWasFullBuffers
                            )
{
    BytesRead = 0;
    bWasFullBuffers = false;
    return;
}
#endif
//-------------------------------------------------------------------------------------------------
void AmiPipeHolder::SendToLog      (BlockFreeQueue<dataAmiPipeAnswer> &queuePipeAnswers, const int iTickerID, const std::string &s)
{
    dataAmiPipeAnswer answ;
    answ.SetType(dataAmiPipeAnswer::TextMessage);
    answ.SetTickerID(iTickerID);
    answ.SetTextInfo(s);
    queuePipeAnswers.Push(answ);

    ThreadFreeCout pcout;
    pcout <<s<<"\n";
}
//-------------------------------------------------------------------------------------------------
void AmiPipeHolder::SendToErrorLog (BlockFreeQueue<dataAmiPipeAnswer> &queuePipeAnswers, const int iTickerID, const std::string &s)
{
    dataAmiPipeAnswer answ;
    answ.SetType(dataAmiPipeAnswer::ErrMessage);
    answ.SetTickerID(iTickerID);
    answ.SetErrString(s);
    queuePipeAnswers.Push(answ);

    ThreadFreeCout pcout;
    pcout <<s<<"\n";
}
//-------------------------------------------------------------------------------------------------
void AmiPipeHolder::dumpToFile     (BlockFreeQueue<dataAmiPipeAnswer> &queuePipeAnswers,  const int iTickerID, const std::string &sFileName, const  char * cBuff, const size_t bytes, const int iReadStart, const bool bWriteHeader)
{
    std::filesystem::path pathFile = std::filesystem::absolute(pathCurr/sFileName);

    if (bWriteHeader){
        std::ofstream fileW(pathFile,std::ios_base::app);
        if (fileW.good()){

                std::time_t t = std::chrono::system_clock::to_time_t( std::chrono::system_clock::now());
                fileW <<"\n====================================================================\n";
                fileW << threadfree_gmtime_to_str(&t)<<"\n";
                fileW << "iReadStart: "<< iReadStart<<"\n";
                fileW <<"====================================================================\n";
                fileW.close();

        }
        else{
            std::stringstream ss;
            ss <<"Unsuccessful write buff dump operation {"<<iTickerID<<"} FileName: {"<<sFileName<<"}";
            SendToErrorLog(queuePipeAnswers,iTickerID,ss.str());
            return;
        }
    }
    //
    std::ofstream fileW(pathFile,std::ios_base::app | std::ios_base::binary);
    if (fileW.good()){
        if(fileW.write(cBuff,bytes)){
//                std::stringstream ss;
//                ss <<"Writing buff dump {"<<iTickerID<<"} bites: {"<<bytes<<"} FileName: {"<<sFileName<<"}";
//                SendToLog(queuePipeAnswers,iTickerID,ss.str());
        }
        else{
            std::stringstream ss;
            ss <<"Unsuccessful write buff dump operation {"<<iTickerID<<"} FileName: {"<<sFileName<<"}";
            SendToErrorLog(queuePipeAnswers,iTickerID,ss.str());
        }
    }
    else{
        std::stringstream ss;
        ss <<"Unsuccessful write buff dump operation {"<<iTickerID<<"} FileName: {"<<sFileName<<"}";
        SendToErrorLog(queuePipeAnswers,iTickerID,ss.str());
    }
}
//-------------------------------------------------------------------------------------------------
void AmiPipeHolder::AskPipesNames(dataAmiPipeTask::pipes_type &pFree, BlockFreeQueue<dataAmiPipeAnswer> & queuePipeAnswers)
{
    //mFreePipesAsked
    for (const auto &p:pFree){
        if (this_thread_flagInterrup.isSet()){
            return;
        }
        if (mPipesFree.find(p.first) == mPipesFree.end()){
            bool bOpend{false};
            try{
#ifdef _WIN32
                mPipesFree[p.first].first  = 1;
                mPipesFree[p.first].second = {std::get<3>(p.second.second),{}};
                mPipesFree[p.first].second.second.setPipePath(std::get<2>(p.second.second));

                if (iMode == Byte_Nonblocking){
                    mPipesFree[p.first].second.second.setMode(Win32NamedPipe::ePipeMode_type::Byte_Nonblocking);
                }
                else{
                    mPipesFree[p.first].second.second.setMode(Win32NamedPipe::ePipeMode_type::Message_Nonblocking);
                }
                mPipesFree[p.first].second.second.open();

                if (mPipesFree[p.first].second.second.good()){
#else
                std::ifstream file(std::get<2>(p.second.second),std::ios::in);
                if (file.good()){
                    mPipesFree[p.first].second = {std::get<3>(p.second.second),
                                                        std::move(file)};
#endif
                    AddUtilityMapEntry(std::get<3>(p.second.second), p.first);
                    //
                    bOpend = true;
                }
#ifdef _WIN32
                else{
                    auto ItD (mPipesFree.find(p.first));
                    if (ItD != mPipesFree.end()){
                        mPipesFree.erase(ItD);
                    }
                }
#endif
            }
            catch(std::exception &){
                ;
            }
            if (!bOpend){
                //
                dataAmiPipeAnswer answ;
                answ.SetType(dataAmiPipeAnswer::AskNameAnswer);
                answ.SetTickerID(std::get<3>(p.second.second));
                answ.SetBind(p.first);
                answ.SetPipeName(std::get<1>(p.second.second));
                queuePipeAnswers.Push(answ);
            }

        }
    }
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
