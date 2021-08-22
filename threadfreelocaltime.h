#ifndef TREADSAFELOCALTIME_H
#define TREADSAFELOCALTIME_H


#include<chrono>
#include<shared_mutex>
#include<string>

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

template <typename T>
inline static std::string  threadfree_localtime_to_str(const T* t)
{
    std::unique_lock lk(mutexLocalTime);
    tmTmp = *std::localtime(t);
    char buffer[100];
    std::strftime(buffer, 100, "%Y/%m/%d %H:%M:%S", &tmTmp);
    std::string strRet(buffer);

    return strRet;
}

template <typename T>
inline static std::string  threadfree_localtime_date_to_str(const T* t)
{
    std::unique_lock lk(mutexLocalTime);
    tmTmp = *std::localtime(t);
    char buffer[100];
    std::strftime(buffer, 100, "%Y/%m/%d", &tmTmp);
    std::string strRet(buffer);

    return strRet;
}


#endif // TREADSAFELOCALTIME_H
