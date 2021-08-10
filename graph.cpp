#include "graph.h"
#include "threadfreecout.h"

#include <iterator>
#include<sstream>
#include<fstream>
#include<ostream>
#include<chrono>
#include<cstring>


//--------------------------------------------------------------------------------------------------------
Graph::Graph(Bar::eInterval Interval):iInterval{Interval}
{

}

//--------------------------------------------------------------------------------------------------------
void Graph::Add (Bar &b, bool bReplaceIfExists)
{
    if (b.Interval() != iInterval){
        std::stringstream ss;
        ss<<"Invalid interval value [Graph::Add (Bar &b)] {" << iInterval << "!=" << b.Interval() << "}";
        throw std::invalid_argument(ss.str());
    }
    //////
    auto It (mDictionary.find(b.Period()));
    if (It == mDictionary.end()){
        //vContainer.begin();

    }
    else{
        if (!bReplaceIfExists){
            std::stringstream ss;
            ss<<"Invalid bar period {" << b.Period() << "}: already exists";
            throw std::invalid_argument(ss.str());
        }
    }


}
//--------------------------------------------------------------------------------------------------------
void Graph::Add (std::list<Bar> &/*lst*/)
{
}
//--------------------------------------------------------------------------------------------------------
void Graph::AddTick (Bar &b, bool bNewSec)
{
    if (b.Interval() != iInterval){
        std::stringstream ss;
        ss<<"Invalid interval value [Graph::Add (Bar &b)] {" << iInterval << "!=" << b.Interval() << "}";
        throw std::invalid_argument(ss.str());
    }
    //////
}
//--------------------------------------------------------------------------------------------------------
void Graph::RemoveFromVector(std::vector<Bar> & vDst, std::time_t tStart, std::time_t tStop)
{
    int iSt = GetMoreThenIndex(vDst, tStart);
    int iEnd = GetMoreThenIndex(vDst, tStop + 1);


    auto It (std::next(vDst.begin(),iSt));
    auto ItEnd (std::next(vDst.begin(),iEnd));
    vDst.erase(It,ItEnd);

}
void Graph::InsertIntoVector(std::vector<Bar> & vDst, std::vector<Bar> & vSrc/*, std::time_t tStart*/)
{
    if (vSrc.size() <=0 ) return;

    std::time_t tStart = vSrc.front().Period()+1;
    int iSt = GetMoreThenIndex(vDst, tStart);

    // if copy to tail
    if (vDst.size() == 0 || vDst.back().Period() < vSrc.front().Period()){
        size_t iTail = vDst.size();

        vDst.resize(vDst.size() + vSrc.size());

        std::move(vSrc.begin(),vSrc.end(),std::next(vDst.begin(),iTail));

//        std::copy(std::next(vDst.begin(),iSt),
//                  vDst.end(),
//                  std::next(vDst.begin(),iTail)
//                  );

//        memcpy(((char*)vDst.data()) + iTail * sizeof (Bar),
//               ((char*)vDst.data()) + iSt   * sizeof (Bar),
//               vDst.size() * sizeof (Bar));

//        memcpy(((char*)vDst.data()) + iSt * sizeof (Bar),
//               ((char*)vSrc.data()),
//               vSrc.size() * sizeof (Bar));
    }



}

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
        if (iCount>100){
            ThreadFreeCout pcout;
            pcout <<"L:"<<iL<<"\n";
            pcout <<"R:"<<iR<<"\n";
            pcout <<"M:"<<middle<<"\n";
            break;
        }
    }
    return  iR;
}
//--------------------------------------------------------------------------------------------------------
