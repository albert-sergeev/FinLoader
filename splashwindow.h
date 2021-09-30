#ifndef SPLASHWINDOW_H
#define SPLASHWINDOW_H

#include <QSplashScreen>

class SplashWindow : public QSplashScreen
{
    Q_OBJECT
public:
    SplashWindow();
    //using QSplashScreen::QSplashScreen;



    // QObject interface
protected:
    //multiplatform Qt::WindowStaysOnTopHint
    void timerEvent(QTimerEvent */*event*/)
    {
        raise();
    }
};

#endif // SPLASHWINDOW_H
