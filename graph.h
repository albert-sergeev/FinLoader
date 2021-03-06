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

#ifndef GRAPH_H
#define GRAPH_H

#include<vector>
#include<list>
#include<map>
#include<set>
#include<stdexcept>
#include<atomic>
#include<utility>
#include<numeric>


#include "threadfreecout.h"
#include "bar.h"
#include "threadpool.h"

/////////////////////////////////////////////
/// \brief Class for store trade graphics. Base class for LSM tree.
/// elements - class Bar
/// elements counts by incremented index. Random access by time implimented by map dictionary
/// contains additional moving averages graphics and aligators
/// contains interfaces for adding new bars from single bar and from lists
///
///
template<typename T>
class Graph
{


private:

    // core data

    std::vector<T> vContainer;              // core stored data
    std::map<time_t,size_t> mDictionary;    // main index

    std::vector<double> vMovingBlue;        // derived data
    std::vector<double> vMovingRed;         // derived data
    std::vector<double> vMovingGreen;       // derived data

    size_t iLastCalculatedMovingAverage;    // mark to calculate derived data


    const Bar::eInterval iInterval;         // key data - used interval
    const int iTickerID;                    // key data - ticker

    size_t iShiftIndex;                     // left shift. used when this is partal data cut

    friend class Graph<Bar>;                // used to access to template class methods, when based on another class
    friend class Graph<BarTick>;            // used to access to template class methods, when based on another class

public:

    // size :)

    inline size_t size() const {return vContainer.size(); };

public:
    //--------------------------------------------------------------------------------------------------------
    // used only specialized constructors for safety
    Graph() = delete;
    Graph(Graph&) = delete;
    Graph& operator=(Graph&) = delete;
    //--------------------------------------------------------------------------------------------------------
    void clear(){
        mDictionary.clear();
        vContainer.clear();
        vMovingBlue.clear();

        vMovingRed.clear();
        vMovingGreen.clear();
        iLastCalculatedMovingAverage = 0;
    };
    //--------------------------------------------------------------------------------------------------------
    explicit Graph(int TickerID, Bar::eInterval Interval):iLastCalculatedMovingAverage{0},iInterval{Interval},iTickerID{TickerID},iShiftIndex{0}{;}
    explicit Graph(Graph&& o):
        vContainer{std::move(o.vContainer)}
       ,mDictionary{std::move(o.mDictionary)}
       ,vMovingBlue{std::move(o.vMovingBlue)}
       ,vMovingRed{std::move(o.vMovingRed)}
       ,vMovingGreen{std::move(o.vMovingGreen)}
       ,iLastCalculatedMovingAverage{o.iLastCalculatedMovingAverage}
       ,iInterval{o.iInterval}
       ,iTickerID{o.iTickerID}
       ,iShiftIndex{o.iShiftIndex}{;}
    //--------------------------------------------------------------------------------------------------------
    // safe access methods to main core data
    T & operator[](const size_t i)  {if (/*i<0 ||*/ i>= vContainer.size()) {throw std::out_of_range("");} return vContainer[i];}
    const T & operator[](const size_t i) const  {if (/*i<0 ||*/ i>= vContainer.size()) {throw std::out_of_range("");} return vContainer[i];}
    //--------------------------------------------------------------------------------------------------------
    // safe access methods to derived data
    double movingBlue (const size_t i) const {
        if constexpr (std::is_same_v<T, Bar>)
            {if (/*i<0 ||*/ i>= vMovingBlue.size())  {throw std::out_of_range("");} return vMovingBlue[i];}
        else{return -10;}}
    double movingRed  (const size_t i) const {
        if constexpr (std::is_same_v<T, Bar>)
            {if (/*i<0 ||*/ i>= vMovingRed.size())   {throw std::out_of_range("");} return vMovingRed[i];}
        else{return 0;}}
    double movingGreen(const size_t i) const {
        if constexpr (std::is_same_v<T, Bar>)
            {if (/*i<0 ||*/ i>= vMovingGreen.size()) {throw std::out_of_range("");} return vMovingGreen[i];}
        else{return 0;}}
    //--------------------------------------------------------------------------------------------------------
    // build methods:
    bool AddBarsList(std::vector<std::vector<T>> &v, std::time_t dtStart,std::time_t dtEnd);
    bool AddBarsListsFast(std::vector<T> &v, std::set<std::time_t>   & stHolderTimeSet,std::pair<std::time_t,std::time_t> &pairRange,Graph<T> &grDst);

    template<typename T_SRC>
    bool BuildFromLowerList(Graph<T_SRC> &grSrc, std::time_t dtStart,std::time_t dtEnd,bool bCopyToDst,Graph<T> &grDst);

    //--------------------------------------------------------------------------------------------------------
    // utility methods for access to container data:

    inline std::time_t GetDateMin() const  {return  vContainer.size()>0? vContainer.front().Period():0;};
    inline std::time_t GetDateMax() const  {return  vContainer.size()>0? vContainer.back().Period():0;};

    std::tuple<double,double,unsigned long,unsigned long>  getMinMax() const;
    std::tuple<double,double,unsigned long,unsigned long>  getMinMax(std::time_t dtStart,std::time_t dtEnd) const;

