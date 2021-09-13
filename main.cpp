#include "mainwindow.h"

#include <QApplication>
#include "oneofkindprotector.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //------------------------------------------------------
    QCoreApplication::setOrganizationName("Albert's Programming");
    QCoreApplication::setApplicationName("FinLoader");
    //------------------------------------------------------
    OneOfKindProtector one;

    if (one.TheOne()){
        MainWindow w;
        w.show();
        return a.exec();
    }
    else{
        (void)QMessageBox::critical(0,"Warning",
                                   "Only one instance of the application can be runing at a time!",
                                   QMessageBox::Ok
                                   );
    }
}
