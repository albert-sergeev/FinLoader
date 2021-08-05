#ifndef BULBULULATOR_H
#define BULBULULATOR_H

#include <QWidget>
#include <QLabel>
#include <QDockWidget>

class Bulbululator : public QWidget
{
    Q_OBJECT

    QLabel *lbl;
    int iTickerID;

public:
    explicit Bulbululator(QWidget *parent = nullptr);

    void SetText(QString sTxt){lbl->setText(sTxt);}

    inline void SetTickerID(const int TickerID)     {iTickerID = TickerID;}
    inline int  TickerID()      const               {return iTickerID;}


signals:

};

#endif // BULBULULATOR_H