    size_t getIndex(const std::time_t t) const;

    size_t getMovingBlueSize() const {return vMovingBlue.size();};
    size_t getMovingRedSize() const {return vMovingRed.size();};
    size_t getMovingGreenSize() const {return vMovingGreen.size();};
    //--------------------------------------------------------------------------------------------------------
    // method for integrity control
    bool CheckMap();
    //--------------------------------------------------------------------------------------------------------
    //  utility get-type methods

    std::string ToString();
    std::string ToStringPeriods();
    std::size_t GetShiftIndex()  const {return iShiftIndex;};

    std::size_t GetUsedMemory() const;

    //--------------------------------------------------------------------------------------------------------
    // method to clone graph to another container
    void CloneGraph(Graph<T> &grNew,const size_t Start, const size_t End, const size_t LetShift);

    // release metod (cut and free tail)
    bool shrink_extras_left(std::time_t dtEnd);
    //
private:
    static size_t GetMoreThenIndex(std::vector<T> & v, std::time_t tT);
    void calculateMovingAverages(size_t iStart);
};

//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
template<typename T>
bool Graph<T>::AddBarsList(std::vector<std::vector<T>> &v, std::time_t dtStart,std::time_t dtEnd)
{

    ///
    size_t iNewLength{0};
    bool bInRange{true};
    for(const auto & lst:v){
        if (lst.size()>0){
            iNewLength += lst.size();
            if (lst.front().Interval() != this->iInterval){
                std::stringstream ss;
                ss<< "AddBarsList. graph interval [" <<this->iInterval<<"] != ["<<lst.front().Interval()<<"]";
                throw std::invalid_argument(ss.str());
            }
            if( (lst.front().Period() < dtStart)){
                std::stringstream ss;
                ss<< "AddBarsList. the period of incoming data  [" <<lst.front().Period()<<"] is less than the beginning of the range ["<<dtStart<<"]";
                throw std::invalid_argument(ss.str());
            }
            if(lst.back().Period() > dtEnd){
                std::stringstream ss;
                ss<< "AddBarsList. the period of incoming data  [" <<lst.back().Period()<<"] is greater than the window of the range ["<<dtEnd<<"]";
                throw std::invalid_argument(ss.str());
            }
            //
            if(!vContainer.empty() && lst.front().Period() <= vContainer.back().Period()){
                bInRange = false;
            }
        }
    }
    if(iNewLength <= 0) {
        return true;
    }
    //////////////////////////////////////////////////////////////////
    size_t iStart{0};
    size_t iEnd{0};

    if (bInRange){
        // if it's addition to the back:
        // 1. scroll through the lists
        // 2. try to look up in a dictionary because there may be multiple tickers in a second
        // 3. if not found, add a new entry to the map
        // 4. push back to vector
//        { ThreadFreeCout pcout; pcout<<"addition to tail\n";}
        iStart = vContainer.size();
        //
        for(const auto & lst:v){
            for(size_t i = 0; i < lst.size(); ++i){
                if (mDictionary.find(lst[i].Period()) == mDictionary.end()){
                    mDictionary[lst[i].Period()] = vContainer.size()+i;
                }
            }
            std::copy(lst.begin(),lst.end(),std::back_inserter(vContainer));
            if(this_thread_flagInterrup.isSet()){return false;}
        }
    }
    else{
        // if it's an insert:
        // 1. find range to delete in the vector
        // 2. clear range to delete from map;
        // 3. resize vector
        // 4. insert new data into the vector


        if(this_thread_flagInterrup.isSet()){return false;}
        // 1. look up range to delete in the vector
        iStart = GetMoreThenIndex(vContainer, dtStart);
        iEnd = GetMoreThenIndex(vContainer, dtEnd + 1); //


        if(this_thread_flagInterrup.isSet()){return false;}
        // 2. clear range to delete from map;
        auto It (mDictionary.begin());
        auto ItEnd (mDictionary.end());
        if (iStart != 0){
            if (iStart == vContainer.size()){
                throw std::runtime_error("logic error in Graph::AddBarsList 1");
            }
            It = mDictionary.find(vContainer[iStart].Period());
        }
        if (iEnd != vContainer.size()){
            ItEnd = mDictionary.find(vContainer[iEnd].Period());
        }
        // 2.2 shift index in the remaining tail
        auto ItEndSrc = ItEnd;
        if (iNewLength > iEnd - iStart){
            while(ItEndSrc != mDictionary.end()){
                (*ItEndSrc).second += (iNewLength - (iEnd - iStart));
                ItEndSrc++;
            }
        }
        else if(iNewLength < iEnd - iStart){
            while(ItEndSrc != mDictionary.end()){
                (*ItEndSrc).second -= ((iEnd - iStart)-iNewLength);
                ItEndSrc++;
            }
        }
        // 2.3 erase olds
        mDictionary.erase(It,ItEnd);
        //
        if(this_thread_flagInterrup.isSet()){return false;}
        // 3. resize vector
        if (iNewLength < iEnd - iStart){

            auto ItDst (std::next(vContainer.begin(),iStart + iNewLength));
            auto ItStart (std::next(vContainer.begin(),iEnd));
            auto ItStop  (vContainer.end());

            std::copy(ItStart,ItStop,ItDst);

            vContainer.resize(vContainer.size() + iNewLength - (iEnd - iStart));
        }
        else if (iNewLength > iEnd - iStart){

            size_t iDelta = iNewLength - (iEnd - iStart);
            size_t iOldSize = vContainer.size();

            vContainer.resize(vContainer.size() + iDelta);

            auto ItDst (vContainer.rbegin());
            auto ItStart (std::next(vContainer.rbegin(),iDelta));
            auto ItStop  (std::next(vContainer.rbegin(),iDelta + (iOldSize-iStart)));

            std::copy(ItStart,ItStop,ItDst);
        }
        if(this_thread_flagInterrup.isSet()){return false;}
        // 4. insert new data into the vector
        auto ItStart (std::next(vContainer.begin(),iStart));
        for(const auto & lst:v){
            //for(const Bar &b:lst){
            for(size_t i = 0; i < lst.size(); ++i){
                if (mDictionary.find(lst[i].Period()) == mDictionary.end()){
                    mDictionary[lst[i].Period()] = std::distance(vContainer.begin(),ItStart)+i;
                }
//                (*ItStart) = (vContainer[i]);
//                ItStart++;
            }
            std::copy(lst.begin(),lst.end(),ItStart);
            ItStart = std::next(ItStart,lst.size());
        }
        //
        if (mDictionary.size() > vContainer.size()){
            throw std::runtime_error("logic error in Graph::AddBarsList 3\n");
        }

    }
    //////////////////////////////////////////////////////////////////////////
    if constexpr (std::is_same_v<T, Bar>){
        if (vContainer.size() + 10 > vMovingBlue.capacity()){
            vMovingBlue.reserve(vContainer.size() + 100);
            vMovingRed.reserve(vContainer.size() + 100);
            vMovingGreen.reserve(vContainer.size() + 100);
        }

        vMovingBlue.resize  (vContainer.size() + 8);
        vMovingRed.resize   (vContainer.size() + 5);
        vMovingGreen.resize (vContainer.size() + 3);

        if (iLastCalculatedMovingAverage + 1 < vContainer.size() || iStart + 1 < vContainer.size()){
            iStart = iLastCalculatedMovingAverage > iStart ? iStart : iLastCalculatedMovingAverage;
            calculateMovingAverages(iStart);
            iLastCalculatedMovingAverage = vContainer.size() - 1;
        }
    }
    //////////////////////////////////////////////////////////////////////////
    return true;;
}
//------------------------------------------------------------------------------------------------------------
template<typename T>
bool Graph<T>::AddBarsListsFast(std::vector<T>                      &vV,
                                std::set<std::time_t>               & stHolderTimeSet,
                                std::pair<std::time_t,std::time_t>  & pairRange,
                                Graph<T>                            & grDst
                                )
{
    pairRange = {0,0};

    if (vV.empty()) return true;
    /////////////////////////////////////////////////////////////////
    /// build map for the new range
    ///
    std::map<std::time_t,std::vector<BarTick>> mNew;
    for(const auto &b:vV){
        if (mNew[b.Period()].empty()){
            mNew[b.Period()].reserve(vV.size());
        }
        mNew[b.Period()].push_back(b);
    }
    /////////////////////////////////////////////////////////////////
    /// build full new map with append and deletion ranges

    std::map<std::time_t,std::vector<BarTick>> mAppendMap;

    auto It (mNew.begin());
    while (It !=mNew.end()){
        if (stHolderTimeSet.find(It->first) == stHolderTimeSet.end()){
            // new tick: no appending but add deletion ranges

            mAppendMap[It->first];// here we are! ;)
            /////////
            /// search range to clear
            std::time_t tB{It->first}; // begin of needed to clean period
            std::time_t tE{It->first}; // end of needed to clean period

            auto ItB (stHolderTimeSet.lower_bound(It->first));
            auto ItE (ItB);//(stTimeSet.upper_bound(tSec));

            if (/*ItB !=stHolderTimeSet.end() &&*/ ItB != stHolderTimeSet.begin()){
                --ItB;
                tB = (*ItB) + 1;
            }
            if (ItE != stHolderTimeSet.end()){
                tE = (*ItE) - 1;
            }
            /////////
            /// adding instances to future clear

            auto ItSrc    ( mDictionary.lower_bound(tB));
            auto ItSrcEnd ( mDictionary.upper_bound(tE));
            while(ItSrc != ItSrcEnd){
                mNew[ItSrc->first];
                ++ItSrc;
            }
            /////////
            stHolderTimeSet.emplace(It->first); // mark done
        }
        else{
            if (mAppendMap.find(It->first) == mAppendMap.end()){ //first interence
                // tick has been in the task: do appending

                mAppendMap[It->first];// here we are! ;)
                //
                auto ItDict (mDictionary.find(It->first));
                if ( ItDict!= mDictionary.end()){ // if there is something
                    auto ItCpB (std::next(vContainer.begin(),ItDict->second));
                    auto ItCpE (ItCpB);
                    ItDict++;
                    if ( ItDict!= mDictionary.end()){
                        ItCpE = std::next(vContainer.begin(),ItDict->second);
                    }
                    else{
                        ItCpE = vContainer.end();
                    }
                    /////////
                    mAppendMap[It->first].reserve(std::distance(ItCpB,ItCpE));
                    std::copy(ItCpB,ItCpE,std::back_inserter(mAppendMap[It->first]));

                }
            }
        }
        ++It;
    }
    /////////////////////////////////////////////////////////////////
    /// calculate new length

    size_t iNewLength{0};
    for(const auto &v:mNew){
        iNewLength += v.second.size();
    }
    for(const auto &v:mAppendMap){
        iNewLength += v.second.size();
    }
    /////////////////////////////////////////////////////////////////
    /// calculate invalid range in olds
    ///

    auto ItSrc    ( mDictionary.lower_bound(mNew.begin()->first));
    auto ItSrcEnd ( mDictionary.upper_bound(mNew.rbegin()->first));

    size_t iOldRange{0};
    size_t iRangeBegin {vContainer.size()};
    if (ItSrc != mDictionary.end()){
        iRangeBegin = ItSrc->second;
        if (ItSrcEnd != mDictionary.end()){
            iOldRange = ItSrcEnd->second - ItSrc->second;
        }
        else{
            iOldRange = vContainer.size() - ItSrc->second;
        }
    }

    /////////////////////////////////////////////////////////////////
    /// resize main storage
    int iDelta ((int)(iNewLength - iOldRange));

    size_t iOldTail = vContainer.size();
    if(ItSrcEnd != mDictionary.end()){
        iOldTail = ItSrcEnd->second;
    }
    size_t iOldTailLength = vContainer.size() - iOldTail;

    if (iDelta > 0){

//        auto ItSrc    ( mDictionary.lower_bound(mNew.begin()->second.front().Period()));
//        auto ItSrcEnd ( mDictionary.upper_bound(mNew.rbegin()->second.back().Period()));

        if (vContainer.capacity() < vContainer.size() + iDelta){
            vContainer.reserve(vContainer.size() + iDelta + 65535);
        }
        vContainer.resize(vContainer.size() + iDelta);

        auto rItBeg (std::next(vContainer.rbegin(),iDelta));
        auto rItEnd (std::next(vContainer.rbegin(),iDelta + iOldTailLength));

        // copy tail data
        std::copy(rItBeg,rItEnd,vContainer.rbegin());

        // shift tail index
        auto ItMShift (ItSrcEnd);
        while(ItMShift !=  mDictionary.end()){
            ItMShift->second = ItMShift->second + iDelta;
            ++ItMShift;
        }

        //erase old range index
        mDictionary.erase(ItSrc,ItSrcEnd);

    }
    else if(iDelta < 0){
        //        auto ItSrc    ( mDictionary.lower_bound(mNew.begin()->second.front().Period()));
        //        auto ItSrcEnd ( mDictionary.upper_bound(mNew.rbegin()->second.back().Period()));

        if (ItSrcEnd != mDictionary.end()){

            auto ItBeg (std::next(vContainer.begin(),ItSrcEnd->second));
            auto ItEnd (vContainer.end());
            // copy tail data
            std::copy(ItBeg,ItEnd,std::next(vContainer.begin(),ItSrcEnd->second + iDelta));
        }
        vContainer.resize(vContainer.size() + iDelta);

        // shift tail index
        auto ItMShift (ItSrcEnd);
        while(ItMShift !=  mDictionary.end()){
            ItMShift->second = ItMShift->second + iDelta;
            ++ItMShift;
        }

        //erase old range index
        mDictionary.erase(ItSrc,ItSrcEnd);
    }
    /////////////////////////
    /// copy new data
    auto ItInsert(vContainer.begin());
    for(const auto &v:mNew){
        std::vector<T> &vA =  mAppendMap[v.first];
        if (v.second.size()>0) mDictionary[v.first] = iRangeBegin;
        if (vA.size()>0){
            ItInsert = std::next(vContainer.begin(),iRangeBegin);
            std::copy(vA.begin(),vA.end(),ItInsert);
            mDictionary[v.first] = iRangeBegin;
            iRangeBegin += vA.size();
        }
        ItInsert = std::next(vContainer.begin(),iRangeBegin);
        std::copy(v.second.begin(),v.second.end(),ItInsert);
        iRangeBegin += v.second.size();
    }

    /////////////////////////
    /// make affected range
    pairRange = {mNew.begin()->first,mNew.rbegin()->first};
    //size_t iStart{0};

    /////////////////////////
    /// copy data chunk for fast load
    grDst.clear();
    auto ItDictRangeBegin (mDictionary.lower_bound(pairRange.first));
    auto ItDictRangeEnd (mDictionary.upper_bound(pairRange.second));

    if(ItDictRangeBegin != mDictionary.end()){
        auto ItRangeBegin   (std::next(vContainer.begin(),ItDictRangeBegin->second));
        auto ItRangeEnd     (ItRangeBegin);
        if(ItDictRangeEnd != mDictionary.end()){
            ItRangeEnd = std::next(vContainer.begin(),ItDictRangeEnd->second);
        }
        else{
            ItRangeEnd = vContainer.end();
        }

        grDst.vContainer.reserve(std::distance(ItRangeBegin,ItRangeEnd));

        std::time_t tOldTime{0};
        while(ItRangeBegin != ItRangeEnd){
            grDst.vContainer.push_back(*ItRangeBegin);
            if (grDst.vContainer.back().Period() !=tOldTime){
                tOldTime = grDst.vContainer.back().Period();
                grDst.mDictionary[tOldTime] = grDst.vContainer.size() - 1;
            }
            ++ItRangeBegin;
        }

        grDst.iShiftIndex = ItDictRangeBegin->second;
        //iStart =  ItDictRangeBegin->second;
    }

    //////////////////////////////////////////////////////////////////////////
    if constexpr (std::is_same_v<T, Bar>){ // never!!!
//        if (vContainer.size() + 8 >  vMovingBlue.size())    vMovingBlue.resize  (vContainer.size() + 8);
//        if (vContainer.size() + 5 >  vMovingRed.size())     vMovingRed.resize   (vContainer.size() + 5);
//        if (vContainer.size() + 3 >  vMovingGreen.size())   vMovingGreen.resize (vContainer.size() + 3);
//        //calculateMovingAverages(iStart);
//        size_t iRecalc = iStart >= 8 ? iStart - 8: 0;
//        calculateMovingAverages(iRecalc);
    }
    //////////////////////////////////////////////////////////////////////////
    return true;
}

