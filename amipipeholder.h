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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Used to hold client tasks to communicate with AmiBroker type pipes
///
/// Used one instance of object in all threads.
///
/// A client connection for a socket (channel) uses a single stream for all channels (i.e. for all connections).
///
/// platform depended part fully implemented only on windows
///
class AmiPipeHolder
{
public:

    // enumerator for pipe work type
    //typedef std::map<std::string,std::pair<int,std::tuple<std::string,std::string,std::string,int,int>>> pipes_type;
    enum ePipeMode_type:int {Byte_Nonblocking = 1,Message_Nonblocking = 2};
protected:

    // constants for time manipulation
    std::time_t t1971_01_01_00_00_00;
    std::time_t t1970_01_01_04_00_00;
    std::time_t t2100_01_01_00_00_00;

    // platform dependent container type for pipes declaration:
#ifdef _WIN32
    typedef std::map<std::string,std::pair<int,std::pair<int,Win32NamedPipe>>> internal_pipes_type;
#else
    typedef std::map<std::string,std::pair<int,std::pair<int,std::ifstream>>> internal_pipes_type;
#endif

    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    // internal_pipes_type description:
    // std::map<std::string,                                                - pipe sign (cut from pipe name and equal to ticker sign)
    //                      std::pair<int,                                  - check flag (used when check pipe connection)
    //                                    std::pair<int,                    - ticker ID in database
    //                                                  Win32NamedPipe>>>   - system dependant pipe object
    ////////////////////////////////////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////
    // containers for pipes, depends of pipe state

    internal_pipes_type mPipesConnected;
    internal_pipes_type mPipesHalted;

    internal_pipes_type mPipesFree;

    //////////////////////////////////////////////////////////////////////
    /// \brief utility maps
    ///
    std::shared_mutex mutUtility;                       // main mutex

    std::map<int,std::chrono::time_point<std::chrono::steady_clock>> mDtActivity;   // container for store last activity of pipes

    //
    std::map<int,std::time_t> mCurrentSecond;           // watermark

    // variables for parce raw data to ticks
    static const int iBlockMaxSize {2048};
    std::map<std::string,std::vector<char>> mBuffer;
    std::map<std::string,int> mPointerToWrite;
    std::map<std::string,int> mPointerToRead;

    // variables to order received ticks
    std::map<int,long> mTask;
    std::map<int,long long> mPacketsCounter;

    // variable to store received from pipe name of company for used ticker
    std::map<std::string,std::string> mPaperName;
    std::map<std::string,std::chrono::time_point<std::chrono::steady_clock>> mCheckTime;

    // work type of pipes to init
    ePipeMode_type iMode {Byte_Nonblocking};

    // current path
    std::filesystem::path pathCurr;

    //////////////////////////////////////////////////////////////////////
    /// variable to count active tasks
    std::atomic<long> aiTaskCounter;
    //////////////////////////////////////////////////////////////////////


public:
    AmiPipeHolder();

    // initial function to store current path
    inline void setCurrentPath(std::filesystem::path path) {pathCurr = path;};

    // function to check new pipes, test connect
    static void CheckPipes(std::vector<Ticker> &vT,
                           dataAmiPipeTask::pipes_type & mBindedPipes,
                           dataAmiPipeTask::pipes_type & mBindedPipesOff,
                           dataAmiPipeTask::pipes_type &mFreePipes,
                           std::vector<int> &vUnconnectedPipes,
                           std::vector<int> &vInformantsPipes);

    // function to check sockets for activity (connected/disconnected etc.)
    void RefreshActiveSockets(dataAmiPipeTask::pipes_type& pipesBindedActive,
                              dataAmiPipeTask::pipes_type& pipesBindedOff,
                              BlockFreeQueue<dataAmiPipeAnswer>                 &queuePipeAnswers);

    // main function to read from connected pipes
    // read all connected pipes until they have no data
    // better to be called from its own thread
    // construct outcoming events with data
    // exit on this_thread_flagInterrup.isSet()
    void ReadConnectedPipes(BlockFreeQueue<dataFastLoadTask>                    &queueFastTasks,
                            BlockFreeQueue<dataAmiPipeAnswer>                   &queuePipeAnswers,
                            BlockFreeQueue<dataBuckgroundThreadAnswer>          &queueTrdAnswers,
                            bool bCheckMode,
                            int & BytesRead,
                            bool & bWasFullBuffers
                            );

    // function to get company name from pipe for used ticker
    void AskPipesNames(dataAmiPipeTask::pipes_type &pFree, BlockFreeQueue<dataAmiPipeAnswer> & queuePipeAnswers);

protected:

    void AddUtilityMapEntry(int iTickerID, std::string sBind);      // init procedure for variables for newly connected pipe
    void RemoveUtilityMapEntry(int iTickerID, std::string sBind,bool bCheckMode = false);   // release variables procedure for closing pipe

    // scans local system for pipes and return map container with them
    static dataAmiPipeTask::pipes_type ScanActivePipes();

    static std::string getSignFromBind(std::string sBind);          // gets sign of ticker from data packet
    static std::string getNameFromRaw(std::string sRaw);            // gets name of ticker from data packet

    // init constants
    void initStartConst();

    // store logs
    void SendToLog      (BlockFreeQueue<dataAmiPipeAnswer> &queuePipeAnswers, const int iTickerID, const std::string &s);
    void SendToErrorLog (BlockFreeQueue<dataAmiPipeAnswer> &queuePipeAnswers, const int iTickerID, const std::string &s);
    void dumpToFile     (BlockFreeQueue<dataAmiPipeAnswer> &queuePipeAnswers, const int iTickerID, const std::string &sFileName, const  char * cBuff, const size_t bytes, const int iReadStart, const bool  bWriteHeader = true);

    //------------------------------------------------------------------------------------------------
    // platform independed part of parse procedure
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
    //------------------------------------------------------------------------------------------------
    //platform depended part of parse procedures
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
