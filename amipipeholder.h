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

#ifndef AMIPIPEHOLDER_H
#define AMIPIPEHOLDER_H

#include<vector>
#include<filesystem>
#include<map>
#include<fstream>

#include"ticker.h"
#include "blockfreequeue.h"
#include "dataamipipetask.h"
#include "dataamipipeanswer.h"
#include "datafinloadtask.h"
#include "databuckgroundthreadanswer.h"
#include "datafastloadtask.h"

#include "win32namedpipe.h"

inline std::once_flag AmiPipeHolder_call_once_flag;


class AmiPipeHolder
{
public:
    //typedef std::map<std::string,std::pair<int,std::tuple<std::string,std::string,std::string,int,int>>> pipes_type;    
    enum ePipeMode_type:int {Byte_Nonblocking = 1,Message_Nonblocking = 2};
protected:

    std::time_t t1971_01_01_00_00_00;
    std::time_t t1970_01_01_04_00_00;
    std::time_t t2100_01_01_00_00_00;

#ifdef _WIN32
    typedef std::map<std::string,std::pair<int,std::pair<int,Win32NamedPipe>>> internal_pipes_type;
#else
    typedef std::map<std::string,std::pair<int,std::pair<int,std::ifstream>>> internal_pipes_type;
#endif

    internal_pipes_type mPipesConnected;
    internal_pipes_type mPipesHalted;

    internal_pipes_type mPipesFree;

    //////////////////////////////////////////////////////////////////////
    /// \brief utility maps
    ///
    std::shared_mutex mutUtility;

    std::map<int,std::chrono::time_point<std::chrono::steady_clock>> mDtActivity;

    std::map<int,std::time_t> mCurrentSecond;

    static const int iBlockMaxSize {2048};
    std::map<std::string,std::vector<char>> mBuffer;
    std::map<std::string,int> mPointerToWrite;
    std::map<std::string,int> mPointerToRead;

    std::map<int,long> mTask;
    std::map<int,long long> mPacketsCounter;

    std::map<std::string,std::string> mPaperName;
    std::map<std::string,std::chrono::time_point<std::chrono::steady_clock>> mCheckTime;

    ePipeMode_type iMode {Byte_Nonblocking};

    std::filesystem::path pathCurr;

    //////////////////////////////////////////////////////////////////////
    std::atomic<long> aiTaskCounter;
    //////////////////////////////////////////////////////////////////////


public:
    AmiPipeHolder();

    inline void setCurrentPath(std::filesystem::path path) {pathCurr = path;};

    static void CheckPipes(std::vector<Ticker> &vT,
                           dataAmiPipeTask::pipes_type & mBindedPipes,
                           dataAmiPipeTask::pipes_type & mBindedPipesOff,
                           dataAmiPipeTask::pipes_type &mFreePipes,
                           std::vector<int> &vUnconnectedPipes,
                           std::vector<int> &vInformantsPipes);

    void RefreshActiveSockets(dataAmiPipeTask::pipes_type& pipesBindedActive,
                              dataAmiPipeTask::pipes_type& pipesBindedOff,
                              BlockFreeQueue<dataAmiPipeAnswer>                 &queuePipeAnswers);

    void ReadConnectedPipes(BlockFreeQueue<dataFastLoadTask>                    &queueFastTasks,
                            BlockFreeQueue<dataAmiPipeAnswer>                   &queuePipeAnswers,
                            BlockFreeQueue<dataBuckgroundThreadAnswer>          &queueTrdAnswers,
                            bool bCheckMode,
                            int & BytesRead,
                            bool & bWasFullBuffers
                            );

    void AskPipesNames(dataAmiPipeTask::pipes_type &pFree, BlockFreeQueue<dataAmiPipeAnswer> & queuePipeAnswers);

protected:

    void AddUtilityMapEntry(int iTickerID, std::string sBind);
    void RemoveUtilityMapEntry(int iTickerID, std::string sBind,bool bCheckMode = false);


