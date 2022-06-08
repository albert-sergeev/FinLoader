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
///////////////////////////////////////////////////////////////////////////////////////
/// \brief Constructor
///
AmiPipeHolder::AmiPipeHolder():
    iMode{Byte_Nonblocking},
    //iMode{Message_Nonblocking},
    aiTaskCounter{1}
{
    std::call_once(AmiPipeHolder_call_once_flag,&AmiPipeHolder::initStartConst,this);


}
//-------------------------------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////////////
/// \brief Procedure to init static constants to use during flow parsing
///
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
///////////////////////////////////////////////////////////////////////////////////////
/// \brief Parses system pipe name and returns ticker name
/// \param sBind    - raw text of system pipe name (normaly contains ticker name)
/// \return         - name of ticker
///
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
///////////////////////////////////////////////////////////////////////////////////////
/// \brief Parses raw data packet and return company name
/// \param sRaw     - data of raw packet wich contains full company name
/// \return         - company name
///
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
///////////////////////////////////////////////////////////////////////////////////////
/// \brief Scan the system for AmiBroker format pipes.
/// fully implemented only on windows platform
/// \return container with pipes
///
dataAmiPipeTask::pipes_type AmiPipeHolder::ScanActivePipes()
{
    //--------------------------------------------------
    // declarations:
    dataAmiPipeTask::pipes_type mRet;

    //--------------------------------------------------
    // standart path to pipes
    std::string sPipeDir = "\\\\.\\pipe\\";
    std::stringstream ssFilePath("");


    //std::filesystem::path pathTickerDir = sPipeDir;
    std::filesystem::path pathTickerDir = std::filesystem::absolute(sPipeDir);
//    if(!std::filesystem::is_directory(pathTickerDir)){
//        ssOut <<" ./data/[TickerID] - is not directory";
//        return false;
//    }
    //--------------------------------------------------
    // prepare regexes to rearch
    std::stringstream ssReg;
    std::stringstream ssRegSign;

    // regex for search pipes by name
    // sample: AmiBroker2QUIK_TQBR.SBER_TICKS
    ssReg <<"^AmiBroker2QUIK_(.*)_TICKS$";
    const std::regex reAmiPipe {ssReg.str()};

    // regex to cut ticker sign from pipe name
    ssRegSign <<"(?:(?![.]).)+$";
    const std::regex reAmiSign {ssRegSign.str()};

    // simple vector to store pipenames
    //std::vector<std::filesystem::directory_entry> vPipes;

    // temporary variables
    std::string sSign;
    std::string sBind;

#ifdef _WIN32
#else
    // simple check that file system is working
    if (std::filesystem::exists(pathTickerDir))
#endif
    {
        // loop through files in pipes list
        for (const std::filesystem::directory_entry &fl:std::filesystem::directory_iterator{pathTickerDir}){

            // if exists and usual file (pipe is an usual file) - proceed
            if ( fl.exists()
                 && fl.is_regular_file()
                 //&& fl.is_fifo()
                 ){
                // check with regex if pipe fits
                std::string ss(fl.path().filename().string());
                const auto ItPipe = std::sregex_token_iterator(ss.begin(),ss.end(),reAmiPipe);
                if ( ItPipe != std::sregex_token_iterator()){
                    // cut with regex  part with sign from pipename
                    const auto ItQuik = std::sregex_token_iterator(ss.begin(),ss.end(),reAmiPipe,1);
                    sBind = *ItQuik;
                    if (ItQuik != std::sregex_token_iterator()){
                        // if success fill pipes list entry

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

                        // create new entry
                        mRet[sBind] = {0,{*ItPipe,sSign,ssFilePath.str(),0,0,sSign}};
                    }
                }
            }
        }
    }
    return mRet;
}

