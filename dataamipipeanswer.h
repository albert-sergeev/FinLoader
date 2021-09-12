#ifndef DATAAMIPIPEANSWER_H
#define DATAAMIPIPEANSWER_H

#include <string>
#include <chrono>


class dataAmiPipeAnswer
{
public:
    enum eAnswerType:int {Nop, ProcessNewComplite, PipeConnected, PipeDisconnected, PipeHalted, PipeOff, TextMessage, ErrMessage, testTimeEvent};
public:

private:
    int iTickerID{0};
    eAnswerType Type {eAnswerType::Nop};
    std::string strErr;
    std::string strTextInfo;
    std::time_t tTime;
public:
    dataAmiPipeAnswer(){};
    dataAmiPipeAnswer(const dataAmiPipeAnswer &o) = default;

    inline eAnswerType  AnswerType() const          {return Type;};
    inline void  SetType(const eAnswerType t)       {Type = t;};
    //
    inline std::string GetErrString() const         {return strErr;}
    inline void SetErrString(const std::string s)   {strErr = s;};
    //
    inline std::string GetTextInfo() const          {return strTextInfo;}
    inline void SetTextInfo(const std::string s)   {strTextInfo = s;};
    //
    inline void SetTickerID(const int i)            {iTickerID = i;};
    inline int  TickerID()   const                  {return iTickerID;};

    inline void SetTime(const std::time_t t)        {tTime = t;};
    inline std::time_t   Time()   const             {return tTime;};

    dataAmiPipeAnswer& operator=(dataAmiPipeAnswer &o) = delete;
};

#endif // DATAAMIPIPEANSWER_H
