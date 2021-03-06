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

#ifndef GRAPHHOLDER_H
#define GRAPHHOLDER_H


//#include<vector>
//#include<list>
//#include<map>
#include<set>
#include<mutex>
#include<shared_mutex>


#include "bar.h"
#include "bartick.h"
#include "graph.h"
#include "ticker.h"

/////////////////////////////////////////////
/// \brief The GraphHolder class
/// Key class for LSM-tree
///
/// contains storage classes for Graph<> for all time intervals
/// contains iterator integrated with mutex to access to storage files and data in multithread environment
/// contains methods to operate/manipulate contained data (using member classes methods)
///

class GraphHolder
{
private:

    const int iTickerID;                        // key data - ticker ID

    mutable std::shared_mutex mutexHolder;      // main mutex for container

    // core data, based on intervals
    Graph<BarTick> graphTick;
    Graph<Bar> graph1;
    Graph<Bar> graph5;
    Graph<Bar> graph10;
    Graph<Bar> graph15;
    Graph<Bar> graph30;
    Graph<Bar> graph60;
    Graph<Bar> graph120;
    Graph<Bar> graph180;
    Graph<Bar> graphDay;
    Graph<Bar> graphWeek;
    Graph<Bar> graphMonth;


    std::map<Bar::eInterval,Graph<Bar>&> mpGraphs;  // construct to access data by index(interval)


public:
    //-----------------------------------------------------------------------------------------------------------------------------------
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Iterator implementation part begin
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    template<typename T>
    class Iterator
    {
        //GraphHolder * const holder;
        GraphHolder * holder;
        Bar::eInterval selectedViewInterval{Bar::eInterval::pTick};
        size_t iCurrentIndex{0};

        //const bool bEndIterator;
        bool bEndIterator;

        std::shared_lock<std::shared_mutex> lk;

    public:
        //-------------------------------------
        Iterator():holder{},bEndIterator{true}{;}
        //
        Iterator(GraphHolder &hl,const  Bar::eInterval it,const  std::time_t tBeg):
           holder{&hl}
          ,selectedViewInterval{it}
          ,bEndIterator{false}
          ,lk {hl.mutexHolder,std::defer_lock}
        {
            (void)lk.try_lock();
            iCurrentIndex = holder->getViewGraphIndex(tBeg,selectedViewInterval);
        };
        //
        Iterator(const Iterator &o):
            holder{o.holder}
           ,selectedViewInterval{o.selectedViewInterval}
           ,iCurrentIndex{o.iCurrentIndex}
           ,bEndIterator{o.bEndIterator}
        {
            if (!o.bEndIterator && o.owns_lock()){
                lk = {holder->mutexHolder,std::defer_lock};
                (void)lk.try_lock();
            }
        };
        //
        Iterator& operator=(const Iterator &o)
        {
            holder                   = o.holder;
            selectedViewInterval     = o.selectedViewInterval;
            iCurrentIndex            = o.iCurrentIndex;

            if (!o.bEndIterator){
                lk = {holder->mutexHolder,std::defer_lock};
                if ( o.owns_lock()) {
                    (void)lk.try_lock();
                }
            }
            else{
                lk = std::shared_lock<std::shared_mutex>{};
            }

            bEndIterator             = o.bEndIterator;
            return *this;
        };
        //-------------------------------------
        bool operator==(const Iterator &o) const{
            //
            if (bEndIterator && o.bEndIterator){
                return true;
            }
            else if (o.bEndIterator){
                if (holder->getViewGraphSize  (selectedViewInterval)   <= iCurrentIndex) return true;
                else return false;
            }
            else if (bEndIterator){
                if (o.holder->getViewGraphSize(o.selectedViewInterval) <= o.iCurrentIndex) return true;
                else return false;
            }
            else if(selectedViewInterval == o.selectedViewInterval && iCurrentIndex == o.iCurrentIndex){
                return true;
            }
            else{
                return false;
            }
        }
        //-------------------------------------
        bool operator!=(const Iterator &o) const{
            return !operator==(o);
        }
        //-------------------------------------
        Iterator & operator++() {
            if(holder->getViewGraphSize(selectedViewInterval) > iCurrentIndex)
                iCurrentIndex++;
            return *this;
        }
        //
        Iterator operator++(int) {
            Iterator It(*this);
            if(holder->getViewGraphSize(selectedViewInterval) > iCurrentIndex )
                iCurrentIndex++;
            return It;
        }
        //-------------------------------------
        int operator-(Iterator o) const{
            //it.iCurrentIndex;
            if (bEndIterator && o.bEndIterator){return 0;}
            else if (o.bEndIterator){
                return iCurrentIndex -  holder->getViewGraphSize  (selectedViewInterval) ;
            }
            else if (bEndIterator){
                return holder->getViewGraphSize  (selectedViewInterval)  - o.iCurrentIndex;
            }
            else{
                return iCurrentIndex - o.iCurrentIndex;
            }
        }
        //-------------------------------------
        T & operator*()  {
            return (holder->getByIndex<T>(selectedViewInterval,iCurrentIndex));
        }
        //
        T * operator->() const {
            return &(holder->getByIndex<T>(selectedViewInterval,iCurrentIndex));
        }
        //-------------------------------------
        inline Bar::eInterval Interval() const {return selectedViewInterval;}
        inline size_t realPosition()     const {return iCurrentIndex;}
        inline bool owns_lock()          const {return !bEndIterator? lk.owns_lock():false;}
        inline void unlock()              { if(!bEndIterator) lk.unlock();}
    };

    //------------------------------------------------------------


