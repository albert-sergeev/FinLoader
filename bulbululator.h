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

    QLabel *lblMain;
    int iTickerID;
    bool bBlink;
    QPalette defPallete;
    std::chrono::time_point<std::chrono::steady_clock> dtStartBlink;

    //--------------------
//    Buble bubleB;
//    bool bStartBuble{false};

//    QParallelAnimationGroup pag;

public:
    explicit Bulbululator(QWidget *parent = nullptr);
    ~Bulbululator();
    //---------------------------

    void SetText(QString sTxt){lblMain->setText(sTxt);}

    inline void SetTickerID(const int TickerID)     {iTickerID = TickerID;}
    inline int  TickerID()      const               {return iTickerID;}

    void Bubble();

signals:

    void DoubleClicked(const int TickerID);

private:
    void timerEvent(QTimerEvent * event) override;
    bool event(QEvent *event) override;

};



#endif // BULBULULATOR_H
