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

class Bulbululator : public QWidget
{
    Q_OBJECT

    QLabel *lbl;
    int iTickerID;
    bool bBlink;
    QPalette defPallete;
    std::chrono::time_point<std::chrono::steady_clock> dtStartBlink;

    //--------------------
    //QLabel *buble;
    QWidget *buble;

    QParallelAnimationGroup pag;

public:
    explicit Bulbululator(QWidget *parent = nullptr);
    ~Bulbululator();
    //---------------------------

    void SetText(QString sTxt){lbl->setText(sTxt);}

    inline void SetTickerID(const int TickerID)     {iTickerID = TickerID;}
    inline int  TickerID()      const               {return iTickerID;}

    void Bubble();

signals:

private:
    void timerEvent(QTimerEvent * event) override;

};

#endif // BULBULULATOR_H
