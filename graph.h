#ifndef GRAPH_H
#define GRAPH_H

#include<vector>
#include<list>
#include<map>
#include<stdexcept>
#include<atomic>
#include<utility>
#include<numeric>


#include "threadfreecout.h"
#include "bar.h"
#include "threadpool.h"

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

    friend class Graph<Bar>;
    friend class Graph<BarTick>;

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
    T & operator[](const size_t i)  {if (/*i<0 ||*/ i>= vContainer.size()) {throw std::out_of_range("");} return vContainer[i];}
    //--------------------------------------------------------------------------------------------------------
//    void Add (Bar &b, bool bReplaceIfExists = true);
//    void AddTick (Bar &b, bool bNewSec);
//    void Add (std::list<Bar> &lst);
    bool AddBarsList(std::vector<std::vector<T>> &v, std::time_t dtStart,std::time_t dtEnd);

    template<typename T_SRC>
    bool BuildFromLowerList(Graph<T_SRC> &grSrc, std::time_t dtStart,std::time_t dtEnd);

    inline std::time_t GetDateMin() const  {return  vContainer.size()>0? vContainer.front().Period():0;};
    inline std::time_t GetDateMax() const  {return  vContainer.size()>0? vContainer.back().Period():0;};

//    std::pair<double,double>  getMinMax() const;
//    std::pair<double,double>  getMinMax(std::time_t dtStart,std::time_t dtEnd) const;
    std::tuple<double,double,unsigned long,unsigned long>  getMinMax() const;
    std::tuple<double,double,unsigned long,unsigned long>  getMinMax(std::time_t dtStart,std::time_t dtEnd) const;

    size_t getIndex(const std::time_t t) const;
    //--------------------------------------------------------------------------------------------------------
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
                ss<< "AddBarsList. graph interval [" <<this->iInterval<<"] != ["<<lst.front().Interval()<<"]";
                throw std::invalid_argument(ss.str());
            }
            if(lst.front().Period() < dtStart){
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
        size_t iStart = GetMoreThenIndex(vContainer, dtStart);
        size_t iEnd = GetMoreThenIndex(vContainer, dtEnd + 1); //


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

template<typename T> template<typename T_SRC>
//bool Graph<T>::BuildFromLowerList(Graph<BarTick> &grSrc, std::time_t dtStart,std::time_t dtEnd)
bool Graph<T>::BuildFromLowerList(Graph<T_SRC> &grSrc, std::time_t dtStart,std::time_t dtEnd)
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
    return bRes;
}
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