//-------------------------------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////////////
/// \brief Check pipes container for activity and change their status if needed
/// \param vT                   - list of tickers
/// \param mBindedPipesActive   - container with binded active pipes
/// \param mBindedPipesOff      - container with binded pipes for wich work was turned off
/// \param mFreePipes           - container with not binded to tickers pipes
/// \param vUnconnectedPipes    - list of tickers with binded pipe wich state is unconnected
/// \param vInformantsPipes     - list of tickers wich have no pipe and work was turned off
///
void AmiPipeHolder::CheckPipes(std::vector<Ticker> &vT,
                               dataAmiPipeTask::pipes_type & mBindedPipesActive,
                               dataAmiPipeTask::pipes_type & mBindedPipesOff,
                               dataAmiPipeTask::pipes_type &mFreePipes,
                               std::vector<int> &vUnconnectedPipes,
                               std::vector<int> &vInformantsPipes
                               )
{
    ///////////////////////////////////////////////////////////////
    // algotithm:
    //
    // 1. clear storage containers for pipes
    // 2. get pipes list from the system
    // 3. loop throught list of tickers and check pipes for them
    // 3.1. gets  identifier for pipe  stored in ticker (sign)
    // 3.2 if pipe for it exists proceed
    // 3.2.1 create pipe entry in temporary map and if set autoload for the ticker - set autoload flag
    // 3.2.2 create pipe entry in temporary map and  if not - set flag to not-autoload
    // 3.3 if no pipes for it:
    // 3.3.1 if set autoload for pipe move it to unconnected list
    // 3.3.2 if not - move it to informants
    // 4. loop throught found pipes
    // 4.1 create new entry in container with active pipe
    // 4.2 create new entry in container with turned off pipe
    // 4.3 create new entry in container with pipes do not bonded to any ticker
    ///////////////////////////////////////////////////////////////

    // 1. clear storage containers for pipes
    mBindedPipesActive.clear();
    mBindedPipesOff.clear();
    mFreePipes.clear();
    // 2. get pipes list from the system
    dataAmiPipeTask::pipes_type mPipes = AmiPipeHolder::ScanActivePipes();
    //
    // 3. loop throught list of tickers and check pipes for them
    std::string sTmp;
    for(const auto &t:vT){
        // 3.1. gets  identifier for pipe  stored in ticker (sign)
        sTmp = trim(t.TickerSignQuik());
        // 3.2 if pipe for it exists proceed
        if(sTmp.size() > 0 && mPipes.find(sTmp) != mPipes.end()){
            // 3.2.1 create pipe entry in temporary map and if set autoload for the ticker - set autoload flag
            if (t.AutoLoad()){
                mPipes[sTmp].second = {std::get<0>(mPipes[sTmp].second),
                                       std::get<1>(mPipes[sTmp].second),
                                       std::get<2>(mPipes[sTmp].second),
                                       t.TickerID(),
                                       1,
                                       std::get<5>(mPipes[sTmp].second)
                                      };
            }
            // 3.2.2 create pipe entry in temporary map and  if not - set flag to not-autoload
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
        // 3.3 if no pipes for it:
        else{
            // 3.3.1 if set autoload for pipe move it to unconnected list
            if (t.AutoLoad()){
                vUnconnectedPipes.push_back(t.TickerID());
            }
            //3.3.2 if not - move it to informants
            else{
                vInformantsPipes.push_back(t.TickerID());
            }
        }
    }
    // 4. loop throught found pipes
    for(const auto &p:mPipes){
        if (std::get<4>(p.second.second) == 1){
            // 4.1 create new entry in container with active pipe
            mBindedPipesActive[p.first] = p.second;
        }
        else if (std::get<4>(p.second.second) == 2){
            // 4.2 create new entry in container with turned off pipe
            mBindedPipesOff[p.first] = p.second;
        }
        else{
            // 4.3 create new entry in container with pipes do not bonded to any ticker
            mFreePipes[p.first] = p.second;
        }
    }
}
//-------------------------------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////////////
/// \brief Check set of active pipes and close in the broken
/// \param pActive              - container with binded active pipes
/// \param pOff                 - container with binded pipes for wich work was turned off
/// \param queuePipeAnswers     - queue to send activity events
///
void AmiPipeHolder::RefreshActiveSockets(dataAmiPipeTask::pipes_type& pActive,
                          dataAmiPipeTask::pipes_type& pOff,
                          BlockFreeQueue<dataAmiPipeAnswer>&queuePipeAnswers)
{

    ///////////////////////////////////////////////////////////
    // algorithm
    // 1. drop check flag for all entry in containers
    // 2. loop throught active pipes
    // 2.1 threadlocal exit variable check
    // 2.2 try to find in halted pipe list and if not find try to connect (halted pipes reconected by hands)
    // 2.3 search in connected pipes list and if its new - try to connect
    // 2.4 system dependant connect procedure
    // 3.1 initialize containers and variables for newly connected pipe
    // 3.2 send connection event
    // 4. if cannot connect - erase pipe from connected pipes list
    // 5. if cannot open file create new entry for halted pipes
    // 5.1 fill system dependant values for the entry
    // 5.2 send halt event for pipe
    // 6. mark connected pipe as checked
    // 7. mark halted pipe as checked
    // 8. for entries wich was not checked (ie check flag was not set) - remove from connect list
    // 8.1 do close for pipe object (ie socket)
    // 8.2 send disconnect signal depending on is pipe active or not
    // 8.3 remove variables set for the pipe
    // 8.4 erase entry in Connected pipe list and do loop
    // 9. erase absent (ie not checked) entries for halted pipes list
    ///////////////////////////////////////////////////////////



    // 1. drop check flag for all entry in containers
    for (auto &m:mPipesHalted){m.second.first =0;}
    for (auto &m:mPipesConnected){m.second.first =0;}
    //
    // 2. loop throught active pipes
    for(const auto & p:pActive){

        // 2.1 threadlocal exit variable check
        if (this_thread_flagInterrup.isSet()){
            return;
        }
        // 2.2 try to find in halted pipe list and if not find try to connect (halted pipes reconected by hands)
        auto ItHalted (mPipesHalted.find(p.first));
        if ( ItHalted == mPipesHalted.end()){
            // 2.3 search in connected pipes list and if its new - try to connect
            auto ItConnected (mPipesConnected.find(p.first));
            if ( ItConnected == mPipesConnected.end()){
                bool bOpend{false};
                try{
                    // 2.4 system dependant connect procedure
#ifdef _WIN32
                    // create new entry for the connected pipe and fill pipe information
                    mPipesConnected[p.first].first  = 1;                                                    // check flag
                    mPipesConnected[p.first].second = {std::get<3>(p.second.second),{}};                    // store pipe object itself
                    mPipesConnected[p.first].second.second.setPipePath(std::get<2>(p.second.second));       // store string pipe name

                    // set mode for pipe to work (mode set in constructor)
                    if (iMode == Byte_Nonblocking){
                        mPipesConnected[p.first].second.second.setMode(Win32NamedPipe::ePipeMode_type::Byte_Nonblocking);
                    }
                    else{
                        mPipesConnected[p.first].second.second.setMode(Win32NamedPipe::ePipeMode_type::Message_Nonblocking);
                    }
                    // try to open pipe (ie socket)
                    mPipesConnected[p.first].second.second.open();

                    // if all is ok proceed
                    if (mPipesConnected[p.first].second.second.good()){
#else
                    // plug - not fully implemented
                    std::ifstream file(std::get<2>(p.second.second),std::ios::in);
                    if (file.good()){
                        mPipesConnected[p.first].second = {std::get<3>(p.second.second),
                                                            std::move(file)};
#endif
                        // 3.1 initialize containers and variables for newly connected pipe
                        AddUtilityMapEntry(std::get<3>(p.second.second), p.first);

                        // 3.2 send connection event
                        dataAmiPipeAnswer answ;
                        answ.SetType(dataAmiPipeAnswer::PipeConnected);
                        answ.SetTickerID(std::get<3>(p.second.second));
                        queuePipeAnswers.Push(answ);
                        bOpend = true;
                    }
#ifdef _WIN32
                    // 4. if cannot connect - erase pipe from connected pipes list
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
                    // 5. if cannot open file create new entry for halted pipes
                    mPipesHalted[p.first].first  = 1; // set check flag

                    // 5.1 fill system dependant values for the entry
#ifdef _WIN32
                    Win32NamedPipe file("");
                    mPipesHalted[p.first].second = {std::get<3>(p.second.second),file};
#else
                    mPipesHalted[p.first].second = {std::get<3>(p.second.second),
                                                    std::ifstream{}
                                                   };
#endif
                    // 5.2 send halt event for pipe
                    dataAmiPipeAnswer answ;
                    answ.SetType(dataAmiPipeAnswer::PipeHalted);
                    answ.SetTickerID(std::get<3>(p.second.second));
                    queuePipeAnswers.Push(answ);
                }
            }
            else{ // 6. mark connected pipe as checked
                ItConnected->second.first = 1;
            }
        }
        else{ // 7. mark halted pipe as checked
            ItHalted->second.first = 1;
        }
        // exit if needed
        if (this_thread_flagInterrup.isSet()){
            return;
        }
    }
    //////////////////////////////////////////////////////////
    // 8. for entries wich was not checked (ie check flag was not set) - remove from connect list
    auto ItConnected = mPipesConnected.begin();
    while (ItConnected != mPipesConnected.end()){
        if (ItConnected->second.first == 0){
            ///
            // 8.1 do close for pipe object (ie socket)
            ItConnected->second.second.second.close();

            // 8.2 send disconnect signal depending on is pipe active or not
            dataAmiPipeAnswer answ;
            if(pOff.find(ItConnected->first)!=pOff.end())    answ.SetType(dataAmiPipeAnswer::PipeOff);
            else                                             answ.SetType(dataAmiPipeAnswer::PipeDisconnected);

            answ.SetTickerID(ItConnected->second.second.first);
            queuePipeAnswers.Push(answ);

            // 8.3 remove variables set for the pipe
            RemoveUtilityMapEntry(ItConnected->second.second.first, ItConnected->first);

            // 8.4 erase entry in Connected pipe list and do loop
            auto ItNext = std::next(ItConnected);
            mPipesConnected.erase(ItConnected);
            ItConnected = ItNext;
        }
        else{
            ItConnected++;
        }
    }
    // 9. erase absent (ie not checked) entries for halted pipes list
    auto ItHulted = mPipesHalted.begin();
    while (ItHulted != mPipesHalted.end()){
        // if check flag do not set - remove
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
///////////////////////////////////////////////////////////////////////////////////////
/// \brief main function to read from connected pipes
/// read all connected pipes until they have no data
/// better to be called from its own thread
/// construct outcoming events with data
/// exit on this_thread_flagInterrup.isSet()
/// \param queueFastTasks           - queue for receiving tasks
/// \param queuePipeAnswers         - queue to form answers
/// \param queueTrdAnswers          - queue to form answers
/// \param bCheckMode               - if true then  read begin of data flow and cut full company name from it else usual work
/// \param BytesRead                - return number of read bytes
/// \param bWasFullBuffers          - return true if there were more data in pipes buffers
///
void AmiPipeHolder::ReadConnectedPipes(BlockFreeQueue<dataFastLoadTask>                    &queueFastTasks,
                                       BlockFreeQueue<dataAmiPipeAnswer>                   &queuePipeAnswers,
                                       BlockFreeQueue<dataBuckgroundThreadAnswer>          &queueTrdAnswers,
                                       bool bCheckMode,
                                       int &BytesRead,
                                       bool & bWasFullBuffers)
{
    // system and mode dependant selector for work function

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
///////////////////////////////////////////////////////////////////////////////////////
/// \brief Utility to create all needed variables for newly connected pipe
/// \param iTickerID        - ticker ID in database
/// \param sBind            - pipe identifier (cut from pipe filename, equal to ticker sign)
///
void AmiPipeHolder::AddUtilityMapEntry(int iTickerID, std::string sBind)
{
    // defend maps change
    std::unique_lock lk(mutUtility);

    // safe get unique task identifier
    long lTask = aiTaskCounter.load();
    while(!aiTaskCounter.compare_exchange_weak(lTask,lTask + 1)){;}


    mBuffer[sBind].resize(iBlockMaxSize);                       // buffer to use with pipe
    mPointerToWrite[sBind] = 0;                                 // pointer to buffer to write (to set to pipe)
    mPointerToRead[sBind] = 0;                                  // pointer to buffer to read

    mDtActivity[iTickerID] = std::chrono::steady_clock::now();  // time to check ticker activity
    mCurrentSecond[iTickerID] = 0;                              // place to store curren second (there can be many ticks in one second)

    mTask[iTickerID] = lTask;                                   // unique task identifier for pipe connection
    mPacketsCounter[iTickerID] = 1;                             // initialize packets counter

    mPaperName[sBind] = "";//getSignFromBind(sBind);            // company name (not known when init)
    mCheckTime[sBind] = std::chrono::steady_clock::now();       // time to check activity for pipe

}
//-------------------------------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////////////
/// \brief Utility to remove all used variables for deleted pipe entry
/// \param iTickerID        - ticker ID in database
/// \param sBind            - pipe identifier (cut from pipe filename, equal to ticker sign)
/// \param bCheckMode       - mode of pipe work (for check mode here are no some variables)
///
void AmiPipeHolder::RemoveUtilityMapEntry(int iTickerID, std::string sBind, bool bCheckMode)
{
    // defend maps change
    std::unique_lock lk(mutUtility);

    // delete existing entries from maps

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
///////////////////////////////////////////////////////////////////////////////////////
/// \brief Read from pipes: windows/bytemode. Not implemented.
/// \param queueFastTasks
/// \param queuePipeAnswers
/// \param queueTrdAnswers
/// \param bCheckMode
/// \param BytesRead
/// \param bWasFullBuffers
///
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
///////////////////////////////////////////////////////////////////////////////////////
/// \brief Read from pipes: linux/messagemode. Not implemented.
/// \param BytesRead
/// \param bWasFullBuffers
///
void AmiPipeHolder::ReadConnectedPipes_messagemode_linux(BlockFreeQueue<dataFastLoadTask>        &/*queueFastTasks*/,
                                                         BlockFreeQueue<dataAmiPipeAnswer>                   &/*queuePipeAnswers*/,
                                                         BlockFreeQueue<dataBuckgroundThreadAnswer>          &/*queueTrdAnswers*/,
                                                         bool /*bCheckMode*/,
                                                         int & BytesRead,
                                                         bool & bWasFullBuffers
                            )
{
    // plug - not implemented
    \
    BytesRead = 0;
    bWasFullBuffers = false;
    return;
}
#endif
//-------------------------------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////////////
/// \brief Create event to send to log
/// \param queuePipeAnswers     - queue for messages
/// \param iTickerID            - ticker id
/// \param s                    - text to sent
///
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
///////////////////////////////////////////////////////////////////////////////////////
/// \brief Create error event to send to errlog
/// \param queuePipeAnswers     - queue for messages
/// \param iTickerID            - ticker id
/// \param s                    - text to sent
///
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
///////////////////////////////////////////////////////////////////////////////////////
/// \brief Write buffer to dump file
/// \param queuePipeAnswers     - queue for messages
/// \param iTickerID            - ticker id
/// \param sFileName            - filename to write
/// \param cBuff                - buffer with data
/// \param bytes                - bytes to write
/// \param iReadStart           - info of read start pointer
/// \param bWriteHeader         - if true then write header
///
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
///////////////////////////////////////////////////////////////////////////////////////
/// \brief Connect to pipe in check mode and get company name from it
/// \param pFree                - container with not binded with tickers pipes
/// \param queuePipeAnswers     - queue to answer
///
void AmiPipeHolder::AskPipesNames(dataAmiPipeTask::pipes_type &pFree, BlockFreeQueue<dataAmiPipeAnswer> & queuePipeAnswers)
{
    // loop throught free pipes
    for (const auto &p:pFree){
        // exit if was global interrupt
        if (this_thread_flagInterrup.isSet()){
            return;
        }
        // check if its new pipe
        if (mPipesFree.find(p.first) == mPipesFree.end()){
            bool bOpend{false};
            try{
                // system dependant connect
#ifdef _WIN32
                // create new pipe entry and fill infos
                mPipesFree[p.first].first  = 1;                                                 // set check flag
                mPipesFree[p.first].second = {std::get<3>(p.second.second),{}};                 // set pipe object itself
                mPipesFree[p.first].second.second.setPipePath(std::get<2>(p.second.second));    // set pipe filename

                // set work mode (set in constructor)
                if (iMode == Byte_Nonblocking){
                    mPipesFree[p.first].second.second.setMode(Win32NamedPipe::ePipeMode_type::Byte_Nonblocking);
                }
                else{
                    mPipesFree[p.first].second.second.setMode(Win32NamedPipe::ePipeMode_type::Message_Nonblocking);
                }

                // try to open
                mPipesFree[p.first].second.second.open();

                // if all is ok
                if (mPipesFree[p.first].second.second.good()){
#else
                // plug - not implemented
                std::ifstream file(std::get<2>(p.second.second),std::ios::in);
                if (file.good()){
                    mPipesFree[p.first].second = {std::get<3>(p.second.second),
                                                        std::move(file)};
#endif
                    // form variables for connected pipe
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
                // if cannot connect - return name same as sign
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
