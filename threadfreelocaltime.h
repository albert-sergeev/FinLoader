#ifndef TREADSAFELOCALTIME_H
#define TREADSAFELOCALTIME_H


#include<chrono>
#include<shared_mutex>

inline thread_local std::tm tmTmp{};
inline std::shared_mutex  mutexLocalTime;

////////////
/// the standard localtime points to a static pointer to tm, which is not safe
/// \param t
/// \return
///
template <typename T>
inline static std::tm *  threadfree_localtime(const T* t)
{
    std::unique_lock lk(mutexLocalTime);
    tmTmp = *std::localtime(t);
    return &tmTmp;
}


#endif // TREADSAFELOCALTIME_H
