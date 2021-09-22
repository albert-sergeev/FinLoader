#ifndef DATAAMIPIPEANSWER_H
#define DATAAMIPIPEANSWER_H

#include <string>
#include <chrono>

#include "graphholder.h"


class dataAmiPipeAnswer
{
public:
    enum eAnswerType:int {Nop, ProcessNewComplite, PipeConnected, PipeDisconnected, PipeHalted, PipeOff,
                          TextMessage, ErrMessage, testTimeEvent,
                          FastShowEvent,
                          AskNameAnswer
                         };
public:

private:
    int iTickerID{0};
    eAnswerType Type {eAnswerType::Nop};
    std::string strErr;
    std::string strTextInfo;
    std::time_t tTime;

    std::string strBind;
    std::string strPipeName;

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
    //
    inline void SetTime(const std::time_t t)        {tTime = t;};
    inline std::time_t   Time()   const             {return tTime;};
    //
    inline std::string GetBind() const              {return strBind;}
    inline void SetBind(const std::string s)        {strBind = s;};
    //
    inline std::string GetPipeName() const          {return strPipeName;}
    inline void SetPipeName(const std::string s)    {strPipeName = s;};
    //

    std::time_t tBegin{0};
    std::time_t tEnd{0};
    std::shared_ptr<GraphHolder> ptrHolder;

    dataAmiPipeAnswer& operator=(dataAmiPipeAnswer &o) = delete;
};

#endif // DATAAMIPIPEANSWER_H