    template<typename T>
    Iterator<T> beginIteratorByDate(Bar::eInterval Interval, std::time_t tBeg,bool & bSuccess){
        Iterator<T> It (*this, Interval, tBeg);
        bSuccess = false;
        if (!It.owns_lock())
            return Iterator<T>{};
        bSuccess = true;
        return It;
    }

    template<typename T>
    Iterator<T> endIteratorByDate(Bar::eInterval it, std::time_t tBeg){
        return Iterator<T>(*this, it, tBeg);
    }

    template<typename T>
    Iterator<T> end(){
        return Iterator<T>{};
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Iterator implementation part end
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:

    //------------------------------------------------------------
    // constructors
    explicit GraphHolder(const int iTickerID = 0);
    GraphHolder(GraphHolder &&);

    //------------------------------------------------------------
    void clear()    {
                    graphMonth.clear();
                    graphWeek.clear();
                    graphDay.clear();
                    graph180.clear();
                    graph120.clear();
                    graph60.clear();
                    graph30.clear();
                    graph15.clear();
                    graph10.clear();
                    graph5.clear();
                    graph1.clear();
                    graphTick.clear();
                    };
    //------------------------------------------------------------
    // get access methods
    inline const BarTick & grTick(size_t i)   {return graphTick[i];};
    inline const Bar & gr1(size_t i)          {return graph1[i];};
    inline const Bar & gr5(size_t i)          {return graph5[i];};
    inline const Bar & gr10(size_t i)         {return graph10[i];};
    inline const Bar & gr15(size_t i)         {return graph15[i];};
    inline const Bar & gr30(size_t i)         {return graph30[i];};
    inline const Bar & gr60(size_t i)         {return graph60[i];};
    inline const Bar & gr120(size_t i)        {return graph120[i];};
    inline const Bar & gr180(size_t i)        {return graph180[i];};
    inline const Bar & grDay(size_t i)        {return graphDay[i];};
    inline const Bar & grWeek(size_t i)       {return graphWeek[i];};
    inline const Bar & grMonth(size_t i)      {return graphMonth[i];};
    //------------------------------------------------------------
    // utility methods to access data

    size_t getViewGraphSize(const Bar::eInterval i) const;
    size_t getViewGraphIndex(const std::time_t t, const Bar::eInterval i) const;

    std::time_t getViewGraphDateMin(Bar::eInterval it);
    std::time_t getViewGraphDateMax(Bar::eInterval it);

    std::tuple<double,double,unsigned long,unsigned long>  getMinMax(const Bar::eInterval it) const;
    std::tuple<double,double,unsigned long,unsigned long>  getMinMax(const Bar::eInterval it, const  std::time_t dtStart, const std::time_t dtEnd) const;


    std::time_t getTimeByIndex(const Bar::eInterval it,const size_t indx);
    template<typename T>
    T & getByIndex(const Bar::eInterval it,const size_t indx);

    double getMovingBlueByIndex(const Bar::eInterval it,const size_t indx) const;
    double getMovingRedByIndex(const Bar::eInterval it,const size_t indx) const;
    double getMovingGreenByIndex(const Bar::eInterval it,const size_t indx) const;

    double getMovingBlueSize(const Bar::eInterval it) const;
    double getMovingRedSize(const Bar::eInterval it) const;
    double getMovingGreenSize(const Bar::eInterval it) const;


    //------------------------------------------------------------
    // construct and manipulate utilities:

    bool AddBarsLists(std::vector<std::vector<BarTick>> &v, std::time_t dtStart,std::time_t dtEnd);
    bool AddBarsListsFast(std::vector<BarTick> &v, std::set<std::time_t>   & stHolderTimeSet,std::pair<std::time_t,std::time_t> &pairRange,GraphHolder &grDest);
    bool CheckMap(); // data integrity check method
    bool shrink_extras_left(std::time_t dtEnd);

    std::size_t getShiftIndex(Bar::eInterval it)  const;

    //------------------------------------------------------------
    // method to access contained grapth
    template<typename T>
    const Graph<T>& getGraph(Bar::eInterval it) const;

    //------------------------------------------------------------
    // clone method
    bool CloneHolder(std::shared_ptr<GraphHolder>  &hlNew, const Bar::eInterval it, const size_t iStart,const size_t iEnd, const size_t LetShift);


    //------------------------------------------------------------
    // used memory calculate
    bool GetUsedMemory(std::size_t &iSize) const;

protected:

    bool BuildUpperList(std::time_t dtStart,std::time_t dtEnd, bool bCopyToDst,GraphHolder &grDest);

};
//-----------------------------------------------------------------------------------------------------------------------------------
namespace std {
template <typename T>
struct iterator_traits<GraphHolder::Iterator<T>> {
    using iterator_category = std::forward_iterator_tag;
    using value_type = T;
    using difference_type = int;

};
}

//-----------------------------------------------------------------------------------------------------------------------------------
template<typename T>
T & GraphHolder::getByIndex(const Bar::eInterval it,const size_t indx)
{
    if constexpr (std::is_same_v<T, Bar>){
        return mpGraphs.at(it)[indx];
    }
    else{
        if(it == Bar::eInterval::pTick)
            return graphTick[indx];
        else{
            throw std::out_of_range("GraphHolder::getByIndex invalid interval");
        }
    }
}
//-----------------------------------------------------------------------------------------------------------------------------------
template<typename T>
const Graph<T>& GraphHolder::getGraph(Bar::eInterval it) const
{
    if constexpr (std::is_same_v<T, Bar>){
        return mpGraphs.at(it);
    }
    else{
        if(it == Bar::eInterval::pTick)
            return graphTick;
        else{
            throw std::out_of_range("GraphHolder::getGraph invalid interval for template<BarTick>");
        }
    }
}
//-----------------------------------------------------------------------------------------------------------------------------------



#endif // GRAPHHOLDER_H
