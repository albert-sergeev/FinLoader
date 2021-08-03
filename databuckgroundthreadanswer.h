#ifndef DATABUCKGROUNDTHREADANSWER_H
#define DATABUCKGROUNDTHREADANSWER_H

#include<QWidget>

class dataBuckgroundThreadAnswer
{

public:
    enum eAnswerType:int {nop,famLoadBegin,famLoadEnd,famLoadCurrent,LoadActivity};

    //-------------------------------------------------------------------------------
    inline eAnswerType  AnswerType() const          {return iAnswerType;};
    //
    inline void SetPercent(const int i)             {iLoadPercent = i;};
    inline int  Percent()   const                   {return iLoadPercent;};
    //
    inline void SetParentWnd(QWidget * const wt)    { parentWnd = wt;};
    inline  QWidget *  GetParentWnd() const         {return parentWnd;}
    //-------------------------------------------------------------------------------
    dataBuckgroundThreadAnswer(eAnswerType iType = eAnswerType::nop, QWidget * parent = nullptr):parentWnd{parent}{iAnswerType = iType;};
    //-------------------------------------------------------------------------------
    dataBuckgroundThreadAnswer(const dataBuckgroundThreadAnswer &o):
        iLoadPercent{o.iLoadPercent}, iAnswerType{o.iAnswerType},parentWnd{o.parentWnd}{;};


private:
    int iLoadPercent{0};
    eAnswerType iAnswerType;

    QWidget * parentWnd;
    //void * parentWnd;
};

#endif // DATABUCKGROUNDTHREADANSWER_H
