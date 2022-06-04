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

#ifndef PLUSBUTTON_H
#define PLUSBUTTON_H

#include <QWidget>
#include <QLabel>

class PlusButton : public QWidget
{
    Q_OBJECT

private:
    const bool bPlus;
    bool bPushed{false};
public:
    explicit PlusButton(bool Plus, QWidget *parent = nullptr);

    //virtual void paint(QPainter* painter,  QWidget*);

public:
signals:

    void clicked();
    void clicked(bool);

    // QWidget interface
protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
};

#endif // PLUSBUTTON_H
