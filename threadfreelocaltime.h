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

#ifndef TREADSAFELOCALTIME_H
#define TREADSAFELOCALTIME_H


#include<chrono>
#include<shared_mutex>
#include<string>
#include <QDate>
#include <QTime>

/////////////////////////////////////////////////////////
/// usefull function set for thread safe time manipulation
///
/// its focused on working with GMT localtime
///
/// all data received from markets usualy have locale of market, so if we will translate it to our locale it will be a pile of rubbish
/// or we will need to do backward operation, control market locale and so on. And why to do useless work?
///
/// so we assume that we are virtually in the market locale and manipulate the time as if it were GMT (and we are at GMT, of course)
///
/// and by the way do cover for threadsafe
///
/////////////////////////////////////////////////////////


//////////////////////////////////////////////////
/// thread_local structure for std::gmtime results
///
inline thread_local std::tm tmTmp{};

//////////////////////////////////////////////////
/// protection mutex for std::gmtime results
///
inline std::shared_mutex  mutexLocalTime;

//---------------------------------------------------------------------------------
/// convert time to thread_local std::tm
///
template <typename T>
inline static std::tm *  threadfree_gmtime(const T* t)
{
    // for win use threadsafe gmtime_s
    // for unix protect it with mutex

    #ifdef _WIN32
        (void)gmtime_s(&tmTmp,t);
    #else
        std::unique_lock lk(mutexLocalTime);
        tmTmp = *std::gmtime(t);
    #endif

    return &tmTmp;
}
//---------------------------------------------------------------------------------
/// threadsafe convert to std::string(%Y/%m/%d %H:%M:%S)
///
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
/// threadsafe convert to std::string(%Y/%m/%d)
///
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
/// threadsafe convert to std::string(%H:%M:%S)
///
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
/// non blocking thread safe convertion GMT from tm* to time_t
///
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
/// GMT std::time_t to  QDate threadsafe convertion
///
inline QDate stdTimeToQDate(std::time_t& t)
{
    std::tm* tmSt=threadfree_gmtime(&t);
    return QDate(tmSt->tm_year+1900,tmSt->tm_mon+1,tmSt->tm_mday);
}
//---------------------------------------------------------------------------------
/// GMT std::time_t to  QTime threadsafe convertion
///
inline QTime stdTimeToQTime(std::time_t& t)
{
    std::tm* tmSt=threadfree_gmtime(&t);
    return QTime(tmSt->tm_hour,tmSt->tm_min,tmSt->tm_sec);
}
//---------------------------------------------------------------------------------
/// GMT QDate to std::time_t threadsafe convertion
///
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
/// GMT QTime to std::time_t threadsafe convertion
///
inline std::time_t QTimeToStdTime(QTime t)
{
    std::tm tmSt;
    {
        tmSt.tm_year   = 1971 - 1900;
        tmSt.tm_mon    = 1 - 1;
        tmSt.tm_mday   = 1;
        tmSt.tm_hour   = t.hour();
        tmSt.tm_min    = t.minute();
        tmSt.tm_sec    = t.second();
        tmSt.tm_isdst  = 0;
    }
    return (mktime_gm(&tmSt));
}
//---------------------------------------------------------------------------------

#endif // TREADSAFELOCALTIME_H
