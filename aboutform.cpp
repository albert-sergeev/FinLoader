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

//    edText->append(tr("FinLoader является свободным программным обеспечением;\r\nвы можете распространять и/или изменять его согласно условиям Стандартной Общественной Лицензии GNU (GNU GPL), опубликованной Фондом свободного программного обеспечения (FSF), либо Лицензии версии 3, либо (на ваше усмотрение) любой более поздней версии.\r\n\r\n"
//                      "Программа распространяется в надежде, что она будет полезной, но БЕЗ КАКИХ БЫ ТО НИ БЫЛО ГАРАНТИЙНЫХ ОБЯЗАТЕЛЬСТВ; даже без косвенных гарантийных обязательств, связанных с ПОТРЕБИТЕЛЬСКИМИ СВОЙСТВАМИ и ПРИГОДНОСТЬЮ ДЛЯ ОПРЕДЕЛЕННЫХ ЦЕЛЕЙ. Подробности читайте в Стандартной Общественной Лицензии GNU..\r\n\r\n"
//                      "Вы должны были получить копию Стандартной Общественной Лицензии GNU вместе с этой программой. Если это не так, прочтите страницу http://www.gnu.org/licenses/ "));



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
