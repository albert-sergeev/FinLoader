#ifndef BULBULULATOR_H
#define BULBULULATOR_H

#include <QWidget>
#include <QLabel>
#include <QDockWidget>
#include<QParallelAnimationGroup>

class Bulbululator : public QWidget
{
    Q_OBJECT

    QLabel *lbl;
    int iTickerID;

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

};

#endif // BULBULULATOR_H
