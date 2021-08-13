#include "graphholder.h"

//------------------------------------------------------------------------------------------------------
GraphHolder::GraphHolder(int TickerID)://Bar::eInterval Interval
    iTickerID{TickerID}
    ,graphTick{TickerID, Bar::eInterval::pTick}
    ,graph1{TickerID, Bar::eInterval::p1}
    ,graph5{TickerID, Bar::eInterval::p5}
    ,graph10{TickerID, Bar::eInterval::p10}
    ,graph15{TickerID, Bar::eInterval::p15}
    ,graph30{TickerID, Bar::eInterval::p30}
    ,graph60{TickerID, Bar::eInterval::p60}
    ,graph120{TickerID, Bar::eInterval::p120}
    ,graph180{TickerID, Bar::eInterval::p180}
    ,graphDay{TickerID, Bar::eInterval::pDay}
    ,graphWeek{TickerID, Bar::eInterval::pWeek}
    ,graphMonth{TickerID, Bar::eInterval::pMonth}

{

}

//------------------------------------------------------------------------------------------------------
bool GraphHolder::AddBarsList(std::vector<std::vector<Bar>> &v, std::time_t dtStart,std::time_t dtEnd)
{
    std::unique_lock lk(mutexHolder);
    //
    return graphTick.AddBarsList(v,dtStart,dtEnd);

}
//------------------------------------------------------------------------------------------------------