//------------------------------------------------------------------------------------------------------------
template<typename T>
bool Graph<T>::CheckMap()
{
    for (const auto &e:mDictionary){
        if(/*e.second <0 &&*/ e.second >=vContainer.size()){
            ThreadFreeCout pcout;
            pcout<<"map->index > vContainer.size\n";
            return false;
        }
        if (vContainer[e.second].Period() != e.first){
            ThreadFreeCout pcout;
            pcout<<"vContainer[map[index]].period != map->index\n";
            pcout<<"vContainer[e.second].Period(): "<<vContainer[e.second].Period()<<"\n";
            pcout<<"e.first: "<<e.first<<"\n";
            pcout<<"e.second: "<<e.second<<"\n";
            return false;
        }
        if (e.second >0 && vContainer[e.second].Period() == vContainer[e.second-1].Period()){
            ThreadFreeCout pcout;
            pcout<<"vContainer[map[index]].period == vContainer[map[index]-1].period\n";
            return false;
        }
        if(this_thread_flagInterrup.isSet()){return true;}
    }
    for (size_t i = 0; i< vContainer.size(); ++i){
        if(mDictionary.find(vContainer[i].Period()) == mDictionary.end()){
            ThreadFreeCout pcout;
            pcout<<"map[vContainer[i].period] not found\n";
            return false;
        }
        if(vContainer[mDictionary[vContainer[i].Period()]].Period() != vContainer[i].Period() ){
            ThreadFreeCout pcout;
            pcout<<"vContainer[mDictionary[vContainer[i].Period()]].Period() != vContainer[i].Period()\n";
            return false;
        }
        if(this_thread_flagInterrup.isSet()){return true;}
    }

    return true;
}
//------------------------------------------------------------------------------------------------------------
template<typename T>
size_t Graph<T>::GetMoreThenIndex(std::vector<T> & v, std::time_t tT)
{
    if(v.size() <= 0) return 0;
    if (v[v.size()-1].Period() < tT) return v.size();
    ////////////////////////////////////////////
    size_t iL = 0;
    size_t iR = v.size();

    size_t middle = iL + (iR - iL)/2;

    int iCount{0};

    while(iL < iR){
        middle = iL + (iR - iL)/2;
        if (v[middle].Period() > tT){
            iR = middle;
        }
        else if (v[middle].Period() < tT){
            iL = middle + 1;
        }
        else{
            iR = middle;
        }
        iCount++;
    }
    return  iR;
}
//------------------------------------------------------------------------------------------------------------
template<typename T>
void Graph<T>::CloneGraph(Graph<T> &grNew,const size_t Start, const size_t End, const size_t LetShift)
{
    grNew.clear();
    size_t iBeg = Start > LetShift ? Start - LetShift : 0;

    if (iBeg >= vContainer.size()) return;

    auto ItBeg(std::next(vContainer.begin(),iBeg));
    auto ItEnd(vContainer.end());
    if(End < vContainer.size()-1){
        ItEnd = std::next(vContainer.begin(),End + 1);
    }

    grNew.vContainer.reserve(std::distance(ItBeg,ItEnd));

    std::copy(ItBeg,ItEnd,std::back_inserter(grNew.vContainer));
    grNew.iShiftIndex = iBeg;

    ////////////////////////////////////////////////////////////////////
    if constexpr (std::is_same_v<T, Bar>){
//        if (bRecalculateAverages){
//            if (vContainer.size() + 8 >  vMovingBlue.size())    vMovingBlue.resize  (vContainer.size() + 8);
//            if (vContainer.size() + 5 >  vMovingRed.size())     vMovingRed.resize   (vContainer.size() + 5);
//            if (vContainer.size() + 3 >  vMovingGreen.size())   vMovingGreen.resize (vContainer.size() + 3);
//            calculateMovingAverages(iBeg);
//        }

        //////////////////////////////////
        if (vMovingBlue.size() > iBeg){
            auto ItBlueBeg(std::next(vMovingBlue.begin(),iBeg));
            auto ItBlueEnd(vMovingBlue.end());
            if(End < vMovingBlue.size()-1){
                ItBlueEnd = std::next(vMovingBlue.begin(),End + 1);
            }
            grNew.vMovingBlue.reserve(std::distance(ItBlueBeg,ItBlueEnd));
            std::copy(ItBlueBeg,ItBlueEnd,std::back_inserter(grNew.vMovingBlue));
        }
        //////////////////////////////////
        if (vMovingRed.size() > iBeg){
            auto ItRedBeg(std::next(vMovingRed.begin(),iBeg));
            auto ItRedEnd(vMovingRed.end());
            if(End < vMovingRed.size()-1){
                ItRedEnd = std::next(vMovingRed.begin(),End + 1);
            }
            grNew.vMovingRed.reserve(std::distance(ItRedBeg,ItRedEnd));
            std::copy(ItRedBeg,ItRedEnd,std::back_inserter(grNew.vMovingRed));
        }
        //////////////////////////////////
        if (vMovingGreen.size() > iBeg){
            auto ItGreenBeg(std::next(vMovingGreen.begin(),iBeg));
            auto ItGreenEnd(vMovingGreen.end());
            if(End < vMovingGreen.size()-1){
                ItGreenEnd = std::next(vMovingGreen.begin(),End + 1);
            }
            grNew.vMovingGreen.reserve(std::distance(ItGreenBeg,ItGreenEnd));
            std::copy(ItGreenBeg,ItGreenEnd,std::back_inserter(grNew.vMovingGreen));
        }
    }
    ////////////////////////////////////////////////////////////////////
}
//------------------------------------------------------------------------------------------------------------
template<typename T>
std::string Graph<T>::ToString()
{
    std::stringstream ss;
    ss<<"{";
    auto It (vContainer.begin());
    if (It != vContainer.end()){
        ss<< It->Period();
        It++;
    }
    while(It != vContainer.end()){
        ss<<","<<It->Period();
        It++;
    }
    ss<<"}";
    return ss.str();
}
//------------------------------------------------------------------------------------------------------------
template<typename T>
std::string Graph<T>::ToStringPeriods()
{
    std::stringstream ss;
    ss<<"{";
    auto It (vContainer.begin());
    if (It != vContainer.end()){
        ss<< It->Period();
        It++;
    }
    while(It != vContainer.end()){
        ss<<","<<It->Period();
        It++;
    }
    ss<<"}";
    return ss.str();
}

