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

#ifndef BULBULULATOR_H
#define BULBULULATOR_H

#include <QWidget>
#include <QLabel>
#include <QDockWidget>
#include <QParallelAnimationGroup>


#include <chrono>

//////////////////////////////////////////////////////////////////////////////////////////////////
/// Bulbululator class - graphics item for using in user interface.
/// Represents activities with specified ticker to user.
///
/// Buble - not used (not implemenyed complitly)
///

//using namespace std::chrono_literals;
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
    void ContextMenuRequested(const int TickerID, const QPoint);

private:
    void timerEvent(QTimerEvent * event) override;
    bool event(QEvent *event) override;

};



#endif // BULBULULATOR_H
