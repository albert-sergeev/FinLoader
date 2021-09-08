#ifndef AMIPIPEHOLDER_H
#define AMIPIPEHOLDER_H

#include<vector>
#include<filesystem>
#include<map>
#include<fstream>

#include"ticker.h"
#include "blockfreequeue.h"
#include "dataamipipeanswer.h"

inline std::once_flag AmiPipeHolder_call_once_flag;

class AmiPipeHolder
{
public:
    //typedef std::map<std::string,std::pair<int,std::tuple<std::string,std::string,std::filesystem::directory_entry,int,int>>> pipes_type;
    typedef std::map<std::string,std::pair<int,std::tuple<std::string,std::string,std::string,int,int>>> pipes_type;
protected:

    typedef std::map<std::string,std::pair<int,std::pair<int,std::ifstream>>> internal_pipes_type;

    internal_pipes_type mPipesConnected;
    internal_pipes_type mPipesHalted;

    std::map<int,std::time_t> mCurrentSec;
    static const int iBlockMaxSize {65536};
    std::map<std::string,std::vector<char>> mBuffer;
    std::map<std::string,int> mPointerToWrite;
    std::map<std::string,int> mPointerToRead;

public:
    AmiPipeHolder();

    static void CheckPipes(std::vector<Ticker> &vT,
                           pipes_type & mBindedPipes,
                           AmiPipeHolder::pipes_type & mBindedPipesOff,
                           pipes_type &mFreePipes,
                           std::vector<int> &vUnconnectedPipes,
                           std::vector<int> &vInformantsPipes);

    void RefreshActiveSockets(pipes_type& pipesBindedActive,
                              pipes_type& pipesBindedOff,
                              BlockFreeQueue<dataAmiPipeAnswer>&queuePipeAnswers);

    void ReadConnectedPipes(std::map<int,std::vector<BarTick>> & mV,
                            BlockFreeQueue<dataAmiPipeAnswer>&queuePipeAnswers);

protected:
    static pipes_type ScanActivePipes();

    std::time_t t1971_01_01_00_00_00;

    std::time_t t1970_01_01_04_00_00;

    std::time_t t2100_01_01_00_00_00;

    void initStartConst(){
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


};

#endif // AMIPIPEHOLDER_H
