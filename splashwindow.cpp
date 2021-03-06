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

#include "splashwindow.h"
#include<QTextEdit>
#include<QLocale>
#include<QSettings>

#include "threadfreecout.h"

//------------------------------------------------------------------------------------------------------------------------
SplashWindow::SplashWindow():QSplashScreen(QPixmap(":/store/images/SplashScreen2")/*,Qt::WindowStaysOnTopHint */)
{
    //showMessage("Loading...",Qt::AlignmentFlag::AlignBottom | Qt::AlignmentFlag::AlignRight);

    strRu = "Добро пожаловать в программу FinLoader.\r\n\r\n"
            "Программа предназначена для загрузки и отображения данных котировок рынков ценных бумаг из файлов и/или источников данных в формате AmiBrocker (например из Quik).\r\n"
            "Программа создана, как демонстратор способности автора работать с многопоточными задачами, STL, создавать хранилища данных на базе  LSM  - деревьев, использовать среду  Qt и пр.\r\n"
            "Если вам понравилось и вы хотите чтобы автор сделал для вас больше - пишите на e-mail.\r\n\r\n"
            "Программа была разработана под GNU GPL (GNU General Public License) версии 3.";
    strEn = "Welcome to FinLoader.\r\n\r\n"
            "The program is designed to download and display data of stock market quotes from files and/or data sources in the AmiBrocker format (for example, from Quik).\r\n"
            "The program was created as a demonstration of the author's ability to work with multithreaded tasks, STL, create data warehouses based on an LSM tree, use the Qt environment etс.\r\n"
            "If you liked what you saw and want the author to do more for you - write to e-mail.\r\n\r\n"
            "Programm was developed under GNU GPL (GNU General Public License) version 3.";



    edText = new QTextEdit(this);
    edText->setReadOnly(true);
    edText->setText(strEn);
    edText->setGeometry(10,120,400,270);

    btnEn = new TransparentButton("En","En",true,this);
    btnRu = new TransparentButton ("Рус","Рус",false,this);


//    btnEn->setGeometry(420                     , 370,btnEn->width(),btnEn->height());
//    btnRu->setGeometry(420 + btnEn->width() + 2, 370,btnRu->width(),btnRu->height());
    btnEn->setGeometry(10                     , 100,btnEn->width(),btnEn->height());
    btnRu->setGeometry(10 + btnEn->width() + 2, 100,btnRu->width(),btnRu->height());


    connect(btnEn,SIGNAL(stateChanged(int)),this,SLOT(slotLanguageEngChanged(int)));
    connect(btnRu,SIGNAL(stateChanged(int)),this,SLOT(slotLanguageRuChanged(int)));


    QString sLc = QLocale::system().name();
    QString  sStarterLanguage = "English";
    if (sLc == "ru_RU"){
        sStarterLanguage = "Русский";
    }
    QString  sLanguage = "English";
    {
        QSettings m_settings;
        m_settings.beginGroup("Settings");
            sLanguage   = m_settings.value("Language",sStarterLanguage).toString();
        m_settings.endGroup();
    }

    if (sLanguage == "Русский")
    {
        btnRu->setChecked(true);
        btnEn->setChecked(false);
        edText->setText(strRu);
    }


    // for multiplatform Qt::WindowStaysOnTopHint
    this->startTimer(100);
}
//------------------------------------------------------------------------------------------------------------------------
void SplashWindow::slotLanguageRuChanged(int iState)
{
    if (iState !=0){
        btnEn->setChecked(false);
        edText->setText(strRu);
    }
    else{
        btnEn->setChecked(true);
        edText->setText(strEn);
    }
}
//------------------------------------------------------------------------------------------------------------------------
void SplashWindow::slotLanguageEngChanged(int iState)
{
    if (iState !=0){
        btnRu->setChecked(false);
        edText->setText(strEn);
    }
    else{
        btnRu->setChecked(true);
        edText->setText(strRu);
    }
}
//------------------------------------------------------------------------------------------------------------------------
