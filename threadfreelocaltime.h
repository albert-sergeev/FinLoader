#ifndef TREADSAFELOCALTIME_H
#define TREADSAFELOCALTIME_H


#include<chrono>
#include<shared_mutex>
#include<string>
#include <QDate>
#include <QTime>



inline thread_local std::tm tmTmp{};
inline std::shared_mutex  mutexLocalTime;

//---------------------------------------------------------------------------------
template <typename T>
inline static std::tm *  threadfree_gmtime(const T* t)
{

    #ifdef _WIN32
        (void)gmtime_s(&tmTmp,t);
    #else
        std::unique_lock lk(mutexLocalTime);
        tmTmp = *std::gmtime(t);
    #endif

    return &tmTmp;
}
//---------------------------------------------------------------------------------
template <typename T>
inline static std::string threadfree_gmtime_to_str(const T* t)
{

    #ifdef _WIN32
        (void)gmtime_s(&tmTmp,t);
    #else
        std::unique_lock lk(mutexLocalTime);
        tmTmp = *std::gmtime(t);
    #endif
    char buffer[100];
    std::strftime(buffer, 100, "%Y/%m/%d %H:%M:%S", &tmTmp);
    std::string strRet(buffer);

    return strRet;
}
//---------------------------------------------------------------------------------
template <typename T>
inline static std::string threadfree_gmtime_date_to_str(const T* t)
{

    #ifdef _WIN32
        (void)gmtime_s(&tmTmp,t);
    #else
        std::unique_lock lk(mutexLocalTime);
        tmTmp = *std::gmtime(t);
    #endif

    char buffer[100];
    std::strftime(buffer, 100, "%Y/%m/%d", &tmTmp);
    std::string strRet(buffer);

    return strRet;
}
//---------------------------------------------------------------------------------
template <typename T>
inline static std::string threadfree_gmtime_time_to_str(const T* t)
{

    #ifdef _WIN32
        (void)gmtime_s(&tmTmp,t);
    #else
        std::unique_lock lk(mutexLocalTime);
        tmTmp = *std::gmtime(t);
    #endif

    char buffer[100];
    std::strftime(buffer, 100, "%H:%M:%S", &tmTmp);
    //std::strftime(buffer, 100, "%H:%M", &tmTmp);
    std::string strRet(buffer);

    return strRet;
}

//---------------------------------------------------------------------------------
inline time_t mktime_gm(struct tm * t)
/* struct tm to seconds since Unix epoch */
{
    long year;
    time_t result;
#define MONTHSPERYEAR   12      /* months per calendar year */
    static const int cumdays[MONTHSPERYEAR] =
        { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };

    /*@ +matchanyintegral @*/
    year = 1900 + t->tm_year + t->tm_mon / MONTHSPERYEAR;
    result = (year - 1970) * 365 + cumdays[t->tm_mon % MONTHSPERYEAR];
    result += (year - 1968) / 4;
    result -= (year - 1900) / 100;
    result += (year - 1600) / 400;
    if ((year % 4) == 0 && ((year % 100) != 0 || (year % 400) == 0) &&
        (t->tm_mon % MONTHSPERYEAR) < 2)
        result--;
    result += t->tm_mday - 1;
    result *= 24;
    result += t->tm_hour;
    result *= 60;
    result += t->tm_min;
    result *= 60;
    result += t->tm_sec;
    if (t->tm_isdst == 1)
        result -= 3600;
    /*@ -matchanyintegral @*/
    return (result);
}
//---------------------------------------------------------------------------------
inline QDate stdTimeToQDate(std::time_t& t)
{
    std::tm* tmSt=threadfree_gmtime(&t);
    return QDate(tmSt->tm_year+1900,tmSt->tm_mon+1,tmSt->tm_mday);
}
//---------------------------------------------------------------------------------
inline QTime stdTimeToQTime(std::time_t& t)
{
    std::tm* tmSt=threadfree_gmtime(&t);
    return QTime(tmSt->tm_hour,tmSt->tm_min,tmSt->tm_sec);
}
//---------------------------------------------------------------------------------
inline std::time_t QDateToStdTime(QDate dt)
{
    std::tm tmSt;
    {
        tmSt.tm_year   = dt.year() - 1900;
        tmSt.tm_mon    = dt.month() - 1;
        tmSt.tm_mday   = dt.day();
        tmSt.tm_hour   = 0;
        tmSt.tm_min    = 0;
        tmSt.tm_sec    = 0;
        tmSt.tm_isdst  = 0;
    }
    return (mktime_gm(&tmSt));
}
//---------------------------------------------------------------------------------

/*
 * const QTime tmS(ui->dateTimeStart->time());

*/

//////////////
///// the standard localtime points to a static pointer to tm, which is not safe
///// \param t
///// \return
/////
//template <typename T>
//inline static std::tm *  threadfree_localtime(const T* t)
//{
//    std::unique_lock lk(mutexLocalTime);
//    tmTmp = *std::localtime(t);
//    return &tmTmp;
//}

//template <typename T>
//inline static std::string  threadfree_localtime_to_str(const T* t)
//{
//    std::unique_lock lk(mutexLocalTime);
//    tmTmp = *std::localtime(t);
//    char buffer[100];
//    std::strftime(buffer, 100, "%Y/%m/%d %H:%M:%S", &tmTmp);
//    std::string strRet(buffer);

//    return strRet;
//}

//template <typename T>
//inline static std::string  threadfree_localtime_date_to_str(const T* t)
//{
//    std::unique_lock lk(mutexLocalTime);
//    tmTmp = *std::localtime(t);
//    char buffer[100];
//    std::strftime(buffer, 100, "%Y/%m/%d", &tmTmp);
//    std::string strRet(buffer);

//    return strRet;
//}


#endif // TREADSAFELOCALTIME_H