//------------------------------------------------------------------------------------------------------------
template<typename T>
size_t Graph<T>::getIndex(const std::time_t t) const
{
    std::time_t tT = Bar::DateAccommodate(t,this->iInterval);
    auto It (mDictionary.lower_bound(tT));
    if (It != mDictionary.end()){
        return It->second;
    }
    else{
        return vContainer.size();
    }
}
//------------------------------------------------------------------------------------------------------------
template<typename T>
bool Graph<T>::shrink_extras_left(std::time_t dtEnd)
{
    auto It = mDictionary.lower_bound(dtEnd);
    if(It == mDictionary.end()) {return true;}

    auto ItContEnd  = std::next(vContainer.begin(),It->second);

    if constexpr (std::is_same_v<T, Bar>){
        auto ItBlueEnd  = std::next(vMovingBlue.begin(),It->second);
        auto ItRedEnd   = std::next(vMovingRed.begin(),It->second);
        auto ItGreenEnd = std::next(vMovingGreen.begin(),It->second);

        vMovingBlue.erase   (vMovingBlue.begin(),ItBlueEnd);
        vMovingRed.erase    (vMovingRed.begin(),ItRedEnd);
        vMovingGreen.erase  (vMovingGreen.begin(),ItGreenEnd);
    }

    vContainer.erase(vContainer.begin(),ItContEnd);
    mDictionary.erase(mDictionary.begin(),It);

    size_t iDelta = It->second;
    while(It != mDictionary.end()){
        It->second += iDelta;
        ++It;
    }
    iLastCalculatedMovingAverage += iDelta;
    return true;

}
//------------------------------------------------------------------------------------------------------------
template<typename T> template<typename T_SRC>
//bool Graph<T>::BuildFromLowerList(Graph<BarTick> &grSrc, std::time_t dtStart,std::time_t dtEnd)
bool Graph<T>::BuildFromLowerList(Graph<T_SRC> &grSrc, std::time_t dtStart,std::time_t dtEnd,
                                  bool bCopyToDst,Graph<T> &grDst)
{


    std::time_t dtAccStart      = Bar::DateAccommodate(dtStart,iInterval);
    std::time_t dtAccEndLower   = Bar::DateAccommodate(dtEnd,iInterval,false);
    std::time_t dtAccEnd        = Bar::DateAccommodate(dtEnd,iInterval,true);

    auto It     (grSrc.mDictionary.lower_bound(dtAccStart));
    auto ItEnd  (grSrc.mDictionary.lower_bound(dtAccEnd));

    if (It == grSrc.mDictionary.end())  return true;

    std::time_t dtNext          = Bar::DateAccommodate(It->first,iInterval,true);

    auto ItCur  (grSrc.mDictionary.lower_bound(dtNext));
    if (ItCur != grSrc.mDictionary.end()){
        dtNext = ItCur->first;
    }

    std::vector<std::vector<Bar>> v;
    v.push_back(std::vector<Bar>{});
    std::vector<Bar> & vRes = v.back();

    try{

    auto v_ItBeg (grSrc.vContainer.end());
    auto v_ItEnd (grSrc.vContainer.end());

    // resize target container
    if (It != grSrc.mDictionary.end()){
        v_ItBeg = std::next(grSrc.vContainer.begin(), It->second);
        if (ItEnd != grSrc.mDictionary.end()){
            v_ItEnd = std::next(grSrc.vContainer.begin(), ItEnd->second);
        }
        vRes.reserve(std::distance(v_ItBeg,v_ItEnd));
    }

    while (It != ItEnd) { // loop throught the section contains data for target tic only

        v_ItBeg = std::next(grSrc.vContainer.begin(), It->second);

        if (ItCur != grSrc.mDictionary.end()){
            v_ItEnd = std::next(grSrc.vContainer.begin(), ItCur->second);
        }
        else{
            v_ItEnd = grSrc.vContainer.end();
        }

        Bar b (*v_ItBeg,iInterval);

        b = std::accumulate(std::next(v_ItBeg),v_ItEnd,
                        b,[](Bar &c, const auto &el){
                        return c.Append(el);
                        }
                    );

        vRes.push_back(b);

        ////
        It = ItCur;

        //dtNext  = Bar::DateAccommodate(dtNext,iInterval) + iInterval * 60;

        dtNext  = Bar::DateAccommodate(dtNext,iInterval,true);

        ItCur = grSrc.mDictionary.lower_bound(dtNext);
        if (ItCur != grSrc.mDictionary.end()){
            dtNext = ItCur->first;
        }
    }

    }
    catch (std::exception &e){
        ThreadFreeCout pcout;
        pcout <<"error: "<<e.what()<<"\n";
        return {};

    }
    /////
    bool bRes = AddBarsList(v,dtAccStart,dtAccEndLower);
    size_t iStart{vContainer.size()};

    if(bCopyToDst){
        grDst.clear();
        (void)grDst.AddBarsList(v,dtAccStart,dtAccEndLower);
        auto It (mDictionary.lower_bound(dtAccStart));
        if(It != mDictionary.end()){
            grDst.iShiftIndex = It->second;
        }
        else{
            grDst.iShiftIndex = vContainer.size();
        }
        iStart = grDst.iShiftIndex;
    }
    //////////////////////////////////////////////////////////////////////////
    if constexpr (std::is_same_v<T, Bar>){
//        if (vContainer.size() + 8 >  vMovingBlue.size())    vMovingBlue.resize  (vContainer.size() + 8);
//        if (vContainer.size() + 5 >  vMovingRed.size())     vMovingRed.resize   (vContainer.size() + 5);
//        if (vContainer.size() + 3 >  vMovingGreen.size())   vMovingGreen.resize (vContainer.size() + 3);
//        size_t iRecalc = iStart >= 8 ? iStart - 8: 0;
//        calculateMovingAverages(iRecalc);

        if(bCopyToDst){
            if (vMovingBlue.size() > iStart){
                auto ItBlueBeg(std::next(vMovingBlue.begin(),iStart));
                auto ItBlueEnd(vMovingBlue.end());
                grDst.vMovingBlue.clear();

                grDst.vMovingBlue.reserve(std::distance(ItBlueBeg,ItBlueEnd));
                std::copy(ItBlueBeg,ItBlueEnd,std::back_inserter(grDst.vMovingBlue));
            }
            //////////////////////////////////
            if (vMovingRed.size() > iStart){
                auto ItRedBeg(std::next(vMovingRed.begin(),iStart));
                auto ItRedEnd(vMovingRed.end());
                grDst.vMovingRed.clear();

                grDst.vMovingRed.reserve(std::distance(ItRedBeg,ItRedEnd));
                std::copy(ItRedBeg,ItRedEnd,std::back_inserter(grDst.vMovingRed));
            }
            //////////////////////////////////
            if (vMovingGreen.size() > iStart){
                auto ItGreenBeg(std::next(vMovingGreen.begin(),iStart));
                auto ItGreenEnd(vMovingGreen.end());
                grDst.vMovingGreen.clear();

                grDst.vMovingGreen.reserve(std::distance(ItGreenBeg,ItGreenEnd));
                std::copy(ItGreenBeg,ItGreenEnd,std::back_inserter(grDst.vMovingGreen));
            }
        }
    }
    //////////////////////////////////////////////////////////////////////////

    return bRes;
}
//------------------------------------------------------------------------------------------------------------
template<typename T>
std::size_t Graph<T>::GetUsedMemory() const
{
    return vContainer.size() * sizeof(T) + mDictionary.size() * (sizeof(std::time_t) + sizeof(size_t))
            + vMovingBlue.size() * sizeof (double) + vMovingRed.size() * sizeof (double) + vMovingGreen.size() * sizeof (double);
}
//------------------------------------------------------------------------------------------------------------
template<typename T>
void Graph<T>::calculateMovingAverages(size_t iStart)
{
    if constexpr (std::is_same_v<T, Bar>){

        //------------------------------------------
        if (iStart > 0) iStart--;
        //------------------------------------------

        const double dBluePeriod  {2.0/(25.0 + 1.0)};
        const double dRedPeriod   {2.0/(15.0 + 1.0)};
        const double dGreenPeriod {2.0/(9.0 + 1.0)};

        double dBlueLeft{0};
        double dRedLeft{0};
        double dGreenLeft{0};
        //------------------------------------------

        if(/*iStart >= 0 &&*/ iStart + 8 < vMovingBlue.size() && vMovingBlue[iStart + 8] != 0){
            dBlueLeft   = vMovingBlue[iStart + 8];
            dRedLeft    = vMovingRed[iStart + 5];
            dGreenLeft  = vMovingGreen[iStart + 3];
        }
        else{
            dBlueLeft   = (vContainer[iStart].High() + vContainer[iStart].Low())/2.0;
            dRedLeft    = dBlueLeft;
            dGreenLeft  = dBlueLeft;
        }
        //------------------------------------------
        iStart++;
        //------------------------------------------
        for (size_t i = iStart; i < vMovingBlue.size() - 8 /*&& i < vContainer.size()*/; ++i){
            vMovingBlue[i + 8] = dBlueLeft * (1 - dBluePeriod) + dBluePeriod * (vContainer[i].High() + vContainer[i].Low())/2.0;
            dBlueLeft = vMovingBlue[i + 8];
        }
        //--
        for (size_t i = iStart; i < vMovingRed.size() - 5 /*&& i < vContainer.size()*/; ++i){
            vMovingRed[i + 5] = dRedLeft * (1 - dRedPeriod) + dRedPeriod * (vContainer[i].High() + vContainer[i].Low())/2.0;
            dRedLeft = vMovingRed[i + 5];
        }
        //--
        for (size_t i = iStart; i < vMovingGreen.size() - 3 /*&& i < vContainer.size()*/; ++i){
            vMovingGreen[i + 3] = dGreenLeft * (1 - dGreenPeriod) + dGreenPeriod * (vContainer[i].High() + vContainer[i].Low())/2.0;
            dGreenLeft = vMovingGreen[i + 3];
        }
        //--
    }
}
//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
template<typename T>
std::tuple<double,double,unsigned long,unsigned long>  Graph<T>::getMinMax() const
{
    if (vContainer.size()==0) return {0,0,0,0};
    return std::accumulate(vContainer.begin(),
                           vContainer.end(),
                           std::tuple<double,double,unsigned long,unsigned long>{vContainer.front().Low(),vContainer.front().High(),vContainer.front().Volume(),vContainer.front().Volume()},
                           [](const auto &p,const auto &t){
                                return std::tuple<double,double,unsigned long,unsigned long>{
                                                      std::get<0>(p) < t.Low()  ? std::get<0>(p) : t.Low(),
                                                      std::get<1>(p) > t.High() ? std::get<1>(p) : t.High(),
                                                      std::get<2>(p) < t.Volume() ? std::get<2>(p) : t.Volume(),
                                                      std::get<3>(p) > t.Volume() ? std::get<3>(p) : t.Volume()
                                            };
                           });
}
//------------------------------------------------------------------------------------------------------------
template<typename T>
std::tuple<double,double,unsigned long,unsigned long>  Graph<T>::getMinMax(std::time_t dtStart,std::time_t dtEnd) const
{

    auto ItM (mDictionary.lower_bound(Bar::DateAccommodate(dtStart,this->iInterval)));
    if (ItM == mDictionary.end()){
        return  {0,0,0,0};
    }
    auto It (std::next(vContainer.begin(), ItM->second));

    auto ItEndM (mDictionary.lower_bound(Bar::DateAccommodate(dtEnd,this->iInterval)));
    auto ItEnd(vContainer.end());
    if (ItEndM != mDictionary.end()){
        ItEnd = std::next(vContainer.begin(), ItEndM->second);
    }

    if (vContainer.size()==0) return {0,0,0,0};
    return std::accumulate(It,
                           ItEnd,
                           std::tuple<double,double,unsigned long,unsigned long>{vContainer.front().Low(),vContainer.front().High(),vContainer.front().Volume(),vContainer.front().Volume()},
                           [](const auto &p,const auto &t){
                                return std::tuple<double,double,unsigned long,unsigned long>{
                                                      std::get<0>(p) < t.Low()  ? std::get<0>(p) : t.Low(),
                                                      std::get<1>(p) > t.High() ? std::get<1>(p) : t.High(),
                                                      std::get<2>(p) < t.Volume() ? std::get<2>(p) : t.Volume(),
                                                      std::get<3>(p) > t.Volume() ? std::get<3>(p) : t.Volume()
                                            };
                           });
};

#endif // GRAPH_H


