#ifndef DATAAMIPIPETASK_H
#define DATAAMIPIPETASK_H

//#include "amipipeholder.h"

#include<map>
#include<string>

class dataAmiPipeTask
{
//typedef std::map<std::string,
//                              std::pair<int,
//                                             std::tuple<std::string,std::string,std::filesystem::directory_entry,int>
//                >> pipes_type;
public:
    enum eTask_type:int {Nop = -1, RefreshPipeList = 1};

    typedef std::map<std::string,std::pair<int,std::tuple<std::string,std::string,std::string,int,int>>> pipes_type;
private:
    eTask_type tType;
public:
    dataAmiPipeTask::pipes_type pipesBindedActive;
    dataAmiPipeTask::pipes_type pipesBindedOff;

public:

    dataAmiPipeTask(eTask_type t = eTask_type::Nop):tType{t}{;};
    dataAmiPipeTask(const dataAmiPipeTask &o) = default;


    inline eTask_type Type()      const {return tType;};
    inline void SetType(eTask_type t)   {tType = t;};

    dataAmiPipeTask& operator=(dataAmiPipeTask &o) = delete;
};

#endif // DATAAMIPIPETASK_H
