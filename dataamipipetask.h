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

#ifndef DATAAMIPIPETASK_H
#define DATAAMIPIPETASK_H

//#include "amipipeholder.h"

//#include<atomic>
#include<map>
#include<string>

//////////////////////////////////////////////////////////////////////////////////////////////////
/// data container for set tasks to ami pipe background process


class dataAmiPipeTask
{
//typedef std::map<std::string,
//                              std::pair<int,
//                                             std::tuple<std::string,std::string,std::filesystem::directory_entry,int>
//                >> pipes_type;
public:
    enum eTask_type:int {Nop = -1, RefreshPipeList = 1, AskPipesNames = 2};

    typedef std::map<std::string,std::pair<int,std::tuple<std::string,std::string,std::string,int,int,std::string>>> pipes_type;
private:
    eTask_type tType;
public:
    dataAmiPipeTask::pipes_type pipesFree;

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
