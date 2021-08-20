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
    ,mpGraphs{ {Bar::eInterval::p1,graph1}
              ,{Bar::eInterval::p5,graph5}
              ,{Bar::eInterval::p10,graph10}
              ,{Bar::eInterval::p15,graph15}
              ,{Bar::eInterval::p30,graph30}
              ,{Bar::eInterval::p60,graph60}
              ,{Bar::eInterval::p120,graph120}
              ,{Bar::eInterval::p180,graph180}
              ,{Bar::eInterval::pDay,graphDay}
              ,{Bar::eInterval::pWeek,graphWeek}
              ,{Bar::eInterval::pMonth,graphMonth}}
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
    ,mpGraphs{{Bar::eInterval::p1,graph1}
             ,{Bar::eInterval::p5,graph5}
             ,{Bar::eInterval::p10,graph10}
             ,{Bar::eInterval::p15,graph15}
             ,{Bar::eInterval::p30,graph30}
             ,{Bar::eInterval::p60,graph60}
             ,{Bar::eInterval::p120,graph120}
             ,{Bar::eInterval::p180,graph180}
             ,{Bar::eInterval::pDay,graphDay}
             ,{Bar::eInterval::pWeek,graphWeek}
             ,{Bar::eInterval::pMonth,graphMonth}}
{

}

//------------------------------------------------------------------------------------------------------
bool GraphHolder::AddBarsLists(std::vector<std::vector<BarTick>> &v, std::time_t dtStart,std::time_t dtEnd)
{
    std::unique_lock lk(mutexHolder);
    //
    bool bRet = graphTick.AddBarsList(v,dtStart,dtEnd);
    if (bRet){
        bRet = BuildUpperList(dtStart,dtEnd);
    }
    return bRet;

}
//------------------------------------------------------------------------------------------------------
bool GraphHolder::BuildUpperList(std::time_t dtStart,std::time_t dtEnd)
{
    graph1.BuildFromLowerList(graphTick, dtStart,dtEnd);

    return true;
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
size_t GraphHolder::getViewGraphSize(Bar::eInterval it) const
{
    if (it == Bar::eInterval::pTick)
        return graphTick.size();
    else
        return mpGraphs.at(it).size();
}
//------------------------------------------------------------------------------------------------------
std::time_t GraphHolder::getViewGraphDateMin(Bar::eInterval it)
{
    if (it == Bar::eInterval::pTick)
        return graphTick.GetDateMin();
    else
        return mpGraphs.at(it).GetDateMin();
}
//------------------------------------------------------------------------------------------------------
std::time_t GraphHolder::getViewGraphDateMax(Bar::eInterval it)
{
    if (it == Bar::eInterval::pTick)
        return graphTick.GetDateMax();
    else
        return mpGraphs.at(it).GetDateMax();
}
//------------------------------------------------------------------------------------------------------
size_t GraphHolder::getViewGraphIndex(const std::time_t t, const Bar::eInterval it) const
{
    if (it == Bar::eInterval::pTick)
        return graphTick.getIndex(t);
    else
        return mpGraphs.at(it).getIndex(t);
}
//------------------------------------------------------------------------------------------------------
//BarTick & GraphHolder::getByIndex(const Bar::eInterval it,const size_t indx)
//{
//    switch (it) {
//    case Bar::eInterval::pTick:
//        return graphTick[indx];
//    case Bar::eInterval::p1:
//        return graph1[indx];
//    default:
//        throw std::invalid_argument("Invalid viewInterval in FraphHolder");
//        break;
//    }
//}
//------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------

