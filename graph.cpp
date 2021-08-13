#include "graph.h"
#include "threadfreecout.h"

#include <iterator>
#include<sstream>
#include<fstream>
#include<ostream>
#include<chrono>
#include<cstring>


//--------------------------------------------------------------------------------------------------------
Graph::Graph(int TickerID, Bar::eInterval Interval):iInterval{Interval},iTickerID{TickerID}
{

}

//--------------------------------------------------------------------------------------------------------
//void Graph::Add (Bar &b, bool bReplaceIfExists)
//{
//    if (b.Interval() != iInterval){
//        std::stringstream ss;
//        ss<<"Invalid interval value [Graph::Add (Bar &b)] {" << iInterval << "!=" << b.Interval() << "}";
//        throw std::invalid_argument(ss.str());
//    }
//    //////
//    auto It (mDictionary.find(b.Period()));
//    if (It == mDictionary.end()){
//        //vContainer.begin();

//    }
//    else{
//        if (!bReplaceIfExists){
//            std::stringstream ss;
//            ss<<"Invalid bar period {" << b.Period() << "}: already exists";
//            throw std::invalid_argument(ss.str());
//        }
//    }


//}
//--------------------------------------------------------------------------------------------------------
//void Graph::Add (std::list<Bar> &/*lst*/)
//{
//}
//--------------------------------------------------------------------------------------------------------
//void Graph::AddTick (Bar &b, bool bNewSec)
//{
//    if (b.Interval() != iInterval){
//        std::stringstream ss;
//        ss<<"Invalid interval value [Graph::Add (Bar &b)] {" << iInterval << "!=" << b.Interval() << "}";
//        throw std::invalid_argument(ss.str());
//    }
//    //////
//}
//--------------------------------------------------------------------------------------------------------
//void Graph::RemoveFromVector(std::vector<Bar> & vDst, std::time_t tStart, std::time_t tStop)
//{
//    int iSt = GetMoreThenIndex(vDst, tStart);
//    int iEnd = GetMoreThenIndex(vDst, tStop + 1);


//    auto It (std::next(vDst.begin(),iSt));
//    auto ItEnd (std::next(vDst.begin(),iEnd));
//    vDst.erase(It,ItEnd);

//}
//void Graph::InsertIntoVector(std::vector<Bar> & vDst, std::vector<Bar> & vSrc/*, std::time_t tStart*/)
//{
//    if (vSrc.size() <=0 ) return;

//    std::time_t tStart = vSrc.front().Period()+1;
//    int iSt = GetMoreThenIndex(vDst, tStart);

//    // if copy to tail
//    if (vDst.size() == 0 || vDst.back().Period() < vSrc.front().Period()){
//        size_t iTail = vDst.size();

//        vDst.resize(vDst.size() + vSrc.size());

//        std::move(vSrc.begin(),vSrc.end(),std::next(vDst.begin(),iTail));

////        std::copy(std::next(vDst.begin(),iSt),
////                  vDst.end(),
////                  std::next(vDst.begin(),iTail)
////                  );

////        memcpy(((char*)vDst.data()) + iTail * sizeof (Bar),
////               ((char*)vDst.data()) + iSt   * sizeof (Bar),
////               vDst.size() * sizeof (Bar));

////        memcpy(((char*)vDst.data()) + iSt * sizeof (Bar),
////               ((char*)vSrc.data()),
////               vSrc.size() * sizeof (Bar));
//    }



//}

//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
//
/////
/// \brief Meaning that v is ordered. May be several bars with the same period.
/// \param v
/// \param tT
/// \return  index of bar with a later time than tT
///
size_t Graph::GetMoreThenIndex(std::vector<Bar> & v, std::time_t tT)
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
//        if (iCount>100){
//            ThreadFreeCout pcout;
//            pcout <<"L:"<<iL<<"\n";
//            pcout <<"R:"<<iR<<"\n";
//            pcout <<"M:"<<middle<<"\n";
//            break;
//        }
    }
    return  iR;
}
//--------------------------------------------------------------------------------------------------------

bool Graph::AddBarsList(std::vector<std::vector<Bar>> &v, std::time_t dtStart,std::time_t dtEnd)
{
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
    if(iNewLength <= 0) return true;
    //////////////////////////////////////////////////////////////////

    if (bInRange){
        // if it's addition to the back:
        // 1. scroll through the lists
        // 2. try to look up in a dictionary because there may be multiple tickers in a second
        // 3. if not found, add a new entry to the map
        // 4. push back to vector


        for(const auto & lst:v){
//            for(const Bar &b:lst){
//                if (mDictionary.find(b.Period()) == mDictionary.end()){
//                    mDictionary[b.Period()] = vContainer.size();
//                }
//                vContainer.push_back(b);
//            }
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

        //std::map<time_t,int> mDictionary;

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
//                (*ItStart) = (b);
//                ItStart++;
            }
            std::copy(lst.begin(),lst.end(),ItStart);
            std::next(ItStart,lst.size());
        }
        //
        if (mDictionary.size() > vContainer.size()){
            throw std::runtime_error("logic error in Graph::AddBarsList 3\n");
        }

    }
    return true;;
}
//--------------------------------------------------------------------------------------------------------
bool Graph::CheckMap()
{
    for (const auto &e:mDictionary){
        if(/*e.second <0 &&*/ e.second >=vContainer.size()){
            return false;
        }
        if (vContainer[e.second].Period() != e.first){
            return false;
        }
        if (e.second >0 && vContainer[e.second].Period() == vContainer[e.second-1].Period()){
            return false;
        }
    }
    return true;
}
//--------------------------------------------------------------------------------------------------------
std::string Graph::ToString()
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
//--------------------------------------------------------------------------------------------------------
std::string Graph::ToStringPeriods()
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
