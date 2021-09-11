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

    //////////////////////////////////////////////////////////////////////
    std::atomic<long> aiTaskCounter;
    //////////////////////////////////////////////////////////////////////


public:
    AmiPipeHolder();

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
                            size_t &BytesRead
                            );

protected:

    void AddUtilityMapEntry(int iTickerID, std::string sBind);
    void RemoveUtilityMapEntry(int iTickerID, std::string sBind);


    static dataAmiPipeTask::pipes_type ScanActivePipes();

    void initStartConst();

};

#endif // AMIPIPEHOLDER_H
