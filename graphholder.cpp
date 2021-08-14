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
GraphHolder::GraphHolder(GraphHolder && o):
    iTickerID{o.iTickerID}
    ,graphTick{std::move(o.graphTick)}
    ,graph1{std::move(o.graph1)}
    ,graph5{std::move(o.graph1)}
    ,graph10{std::move(o.graph1)}
    ,graph15{std::move(o.graph1)}
    ,graph30{std::move(o.graph1)}
    ,graph60{std::move(o.graph1)}
    ,graph120{std::move(o.graph1)}
    ,graph180{std::move(o.graph1)}
    ,graphDay{std::move(o.graph1)}
    ,graphWeek{std::move(o.graph1)}
    ,graphMonth{std::move(o.graph1)}
{

}

//------------------------------------------------------------------------------------------------------
bool GraphHolder::AddBarsLists(std::vector<std::vector<BarTick>> &v, std::time_t dtStart,std::time_t dtEnd)
{
    std::unique_lock lk(mutexHolder);
    //
    bool bRet = graphTick.AddBarsList(v,dtStart,dtEnd);
    return bRet;

}
//------------------------------------------------------------------------------------------------------
bool GraphHolder::CheckMap()
{
    std::shared_lock lk(mutexHolder);

    if (!graphTick.CheckMap())  return false;
    if (!graph1.CheckMap())     return false;
    if (!graph5.CheckMap())     return false;
    if (!graph10.CheckMap())    return false;
    if (!graph15.CheckMap())    return false;
    if (!graph30.CheckMap())    return false;
    if (!graph60.CheckMap())    return false;
    if (!graph120.CheckMap())   return false;
    if (!graph180.CheckMap())   return false;
    if (!graphDay.CheckMap())   return false;
    if (!graphWeek.CheckMap())  return false;
    if (!graphMonth.CheckMap()) return false;

    return true;
}
//------------------------------------------------------------------------------------------------------
