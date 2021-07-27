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
    Graph();
    //--------------------------------------------------------------------------------------------------------
    void Add (Bar &b);
    void Add (std::list<Bar> &lst);
    //--------------------------------------------------------------------------------------------------------
};

#endif // GRAPH_H
