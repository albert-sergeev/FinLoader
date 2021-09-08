#ifndef AMIPIPEHOLDER_H
#define AMIPIPEHOLDER_H

#include<vector>
#include<filesystem>
#include<map>
#include<fstream>

#include"ticker.h"
#include "blockfreequeue.h"
#include "dataamipipeanswer.h"


class AmiPipeHolder
{
public:
    //typedef std::map<std::string,std::pair<int,std::tuple<std::string,std::string,std::filesystem::directory_entry,int,int>>> pipes_type;
    typedef std::map<std::string,std::pair<int,std::tuple<std::string,std::string,std::string,int,int>>> pipes_type;
protected:

    typedef std::map<std::string,std::pair<int,std::pair<int,std::ifstream>>> internal_pipes_type;

    internal_pipes_type mPipesConnected;
    internal_pipes_type mPipesHalted;

public:
    AmiPipeHolder();

    static void CheckPipes(std::vector<Ticker> &vT,
                           pipes_type & mBindedPipes,
                           AmiPipeHolder::pipes_type & mBindedPipesOff,
                           pipes_type &mFreePipes,
                           std::vector<int> &mUnconnectedPipes);

    void RefreshActiveSockets(pipes_type& pipesBindedActive,
                              pipes_type& pipesBindedOff,
                              BlockFreeQueue<dataAmiPipeAnswer>&queuePipeAnswers);

protected:
    static pipes_type ScanActivePipes();


};

#endif // AMIPIPEHOLDER_H
