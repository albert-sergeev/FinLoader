#ifndef DATABUCKGROUNDTHREADANSWER_H
#define DATABUCKGROUNDTHREADANSWER_H

#include<QWidget>

class dataBuckgroundThreadAnswer
{

public:
    enum eAnswerType:int {nop,famLoadBegin,famLoadEnd,famLoadCurrent,LoadActivity,TextInfoMessage};

    //-------------------------------------------------------------------------------
    inline eAnswerType  AnswerType() const          {return iAnswerType;};
    //
    inline bool Successfull() const                 {return bSuccessfull;}
    inline void SetSuccessfull(const bool bS)       {bSuccessfull = bS;};
    //
    inline std::string GetErrString() const         {return strErr;}
    inline void SetErrString(const std::string s)   {strErr = s;};
    //
    inline std::string GetTextInfo() const          {return strTextInfo;}
    inline void SetTextInfo(const std::string s)   {strTextInfo = s;};

    //
    inline void SetPercent(const int i)             {iLoadPercent = i;};
    inline int  Percent()   const                   {return iLoadPercent;};
    //
    inline void SetParentWnd(QWidget * const wt)    { parentWnd = wt;};
    inline  QWidget *  GetParentWnd() const         {return parentWnd;}
    //-------------------------------------------------------------------------------
    dataBuckgroundThreadAnswer(eAnswerType iType = eAnswerType::nop, QWidget * parent = nullptr):parentWnd{parent}{
        iAnswerType = iType;
        bSuccessfull = false;
    };
    //-------------------------------------------------------------------------------
    dataBuckgroundThreadAnswer(const dataBuckgroundThreadAnswer &o){

        iLoadPercent    = o.iLoadPercent;
        iAnswerType     = o.iAnswerType;
        bSuccessfull    = o.bSuccessfull;
        strErr          = o.strErr;
        strTextInfo     = o.strTextInfo;

        parentWnd       = o.parentWnd;
    };


private:
    int iLoadPercent{0};
    eAnswerType iAnswerType;
    bool bSuccessfull;
    std::string strErr;
    std::string strTextInfo;

    QWidget * parentWnd;
};

#endif // DATABUCKGROUNDTHREADANSWER_H
