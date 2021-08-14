#ifndef GRAPH_H
#define GRAPH_H

#include<vector>
#include<list>
#include<map>
#include<stdexcept>
#include<atomic>


#include "threadfreecout.h"
#include "bar.h"

/////////////////////////////////////////////
/// \brief Class for store trade graphics
/// elements - class Bar
/// elements counts by incremented index. Random access by time implimented by map dictionary
/// contains additional moving averages graphics and aligators
/// contains interfaces for adding new bars from single bar and from lists
///
/// removing not implemented (dont needed)
///
template<typename T>
class Graph
{


private:

    std::vector<T> vContainer;
    std::map<time_t,size_t> mDictionary;

    const Bar::eInterval iInterval;
    const int iTickerID;

public:

    inline size_t size() const {return vContainer.size(); };

public:
    //--------------------------------------------------------------------------------------------------------
    Graph() = delete;
    Graph(Graph&) = delete;
    Graph& operator=(Graph&) = delete;
    //--------------------------------------------------------------------------------------------------------
    explicit Graph(int TickerID, Bar::eInterval Interval):iInterval{Interval},iTickerID{TickerID}{;}
    explicit Graph(Graph&& o):
        vContainer{std::move(o.vContainer)}
       ,mDictionary{std::move(o.mDictionary)}
       ,iInterval{o.iInterval}
       ,iTickerID{o.iTickerID}{;}
    //--------------------------------------------------------------------------------------------------------
    Bar & operator[](size_t i) {if (/*i<0 ||*/ i>= vContainer.size()) {throw std::out_of_range("");} return vContainer[i];}
    //--------------------------------------------------------------------------------------------------------
//    void Add (Bar &b, bool bReplaceIfExists = true);
//    void AddTick (Bar &b, bool bNewSec);
//    void Add (std::list<Bar> &lst);
    bool AddBarsList(std::vector<std::vector<T>> &v, std::time_t dtStart,std::time_t dtEnd);

    bool CheckMap();

    std::string ToString();
    std::string ToStringPeriods();
    //
private:
    static size_t GetMoreThenIndex(std::vector<T> & v, std::time_t tT);
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
                ss<< "graph onterval [" <<this->iInterval<<"] != ["<<lst.front().Interval()<<"]";
                throw std::invalid_argument(ss.str());
            }
            if(lst.front().Period() < dtStart){
                std::stringstream ss;
                ss<< "the period of incoming data  [" <<lst.front().Period()<<"] is less than the beginning of the range ["<<dtStart<<"]";
                throw std::invalid_argument(ss.str());
            }
            if(lst.back().Period() > dtEnd){
                std::stringstream ss;
                ss<< "the period of incoming data  [" <<lst.back().Period()<<"] is greater than the window of the range ["<<dtEnd<<"]";
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

    if (bInRange){
        // if it's addition to the back:
        // 1. scroll through the lists
        // 2. try to look up in a dictionary because there may be multiple tickers in a second
        // 3. if not found, add a new entry to the map
        // 4. push back to vector
//        { ThreadFreeCout pcout; pcout<<"addition to tail\n";}

        for(const auto & lst:v){
            for(size_t i = 0; i < lst.size(); ++i){
                if (mDictionary.find(lst[i].Period()) == mDictionary.end()){
                    mDictionary[lst[i].Period()] = vContainer.size()+i;
                }
            }
            std::copy(lst.begin(),lst.end(),std::back_inserter(vContainer));
        }
    }
    else{
        // if it's an insert:
        // 1. find range to delete in the vector
        // 2. clear range to delete from map;
        // 3. resize vector
        // 4. insert new data into the vector


        // 1. look up range to delete in the vector
        size_t iStart = GetMoreThenIndex(vContainer, dtStart);
        size_t iEnd = GetMoreThenIndex(vContainer, dtEnd + 1); //

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
    return true;;
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
            return false;
        }
        if (e.second >0 && vContainer[e.second].Period() == vContainer[e.second-1].Period()){
            ThreadFreeCout pcout;
            pcout<<"vContainer[map[index]].period == vContainer[map[index]-1].period\n";
            return false;
        }
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
    }

    return true;
}

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
//------------------------------------------------------------------------------------------------------------

#endif // GRAPH_H
