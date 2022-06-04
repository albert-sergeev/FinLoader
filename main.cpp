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

#include "mainwindow.h"

#include<thread>
#include <QApplication>
#include "oneofkindprotector.h"
#include "splashwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //------------------------------------------------------
    QCoreApplication::setOrganizationName("Albert's Programming");
    QCoreApplication::setApplicationName("FinLoader");
    //------------------------------------------------------
    SplashWindow splash;
    splash.show();
    //------------------------------------------------------
    int iCount{0};
    while(iCount < 5){ // wait for 5 second if it was reboot
        OneOfKindProtector one;
        if (one.TheOne()){
            /////////////////////////
            MainWindow w;
            //splash.finish(&w);
            w.show();
            return a.exec();
            /////////////////////////
            break;
        }
        std::this_thread::sleep_for(milliseconds(1000));//1000ms
        ++iCount;
    }
    splash.close();

    (void)QMessageBox::critical(0,"Warning",
                               "Only one instance of the application can be runing at a time!",
                               QMessageBox::Ok
                               );
    std::cout<<"Only one instance of the application can be runing at a time!\n";
}
