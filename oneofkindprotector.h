#ifndef ONEOFKINDPROTECTOR_H
#define ONEOFKINDPROTECTOR_H

#include<QSystemSemaphore>
#include<QSharedMemory>
#include<QCoreApplication>
#include<QCryptographicHash>
#include<QDebug>

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
