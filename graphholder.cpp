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
    ,graphTick  {std::move(o.graphTick)}
    ,graph1     {std::move(o.graph1)}
    ,graph5     {std::move(o.graph5)}
    ,graph10    {std::move(o.graph10)}
    ,graph15    {std::move(o.graph15)}
    ,graph30    {std::move(o.graph30)}
    ,graph60    {std::move(o.graph60)}
    ,graph120   {std::move(o.graph120)}
    ,graph180   {std::move(o.graph180)}
    ,graphDay   {std::move(o.graphDay)}
    ,graphWeek  {std::move(o.graphWeek)}
    ,graphMonth {std::move(o.graphMonth)}
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
bool GraphHolder::shrink_extras_left(std::time_t dtEnd)
{
    std::unique_lock lk(mutexHolder);

    bool bRet = graphTick.shrink_extras_left(dtEnd);
    if (bRet) bRet = graph1.shrink_extras_left(dtEnd);
    if (bRet) bRet = graph5.shrink_extras_left(dtEnd);
    if (bRet) bRet = graph10.shrink_extras_left(dtEnd);
    if (bRet) bRet = graph15.shrink_extras_left(dtEnd);
    if (bRet) bRet = graph30.shrink_extras_left(dtEnd);
    if (bRet) bRet = graph60.shrink_extras_left(dtEnd);
    if (bRet) bRet = graph120.shrink_extras_left(dtEnd);
    if (bRet) bRet = graph180.shrink_extras_left(dtEnd);
    if (bRet) bRet = graphDay.shrink_extras_left(dtEnd);
    if (bRet) bRet = graphMonth.shrink_extras_left(dtEnd);

    return bRet;
}
//------------------------------------------------------------------------------------------------------
bool GraphHolder::AddBarsLists(std::vector<std::vector<BarTick>> &v, std::time_t dtStart,std::time_t dtEnd)
{
    std::unique_lock lk(mutexHolder);
    //
    bool bRet = graphTick.AddBarsList(v,dtStart,dtEnd);
    if (bRet){
        GraphHolder hlTmp(iTickerID);
        bRet = BuildUpperList(dtStart,dtEnd,false,hlTmp);
    }
    return bRet;
}
//------------------------------------------------------------------------------------------------------
bool GraphHolder::AddBarsListsFast(std::vector<BarTick> &v,
                                   std::set<std::time_t>   & stHolderTimeSet,
                                   std::pair<std::time_t,
                                   std::time_t> &pairRange,
                                   GraphHolder &grDest)
{
    pairRange = {0,0};

    std::unique_lock lk(mutexHolder,std::defer_lock);
    while(!lk.try_lock()){
        if (this_thread_flagInterrup.isSet())   {return false;}
        std::this_thread::yield();
        if (this_thread_flagInterrup.isSet())   {return false;}
    }
    //////////////////////////////////////
    bool bRet{true};
    if (!v.empty()){
        bRet = graphTick.AddBarsListsFast(v,stHolderTimeSet,pairRange,grDest.graphTick);
        if (bRet){
            bRet = BuildUpperList(v.front().Period(),v.back().Period(),true,grDest);
        }
    }

    return bRet;
}
//------------------------------------------------------------------------------------------------------
bool GraphHolder::BuildUpperList(std::time_t dtStart,std::time_t dtEnd, bool bCopyToDst,GraphHolder &grDest)
{
    graph1.BuildFromLowerList(graphTick, dtStart,dtEnd,bCopyToDst,grDest.graph1);
    graph5.BuildFromLowerList(graph1, dtStart,dtEnd,bCopyToDst,grDest.graph5);
    graph10.BuildFromLowerList(graph5, dtStart,dtEnd,bCopyToDst,grDest.graph10);
    graph15.BuildFromLowerList(graph5, dtStart,dtEnd,bCopyToDst,grDest.graph15);
    graph30.BuildFromLowerList(graph15, dtStart,dtEnd,bCopyToDst,grDest.graph30);
    graph60.BuildFromLowerList(graph30, dtStart,dtEnd,bCopyToDst,grDest.graph60);
    graph120.BuildFromLowerList(graph60, dtStart,dtEnd,bCopyToDst,grDest.graph120);
    graph180.BuildFromLowerList(graph60, dtStart,dtEnd,bCopyToDst,grDest.graph180);
    graphDay.BuildFromLowerList(graph60, dtStart,dtEnd,bCopyToDst,grDest.graphDay);
    graphWeek.BuildFromLowerList(graphDay, dtStart,dtEnd,bCopyToDst,grDest.graphWeek);
    graphMonth.BuildFromLowerList(graphDay, dtStart,dtEnd,bCopyToDst,grDest.graphMonth);

    return true;
}
//-----------------------------------------------------------------------------------------------------------------------------------
bool GraphHolder::CloneHolder(std::shared_ptr<GraphHolder>  &hlNew, const Bar::eInterval it, const size_t iStart,const size_t iEnd, const size_t LetShift)
{
    std::shared_lock lk(mutexHolder,std::defer_lock);
    if(!lk.try_lock()) return false;

    hlNew = std::make_shared<GraphHolder>(GraphHolder{iTickerID});

    if (it == Bar::eInterval::pTick){
        graphTick.CloneGraph(hlNew->graphTick,iStart, iEnd,LetShift);
    }
    else{
        mpGraphs.at(it).CloneGraph(hlNew->mpGraphs.at(it),iStart, iEnd,LetShift);
    }
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
std::tuple<double,double,unsigned long,unsigned long>  GraphHolder::getMinMax(const Bar::eInterval it) const
{
    if(it == Bar::eInterval::pTick)
        return graphTick.getMinMax();
    else{
        return  mpGraphs.at(it).getMinMax();
    }
};
//-----------------------------------------------------------------------------------------------------------------------------------
std::tuple<double,double,unsigned long,unsigned long>  GraphHolder::getMinMax(const Bar::eInterval it, const  std::time_t dtStart, const std::time_t dtEnd) const
{
    if(it == Bar::eInterval::pTick)
        return graphTick.getMinMax(dtStart,dtEnd);
    else{
        return  mpGraphs.at(it).getMinMax(dtStart,dtEnd);
    }
};
//------------------------------------------------------------------------------------------------------
std::size_t GraphHolder::getShiftIndex(Bar::eInterval it)  const
{
    if(it == Bar::eInterval::pTick)
        return graphTick.GetShiftIndex();
    else{
        return  mpGraphs.at(it).GetShiftIndex();;
    }
}
//------------------------------------------------------------------------------------------------------
std::time_t GraphHolder::getTimeByIndex(const Bar::eInterval it,const size_t indx)
{
    if(it == Bar::eInterval::pTick){
        BarTick & b = getByIndex<BarTick>(it,indx);
        return b.Period();
    }
    else{
        Bar & b = getByIndex<Bar>(it,indx);
        return b.Period();
    }
}
//------------------------------------------------------------------------------------------------------
bool GraphHolder::GetUsedMemory(std::size_t &iSize) const
{
    iSize = 0;
    std::shared_lock lk(mutexHolder,std::defer_lock);
    if(lk.try_lock()){
        iSize += graphTick.GetUsedMemory();
        iSize += graph1.GetUsedMemory();
        iSize += graph5.GetUsedMemory();
        iSize += graph10.GetUsedMemory();
        iSize += graph15.GetUsedMemory();
        iSize += graph30.GetUsedMemory();
        iSize += graph60.GetUsedMemory();
        iSize += graph120.GetUsedMemory();
        iSize += graph180.GetUsedMemory();
        iSize += graphDay.GetUsedMemory();
        iSize += graphWeek.GetUsedMemory();
        iSize += graphMonth.GetUsedMemory();
        return true;
    }
    else{
        return false;
    }
}
//------------------------------------------------------------------------------------------------------
double GraphHolder::getMovingBlueByIndex(const Bar::eInterval it,const size_t indx) const
{
    if(it == Bar::eInterval::pTick){
        std::stringstream ss;
        ss <<"getMovingBlueByIndex invalid argument. Wrong interval: {"<< it<<"}";
        throw std::invalid_argument(ss.str());
    }
    else{
        return  mpGraphs.at(it).movingBlue(indx);
    }
}
//------------------------------------------------------------------------------------------------------
double GraphHolder::getMovingRedByIndex(const Bar::eInterval it,const size_t indx) const
{
    if(it == Bar::eInterval::pTick){
        std::stringstream ss;
        ss <<"getMovingRedByIndex invalid argument. Wrong interval: {"<< it<<"}";
        throw std::invalid_argument(ss.str());
    }
    else{
        return  mpGraphs.at(it).movingRed(indx);
    }
}
//------------------------------------------------------------------------------------------------------
double GraphHolder::getMovingGreenByIndex(const Bar::eInterval it,const size_t indx) const
{
    if(it == Bar::eInterval::pTick){
        std::stringstream ss;
        ss <<"getMovingGreenByIndex invalid argument. Wrong interval: {"<< it <<"}";
        throw std::invalid_argument(ss.str());
    }
    else{
        return  mpGraphs.at(it).movingGreen(indx);
    }
}
//------------------------------------------------------------------------------------------------------
double GraphHolder::getMovingBlueSize(const Bar::eInterval it) const
{
    if(it == Bar::eInterval::pTick){
        std::stringstream ss;
        ss <<"getMovingBlueByIndex invalid argument. Wrong interval: {"<< it<<"}";
        throw std::invalid_argument(ss.str());
    }
    else{
        return  mpGraphs.at(it).getMovingBlueSize();
    }
}
//------------------------------------------------------------------------------------------------------
double GraphHolder::getMovingRedSize(const Bar::eInterval it) const
{
    if(it == Bar::eInterval::pTick){
        std::stringstream ss;
        ss <<"getMovingBlueByIndex invalid argument. Wrong interval: {"<< it<<"}";
        throw std::invalid_argument(ss.str());
    }
    else{
        return  mpGraphs.at(it).getMovingRedSize();
    }
}
//------------------------------------------------------------------------------------------------------
double GraphHolder::getMovingGreenSize(const Bar::eInterval it) const
{
    if(it == Bar::eInterval::pTick){
        std::stringstream ss;
        ss <<"getMovingBlueByIndex invalid argument. Wrong interval: {"<< it<<"}";
        throw std::invalid_argument(ss.str());
    }
    else{
        return  mpGraphs.at(it).getMovingGreenSize();
    }
}
//------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------

