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

#ifndef UTILITES_H
#define UTILITES_H

#include <algorithm>
#include <numeric>
#include <string>

//--------------------------------------------------------------------------------------------------------
// trimm left spaces
static inline std::string & ltrim(std::string &s){
    s.erase(begin(s),std::find_if(s.begin(),s.end(),[](const unsigned char t){
        return !std::isspace(t);
    }));
    return s;
}
static inline std::string & ltrim(std::string &&s){ return  ltrim(s);}
//--------------------------------------------------------------------------------------------------------
// trimm right spaces
static inline std::string & rtrim(std::string &s){
    s.erase(std::find_if(s.rbegin(),s.rend(),[](unsigned char ch){
        return !std::isspace(ch);
    }).base(),s.end()
            );
    return s;
}
static inline std::string & rtrim(std::string &&s){ return  rtrim(s);}
//--------------------------------------------------------------------------------------------------------
// trimm spaces from both sides
static inline std::string & trim(std::string &s){
    ltrim(s);
    rtrim(s);
    return s;
}
static inline std::string & trim(std::string &&s){ return  trim(s);}
//--------------------------------------------------------------------------------------------------------
// filter out ',' char
static inline std::string & filter(std::string &s){

    auto ItBeg(s.begin());

    auto ItEnd = std::accumulate(s.begin(),s.end(),ItBeg,[&](auto &It,const auto &c){
        if (c != ','){
            if (&(*It) != &c){
                *It = c;
            }
            ++It;
        }
        return It;
        });

    s.resize(std::distance(s.begin(),ItEnd));

    return s;
}
//--------------------------------------------------------------------------------------------------------
/// copy type trimms

//
//static inline std::string ltrim_copy(std::string s) {
//    ltrim(s);
//    return s;
//}

//static inline std::string rtrim_copy(std::string s) {
//    rtrim(s);
//    return s;
//}

//static inline std::string trim_copy(std::string s) {
//    trim(s);
//    return s;
//}
//--------------------------------------------------------------------------------------------------------

#endif // UTILITES_H
