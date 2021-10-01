#ifndef SPLASHWINDOW_H
#define SPLASHWINDOW_H

#include <QSplashScreen>
#include <QTextEdit>
#include "transparentbutton.h"

class SplashWindow : public QSplashScreen
{
    Q_OBJECT

QTextEdit* edText;
TransparentButton * btnEn;
TransparentButton * btnRu;

QString strRu;
QString strEn;

public:
    SplashWindow();
    //using QSplashScreen::QSplashScreen;

public slots:
    void slotLanguageRuChanged(int);
    void slotLanguageEngChanged(int);


    // QObject interface
protected:
    //multiplatform Qt::WindowStaysOnTopHint
    void timerEvent(QTimerEvent */*event*/)
    {
        raise();
    }
};

#endif // SPLASHWINDOW_H
