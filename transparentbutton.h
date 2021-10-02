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

#ifndef TRANSPARENTBUTTON_H
#define TRANSPARENTBUTTON_H

#include <QWidget>

//////////////////////////////////////////////////////////////////////////////////
/// \brief The TransparentButton button widget
/// can work in two modes: as button and as checkbox (defined in constructor)
/// in checkbox mode can switch displayed text depend on state
/// works as usual button or checkbox control with usual interace
/// !Warning! it doesn't know its own size until realy painted, so need to correct positon on resize event if it is not in layout
///
class TransparentButton : public QWidget
{
    Q_OBJECT

public:
    enum eMode:int {Button, CheckBox};

protected:
    // behavor
    QString strText;
    QString strTextAlternate;
    bool bState;
    eMode modeMode;
    bool bPushed;

    QFont font;
    int iWidth;
    int iHeight;

public:
    //---------------------------------------------------------------------------------------
    // constructors and initialize procedures
    explicit TransparentButton(QString Text, QWidget *parent = nullptr);
    explicit TransparentButton(QString Text,QString TextAlternate, bool State, QWidget *parent = nullptr);

    void setText(QString Text)              {strText = Text;};
    void setTextAlternate(QString Text)     {strTextAlternate = Text;};
    void setState(bool State)               {bState = State;};
    void setFont(const QFont &f)            {font = f;}
    // constructors and initialize procedures end
    //---------------------------------------------------------------------------------------

    eMode Mode()        const   {return modeMode;};

    // checkbox-like interface
    bool IsChecked()    const   {return modeMode == eMode::CheckBox ? bState: true;};
    void setChecked(bool b)     {   if (modeMode == eMode::CheckBox)  bState =b; repaint();};

public:
signals:

    void clicked();         // pushbutton-like event
    void stateChanged(int); // checkbox-like event

    // QWidget interface
protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

};

#endif // TRANSPARENTBUTTON_H
