#ifndef AMIPIPEHOLDER_H
#define AMIPIPEHOLDER_H

#include<vector>
#include<filesystem>
#include<map>

#include"ticker.h"


class AmiPipeHolder
{
public:
    typedef std::map<std::string,std::pair<int,std::tuple<std::string,std::string,std::filesystem::directory_entry>>> pipes_type;
protected:
    pipes_type mPipes;

public:
    AmiPipeHolder();

    static void CheckPipes(std::vector<Ticker> &vT,pipes_type & mBindedPipes,AmiPipeHolder::pipes_type & mBindedPipesOff, pipes_type &mFreePipes);

protected:
    static pipes_type ScanActivePipes();


};

#endif // AMIPIPEHOLDER_H
