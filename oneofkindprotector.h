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

#ifndef ONEOFKINDPROTECTOR_H
#define ONEOFKINDPROTECTOR_H

#include<QSystemSemaphore>
#include<QSharedMemory>
#include<QCoreApplication>
#include<QCryptographicHash>
#include<QDebug>

/////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Class used to protect from running multiple instances of the program
///
class OneOfKindProtector
{
    bool bTheOne;
    QSharedMemory memShared;
public:
    OneOfKindProtector():bTheOne{false}{
        QString sQtrSemaphore = QCoreApplication::organizationName() + QCoreApplication::applicationName()+"Semaphore";
        QString sQtrMemory = QCoreApplication::organizationName() + QCoreApplication::applicationName()+"Memory";
        QString hashStrSemaphore = QString("%1").arg(QString(QCryptographicHash::hash(sQtrSemaphore.toUtf8(),QCryptographicHash::Md5).toHex()));
        QString hashStrMemory = QString("%1").arg(QString(QCryptographicHash::hash(sQtrMemory.toUtf8(),QCryptographicHash::Md5).toHex()));

        QSystemSemaphore sm (hashStrSemaphore,1);
        sm.acquire();
        {
            QSharedMemory memSharedTmp(hashStrMemory);
            memSharedTmp.attach();
        }
        memShared.setKey(hashStrMemory);
        if (memShared.attach()){
            bTheOne = false;
        }
        else{
            bTheOne = true;
            memShared.create(1);
        }
        sm.release();
    }

    bool TheOne()   const {return bTheOne;};

};

#endif // ONEOFKINDPROTECTOR_H
