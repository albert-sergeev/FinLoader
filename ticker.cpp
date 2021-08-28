#include "ticker.h"

std::map<std::time_t,std::pair<std::time_t,std::map<std::time_t,std::time_t>>>
Market::buildDefaultSessionsMap(){

    std::tm tmPer;
    {
        tmPer.tm_year   = 1990 - 1900;
        tmPer.tm_mon    = 1 - 1;
        tmPer.tm_mday   = 1;
        tmPer.tm_hour   = 0;
        tmPer.tm_min    = 0;
        tmPer.tm_sec    = 0;
        tmPer.tm_isdst  = 0;
    }


    std::time_t t   = std::mktime(&tmPer);

    tmPer.tm_hour   = 10;
    std::time_t t1_1   = std::mktime(&tmPer);

    tmPer.tm_hour   = 18;
    tmPer.tm_min    = 39;
    std::time_t t1_2   = std::mktime(&tmPer);

    tmPer.tm_hour   = 19;
    tmPer.tm_min    = 05;
    std::time_t t2_1   = std::mktime(&tmPer);

    tmPer.tm_hour   = 23;
    tmPer.tm_min    = 49;
    std::time_t t2_2   = std::mktime(&tmPer);


    tmPer.tm_year   = 2100 - 1900;
    tmPer.tm_hour   = 0;
    tmPer.tm_min    = 0;
    tmPer.tm_isdst  = 0;

    std::time_t tE   = std::mktime(&tmPer);


    std::map<std::time_t,std::pair<std::time_t, std::map<std::time_t,std::time_t> >> m{{t,{tE,{{t1_1,t1_2},{t2_1,t2_2}}}}};

    return m;
};
//--------------------------------------------------------------------------------------------------------
bool IsInSessionRange(std::map<std::time_t,std::pair<std::time_t,std::map<std::time_t,std::time_t>>> &m, std::time_t &t)
{
    auto It (m.lower_bound(t));
    if( It!= m.end()){ // search left border
        if (It->second.first <= t){ // if we are in right border

            std::tm tmTmp = *threadfree_localtime(&t);
            std::tm tmAcc;
            {
                tmAcc.tm_year   = 1990 - 1900;
                tmAcc.tm_mon    = 1 - 1;
                tmAcc.tm_mday   = 1;
                tmAcc.tm_hour   = tmTmp.tm_hour;
                tmAcc.tm_min    = tmTmp.tm_min;
                tmAcc.tm_sec    = tmTmp.tm_sec;
                tmAcc.tm_isdst  = 0;
            }

            std::time_t tAcc = std::mktime(&tmAcc); // accomodate time (cut date, left  time only)

            auto ItIn (It->second.second.lower_bound(tAcc)); // search time ranges
            if (ItIn != It->second.second.end()){
                if (ItIn->second <= tAcc){ //if we are in then true
                    return true;
                }
            }
        }
    }
    return false;
}
//--------------------------------------------------------------------------------------------------------

