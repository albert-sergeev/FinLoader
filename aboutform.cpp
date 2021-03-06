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

#include "aboutform.h"
#include "ui_aboutform.h"

#include "threadfreecout.h"

#include<QTextEdit>

//----------------------------------------------------------------------------------------------------
AboutForm::AboutForm(QWidget *parent,Qt::WindowFlags f ) :
    QWidget(parent,f),
    ui(new Ui::AboutForm)
{
    ui->setupUi(this);

    connect(ui->btnClose,SIGNAL(clicked()),this,SLOT(slotQuit()));
    connect(ui->btnLicense,SIGNAL(clicked()),this,SLOT(slotLicense()));
}
//----------------------------------------------------------------------------------------------------
AboutForm::~AboutForm()
{
    delete ui;
}
//----------------------------------------------------------------------------------------------------
void AboutForm::slotQuit()
{
    this->close();
}
//----------------------------------------------------------------------------------------------------
void AboutForm::slotLicense()
{

    QWidget *wt = new QWidget(this,Qt::Window);
    QVBoxLayout *lt = new QVBoxLayout();
    QTextEdit *edText = new QTextEdit();
    edText->setReadOnly(true);
    lt->addWidget(edText);
    QHBoxLayout * ltH = new QHBoxLayout();
    lt->addLayout(ltH);
    QLabel *lbl = new QLabel("");
    ltH->addWidget(lbl);
    QPushButton * btnExit = new QPushButton(tr("Close license"));
    ltH->addWidget(btnExit);
    connect(btnExit,SIGNAL(clicked()),wt,SLOT(close()));
    wt->setAttribute(Qt::WA_DeleteOnClose);
    wt->setWindowTitle(tr("License"));
    wt->setLayout(lt);

//    edText->append(tr("FinLoader ???????????????? ?????????????????? ?????????????????????? ????????????????????????;\r\n???? ???????????? ???????????????????????????? ??/?????? ???????????????? ?????? ???????????????? ???????????????? ?????????????????????? ???????????????????????? ???????????????? GNU (GNU GPL), ???????????????????????????? ???????????? ???????????????????? ???????????????????????? ?????????????????????? (FSF), ???????? ???????????????? ???????????? 3, ???????? (???? ???????? ????????????????????) ?????????? ?????????? ?????????????? ????????????.\r\n\r\n"
//                      "?????????????????? ???????????????????????????????? ?? ??????????????, ?????? ?????? ?????????? ????????????????, ???? ?????? ?????????? ???? ???? ???? ???????? ?????????????????????? ????????????????????????; ???????? ?????? ?????????????????? ?????????????????????? ????????????????????????, ?????????????????? ?? ???????????????????????????????? ???????????????????? ?? ???????????????????????? ?????? ???????????????????????? ??????????. ?????????????????????? ?????????????? ?? ?????????????????????? ???????????????????????? ???????????????? GNU..\r\n\r\n"
//                      "???? ???????????? ???????? ???????????????? ?????????? ?????????????????????? ???????????????????????? ???????????????? GNU ???????????? ?? ???????? ????????????????????. ???????? ?????? ???? ??????, ???????????????? ???????????????? http://www.gnu.org/licenses/ "));



    edText->append(tr("FinLoader is free software;"));
    edText->append(tr("you may redistribute and/or modify it under the terms of the GNU General Public License (GNU GPL) as published by the Free Software Foundation (FSF), or License version 3, or (at your option) any later version."));
    edText->append("");
    edText->append(tr("This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.."));
    edText->append("");
    edText->append(tr("You should have received a copy of the GNU General Public License along with this program. If not, read http://www.gnu.org/licenses/"));


    QSize sz = wt->sizeHint();
    QPoint pt= this->pos();
    wt->setGeometry(pt.x(),pt.y(),sz.width(),sz.height());

    wt->show();


}
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
