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

#ifndef MEMOMETER_H
#define MEMOMETER_H

#include <QWidget>

class Memometer : public QWidget
{
    Q_OBJECT
private:
    QFont font;
    double dPercent;
    std::string sToolTip;
public:
    explicit Memometer(QWidget *parent = nullptr);

    inline void setToolTipText(const std::string &s){
        sToolTip = s;
        this->setToolTip(QString::fromStdString(sToolTip));
    };

    inline void setValue(double d){
        double dNewValue = d < 0 ? 0 : (d > 1 ? 1 : d);
        if (dPercent != dNewValue){
            dPercent = dNewValue;
            repaint();
        }
    }

signals:

protected:
    void paintEvent(QPaintEvent *event);
};

#endif // MEMOMETER_H
