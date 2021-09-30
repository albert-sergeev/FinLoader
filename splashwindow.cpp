#include "splashwindow.h"
#include <QGridLayout>
#include <QLabel>
#include<QTextEdit>

SplashWindow::SplashWindow():QSplashScreen(QPixmap(":/store/images/splashT")/*,Qt::WindowStaysOnTopHint */)
{
    //showMessage("Loading...",Qt::AlignmentFlag::AlignBottom | Qt::AlignmentFlag::AlignRight);
    QGridLayout *lt=new QGridLayout();
    setLayout(lt);

    QTextEdit* ed = new QTextEdit();
    ed->setReadOnly(true);
    ed->setText(tr("Hellow world! \r\n another world too :)"
                   "Hellow world! \r\n another world too :)"
                   "Hellow world! \r\n another world too :)"
                   "Hellow world! \r\n another world too :)"
                   "Hellow world! \r\n another world too :)"
                   "Hellow world! \r\n another world too :)"
                   "Hellow world! \r\n another world too :)"
                   "Hellow world! \r\n another world too :)"
                   "Hellow world! \r\n another world too :)"
                   "Hellow world! \r\n another world too :)"
                   "Hellow world! \r\n another world too :)"
                   "Hellow world! \r\n another world too :)"
                   "Hellow world! \r\n another world too :)"
                   "Hellow world! \r\n another world too :)"
                   "Hellow world! \r\n another world too :)"

                   ));

    QFont font;
    font.setPixelSize(72);
    font.setBold(false);
    font.setFamily("Times New Roman");

    QLabel *lbl=new QLabel("FinLoader");
    lbl->setFont(font);

    lt->addWidget(lbl,0,0,0,3,Qt::AlignmentFlag::AlignHCenter | Qt::AlignmentFlag::AlignTop);

    lt->addWidget(ed,1,1,Qt::AlignmentFlag::AlignHCenter| Qt::AlignmentFlag::AlignTop);


    // for multiplatform Qt::WindowStaysOnTopHint
    this->startTimer(100);
}
