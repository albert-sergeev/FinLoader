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

#ifndef SPLASHWINDOW_H
#define SPLASHWINDOW_H

#include <QSplashScreen>
#include <QTextEdit>
#include "transparentbutton.h"

///////////////////////////////////////////////////////////////////////////
/// \brief Startup splash window
///
class SplashWindow : public QSplashScreen
{
    Q_OBJECT

QTextEdit* edText;
TransparentButton * btnEn;
TransparentButton * btnRu;

QString strRu;
QString strEn;

public:
    SplashWindow();
    //using QSplashScreen::QSplashScreen;

public slots:
    void slotLanguageRuChanged(int);
    void slotLanguageEngChanged(int);


    // QObject interface
protected:
    //multiplatform Qt::WindowStaysOnTopHint
    void timerEvent(QTimerEvent */*event*/)
    {
        raise();
    }
};

#endif // SPLASHWINDOW_H
