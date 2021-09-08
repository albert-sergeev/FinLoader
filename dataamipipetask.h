#ifndef DATAAMIPIPETASK_H
#define DATAAMIPIPETASK_H

#include "amipipeholder.h"



class dataAmiPipeTask
{
//typedef std::map<std::string,
//                              std::pair<int,
//                                             std::tuple<std::string,std::string,std::filesystem::directory_entry,int>
//                >> pipes_type;
public:
    enum eTask_type:int {Nop = -1, RefreshPipeList = 1};
private:
    eTask_type tType;
public:
    AmiPipeHolder::pipes_type pipesBindedActive;
    AmiPipeHolder::pipes_type pipesBindedOff;

public:

    dataAmiPipeTask(eTask_type t = eTask_type::Nop):tType{t}{;};
    dataAmiPipeTask(const dataAmiPipeTask &o) = default;


    inline eTask_type Type()      const {return tType;};
    inline void setType(eTask_type t)   {tType = t;};

    dataAmiPipeTask& operator=(dataAmiPipeTask &o) = delete;
};

#endif // DATAAMIPIPETASK_H
