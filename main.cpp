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

    (void)QMessageBox::critical(0,"Warning",
                               "Only one instance of the application can be runing at a time!",
                               QMessageBox::Ok
                               );
    std::cout<<"Only one instance of the application can be runing at a time!\n";
}
