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

#ifndef STYLEDSWITCHER_H
#define STYLEDSWITCHER_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QStateMachine>
#include <QPropertyAnimation>

/////////////////////////////////////////////////////////
/// \brief Styled checkbox-like widget.
/// Has, usual for checkbox, events and slots
///
class StyledSwitcher : public QWidget
{
    Q_OBJECT

private:

    QString sLeft;
    QString sRight;

    bool bChecked;
    int ibtnWidth;

    QLabel *lblL;
    QLabel *lblR;
    QPushButton *btnP;

protected slots:

    void slotStateOnActivated(bool);
    void slotStateOffActivated(bool);
    void slotButtonClicked();

private: signals:

    void DoChangeStateOn();
    void DoChangeStateOff();

public:
    explicit StyledSwitcher(QString Left,QString Right,bool InitOn, int btnWidth,QWidget *parent = nullptr);
    ~StyledSwitcher();

    inline bool isChecked()    const                {return bChecked;};
    void setCheckState(Qt::CheckState state);
    void setChecked(bool bState);

    void SetOnColor(const QPalette::ColorRole role, const QColor q) ;
    void SetOffColor(const QPalette::ColorRole role, const QColor q) ;

signals:

    void stateChanged(int);
    // QObject interface
public:
    bool eventFilter(QObject *watched, QEvent *event);
};

#endif // STYLEDSWITCHER_H
