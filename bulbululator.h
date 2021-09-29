#ifndef BULBULULATOR_H
#define BULBULULATOR_H

#include <QWidget>
#include <QLabel>
#include <QDockWidget>
#include <QParallelAnimationGroup>


#include <chrono>

using namespace std::chrono_literals;
using seconds=std::chrono::duration<double>;
using milliseconds=std::chrono::duration<double,
    std::ratio_multiply<seconds::period,std::milli>
    >;

class Buble : public QWidget
{
    Q_OBJECT
public:
    explicit Buble(QWidget *parent = nullptr);
    ~Buble();

private:
    void paintEvent(QPaintEvent *event) override;
};

class Bulbululator : public QWidget
{
    Q_OBJECT

public:
    enum eTickerState:int {Informant,Connected,NeededPipe,Halted};
protected:

    QLabel *lblMain;
    int iTickerID;
    bool bBlink;
    QPalette defPallete;
    std::chrono::time_point<std::chrono::steady_clock> dtStartBlink;

    int iProcessCount;

    eTickerState stState;
    QString strName;

    //--------------------
//    Buble bubleB;
//    bool bStartBuble{false};

//    QParallelAnimationGroup pag;

public:
    explicit Bulbululator(QWidget *parent = nullptr);
    ~Bulbululator();
    //---------------------------

    inline void SetText(QString sTxt)    {lblMain->setText(sTxt);}
    inline QString Text()      const     {  return lblMain->text();}

    inline void SetTickerName(QString s) {strName = s;}
    inline QString TickerName() const    {  return strName;}

    inline void SetTickerID(const int TickerID)     {iTickerID = TickerID;}
    inline int  TickerID()      const               {return iTickerID;}

    inline int AddInstance()                    {return  ++iProcessCount;};
    inline int RemoveInstance()                 {return  iProcessCount > 0 ? --iProcessCount : 0;};

    void setState(Bulbululator::eTickerState State);

    void Bubble();

    bool operator<(const Bulbululator &o){ return QString::localeAwareCompare(lblMain->text(),o.lblMain->text())<0;}


signals:

    void DoubleClicked(const int TickerID);

private:
    void timerEvent(QTimerEvent * event) override;
    bool event(QEvent *event) override;

};



#endif // BULBULULATOR_H
