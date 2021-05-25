#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("Albert's Programming");
    QCoreApplication::setApplicationName("FinLoader");

    MainWindow w;
    w.show();
    return a.exec();
}
