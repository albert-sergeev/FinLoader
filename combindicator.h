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

#ifndef COMBINDICATOR_H
#define COMBINDICATOR_H

//#include <sstream>
#include <QWidget>

class CombIndicator : public QWidget
{
    Q_OBJECT
private:
    int iCurrentLevel;
    const int iMaxLevel;
public:
    explicit CombIndicator(int MaxLevel, QWidget *parent = nullptr);

    void setCurrentLevel(int Level)     {
        if (Level >= 0 && Level != iCurrentLevel ){
            iCurrentLevel = Level < iMaxLevel ? Level : iMaxLevel;
            this->setToolTip(QString(tr("Active processes: "))
                         +QString::number(iCurrentLevel)
                         +QString(tr(" from: "))
                         +QString::number(iMaxLevel)
                         );
            this->repaint();
        }
    };

signals:

protected:
    void paintEvent(QPaintEvent *event);

};

#endif // COMBINDICATOR_H
