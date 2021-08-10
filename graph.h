#ifndef GRAPH_H
#define GRAPH_H

#include<vector>
#include<list>
#include<map>

#include "bar.h"

/////////////////////////////////////////////
/// \brief Class for store trade graphics
/// elements - class Bar
/// elements counts by incremented index. Random access by time implimented by map dictionary
/// contains additional moving averages graphics and aligators
/// contains interfaces for adding new bars from single bar and from lists
///
/// removing not implemented (dont needed)
///
class Graph
{
private:

    std::vector<Bar> vContainer;
    std::map<time_t,int> mDictionary;

    Bar::eInterval iInterval;

public:

    inline size_t size() const {return vContainer.size(); };

public:
    //--------------------------------------------------------------------------------------------------------
    Graph() = delete;
    Graph(Graph&) = delete;
    Graph& operator=(Graph&) = delete;
    //--------------------------------------------------------------------------------------------------------
    explicit Graph(Bar::eInterval Interval);
    //--------------------------------------------------------------------------------------------------------
    void Add (Bar &b, bool bReplaceIfExists = true);
    void AddTick (Bar &b, bool bNewSec);
    void Add (std::list<Bar> &lst);
    //--------------------------------------------------------------------------------------------------------
    static void RemoveFromVector(std::vector<Bar> & vDst, std::time_t tStart, std::time_t tStop);
    static void InsertIntoVector(std::vector<Bar> & vDst, std::vector<Bar> & vSrc/*, std::time_t tStart*/);
    //--------------------------------------------------------------------------------------------------------
private:
    static size_t GetMoreThenIndex(std::vector<Bar> & v, std::time_t tT);
};

#endif // GRAPH_H
