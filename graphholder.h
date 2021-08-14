#ifndef GRAPHHOLDER_H
#define GRAPHHOLDER_H


//#include<vector>
//#include<list>
//#include<map>
#include<mutex>
#include<shared_mutex>


#include "bar.h"
#include "graph.h"
#include "ticker.h"

class GraphHolder
{
private:
    const int iTickerID;

    std::shared_mutex mutexHolder;

    Graph<Bar> graphTick;
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

public:

    explicit GraphHolder(const int iTickerID = 0);
    GraphHolder(GraphHolder &&);



    //------------------------------------------------------------
    inline Bar & grTick(size_t i)       {return graphTick[i];};
    inline Bar & gr1(size_t i)          {return graph1[i];};
    inline Bar & gr5(size_t i)          {return graph5[i];};
    inline Bar & gr10(size_t i)         {return graph10[i];};
    inline Bar & gr15(size_t i)         {return graph15[i];};
    inline Bar & gr30(size_t i)         {return graph30[i];};
    inline Bar & gr60(size_t i)         {return graph60[i];};
    inline Bar & gr120(size_t i)        {return graph120[i];};
    inline Bar & gr180(size_t i)        {return graph180[i];};
    inline Bar & grDay(size_t i)        {return graphDay[i];};
    inline Bar & grWeek(size_t i)       {return graphWeek[i];};
    inline Bar & grMonth(size_t i)      {return graphMonth[i];};
    //------------------------------------------------------------

    bool AddBarsLists(std::vector<std::vector<Bar>> &v, std::time_t dtStart,std::time_t dtEnd);
    bool CheckMap();

};

#endif // GRAPHHOLDER_H
