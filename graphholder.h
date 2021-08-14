#ifndef GRAPHHOLDER_H
#define GRAPHHOLDER_H


//#include<vector>
//#include<list>
//#include<map>
#include<mutex>
#include<shared_mutex>


#include "bar.h"
#include "bartick.h"
#include "graph.h"
#include "ticker.h"

template<typename T>
class GraphHolderIterator;

class GraphHolder
{
private:

    const int iTickerID;

    //mutable std::shared_mutex mutexHolder;

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

    std::map<Bar::eInterval,Graph<Bar>&> mpGraphs;


    template<typename T>
    friend class GraphHolderIterator;

public:

    explicit GraphHolder(const int iTickerID = 0);
    GraphHolder(GraphHolder &&);

    mutable std::shared_mutex mutexHolder;

    //------------------------------------------------------------
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
    size_t getViewGraphSize(const Bar::eInterval i) const;
    std::time_t getViewGraphDateMin(Bar::eInterval it);
    std::time_t getViewGraphDateMax(Bar::eInterval it);
    //------------------------------------------------------------


    template<typename T>
    GraphHolderIterator<T> beginIteratorByDate(Bar::eInterval it, std::time_t tBeg,bool & bSuccess){
        std::shared_lock lkT(mutexHolder,std::defer_lock);
        bSuccess = false;
        if (!lkT.try_lock())
            return GraphHolderIterator<T>{};
        bSuccess = true;
        return GraphHolderIterator<T>(lkT,*this, it, tBeg);
    }

    template<typename T>
    GraphHolderIterator<T> endIteratorByDate(Bar::eInterval it, std::time_t tBeg){
        return GraphHolderIterator<T>(std::shared_lock{mutexHolder,std::defer_lock},*this, it, tBeg);
    }

    template<typename T>
    GraphHolderIterator<T> end(){
        return GraphHolderIterator<T>{};
    }


    //------------------------------------------------------------
    bool AddBarsLists(std::vector<std::vector<BarTick>> &v, std::time_t dtStart,std::time_t dtEnd);
    bool CheckMap();
private:

    size_t getViewGraphIndex(const std::time_t t, const Bar::eInterval i) const;
    template<typename T>
    const T & getByIndex(const Bar::eInterval it,const size_t indx) const;

};

//-----------------------------------------------------------------------------------------------------------------------------------
template<typename T>
class GraphHolderIterator
{
    const GraphHolder * holder{nullptr};
    Bar::eInterval selectedViewInterval{Bar::eInterval::pTick};
    size_t iCurrentIndex{0};

    const bool bEndIterator;

    std::shared_lock<std::shared_mutex> lk;

public:
    //-------------------------------------
    explicit GraphHolderIterator():bEndIterator{true} {};
    //
    explicit GraphHolderIterator(std::shared_lock<std::shared_mutex>& lkT,const GraphHolder &hl,const  Bar::eInterval it,const  std::time_t tBeg):
       holder{&hl}
      ,selectedViewInterval{it}
      ,bEndIterator{false}
      ,lk {std::move(lkT)}
    {
        iCurrentIndex = holder->getViewGraphIndex(tBeg,selectedViewInterval);
    };
    //
    explicit GraphHolderIterator(const GraphHolderIterator &o) = delete;
    //
    explicit GraphHolderIterator(const GraphHolderIterator &&o):
       holder{o.holder}
      ,selectedViewInterval{o.selectedViewInterval}
      ,iCurrentIndex{o.iCurrentIndex}
      ,bEndIterator{o.bEndIterator}
      ,lk{std::move(o.lk)}
    {};
    //
    GraphHolderIterator & operator=(const GraphHolderIterator &o) = delete;
    //
    GraphHolderIterator & operator=(const GraphHolderIterator &&o) {

        holder                  = o.holder;
        selectedViewInterval    = o.selectedViewInterval;
        iCurrentIndex           = o.iCurrentIndex;
        bEndIterator            = o.bEndIterator;
        lk                      = std::move(o.lk);

        return *this;
    }
    //-------------------------------------
    bool operator==(const GraphHolderIterator &o) const{
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
    bool operator!=(const GraphHolderIterator &o) const{
        return !operator==(o);
    }
    //-------------------------------------
    GraphHolderIterator & operator++() {
        if(holder->getViewGraphSize(selectedViewInterval) > iCurrentIndex)
            iCurrentIndex++;
        return *this;
    }
    //-------------------------------------
    const T & operator*() {
        return (holder->getByIndex<T>(selectedViewInterval,iCurrentIndex));
    }
    //-------------------------------------
    GraphHolderIterator&  operator+(size_t t){
        if (iCurrentIndex + t > holder->getViewGraphSize(selectedViewInterval)){
            iCurrentIndex = holder->getViewGraphSize(selectedViewInterval);
        }
        else{
            iCurrentIndex +=t;
        }
        return *this;
    }
    //-------------------------------------
    inline Bar::eInterval Interval() const {return selectedViewInterval;}
    inline bool owns_lock()            const {return lk.owns_lock();}
};

//-----------------------------------------------------------------------------------------------------------------------------------
template<typename T>
const T & GraphHolder::getByIndex(const Bar::eInterval it,const size_t indx) const
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
//-----------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------



#endif // GRAPHHOLDER_H