    static dataAmiPipeTask::pipes_type ScanActivePipes();
    static std::string getSignFromBind(std::string sBind);
    static std::string getNameFromRaw(std::string sRaw);

    void initStartConst();

    void SendToLog      (BlockFreeQueue<dataAmiPipeAnswer> &queuePipeAnswers, const int iTickerID, const std::string &s);
    void SendToErrorLog (BlockFreeQueue<dataAmiPipeAnswer> &queuePipeAnswers, const int iTickerID, const std::string &s);
    void dumpToFile     (BlockFreeQueue<dataAmiPipeAnswer> &queuePipeAnswers, const int iTickerID, const std::string &sFileName, const  char * cBuff, const size_t bytes, const int iReadStart, const bool  bWriteHeader = true);

    //------------------------------------------------------------------------------------------------

    int ProcessReceivedBuffer(BlockFreeQueue<dataFastLoadTask>              &queueFastTasks,
                              BlockFreeQueue<dataAmiPipeAnswer>             &queuePipeAnswers,
                              BlockFreeQueue<dataBuckgroundThreadAnswer>    &queueTrdAnswers,
                              const int iTickerID,
                              const std::string strBind,
                              bool bCheckMode,
                              dataFastLoadTask &task,
                              char *buff,
                              int &ptrToRead,
                              int &ptrToWrite
                              );
protected:
#ifdef _WIN32
    void ReadConnectedPipes_bytemode_win32(BlockFreeQueue<dataFastLoadTask>     &queueFastTasks,
                            BlockFreeQueue<dataAmiPipeAnswer>                   &queuePipeAnswers,
                            BlockFreeQueue<dataBuckgroundThreadAnswer>          &queueTrdAnswers,
                            bool bCheckMode,
                            int & BytesRead,
                            bool & bWasFullBuffers
                            );
    void ReadConnectedPipes_messagemode_win32(BlockFreeQueue<dataFastLoadTask>        &queueFastTasks,
                            BlockFreeQueue<dataAmiPipeAnswer>                   &queuePipeAnswers,
                            BlockFreeQueue<dataBuckgroundThreadAnswer>          &queueTrdAnswers,
                            bool bCheckMode,
                            int & BytesRead,
                            bool & bWasFullBuffers
                            );

    bool ReadPipe_bytemode_win32(Win32NamedPipe &pip,
                                 bool bCheckMode,
                                 const int iTickerID,
                                 const std::string sBind,
                                 BlockFreeQueue<dataFastLoadTask>                    &queueFastTasks,
                                 BlockFreeQueue<dataAmiPipeAnswer>                   &queuePipeAnswers,
                                 BlockFreeQueue<dataBuckgroundThreadAnswer>          &queueTrdAnswers,
                                 int & BytesRead,
                                 bool & bWasFullBuffers);
#else
    void ReadConnectedPipes_bytemode_linux(BlockFreeQueue<dataFastLoadTask>     &queueFastTasks,
                            BlockFreeQueue<dataAmiPipeAnswer>                   &queuePipeAnswers,
                            BlockFreeQueue<dataBuckgroundThreadAnswer>          &queueTrdAnswers,
                            bool bCheckMode,
                            int & BytesRead,
                            bool & bWasFullBuffers
                            );
    void ReadConnectedPipes_messagemode_linux(BlockFreeQueue<dataFastLoadTask>        &queueFastTasks,
                            BlockFreeQueue<dataAmiPipeAnswer>                   &queuePipeAnswers,
                            BlockFreeQueue<dataBuckgroundThreadAnswer>          &queueTrdAnswers,
                            bool bCheckMode,
                            int & BytesRead,
                            bool & bWasFullBuffers
                            );
#endif
    //------------------------------------------------------------------------------------------------
};

#endif // AMIPIPEHOLDER_H
