#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //------------------------------------------------------
    QCoreApplication::setOrganizationName("Albert's Programming");
    QCoreApplication::setApplicationName("FinLoader");
    //------------------------------------------------------
    MainWindow w;
    w.show();

//    Win32NamedPipe pip("\\\\.\\pipe\\AmiBroker2QUIK_TQBR.SBER_TICKS");
//    char testbuffer[1024];
//    int iBytesRead{0};
//    pip.Open();
//    std::this_thread::sleep_for(std::chrono::microseconds(10));
//    pip.Read(testbuffer,1024,iBytesRead);
    return a.exec();
}
