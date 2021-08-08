#ifndef STYLEDSWITCHER_H
#define STYLEDSWITCHER_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QStateMachine>
#include <QPropertyAnimation>

class StyledSwitcher : public QWidget
{
    Q_OBJECT

private:

    QString sLeft;
    QString sRight;

    bool bChecked;
    int ibtnWidth;

    QLabel *lblL;
    QLabel *lblR;
    QPushButton *btnP;

protected slots:

    void slotStateOnActivated(bool);
    void slotStateOffActivated(bool);


public:
    explicit StyledSwitcher(QString Left,QString Right,bool InitOn, int btnWidth,QWidget *parent = nullptr);
    ~StyledSwitcher();

    inline bool isChecked()    const                {return bChecked;};
    void SetChecked(bool bChecked);

    void SetOnColor(const QPalette::ColorRole role, const QColor q) ;
    void SetOffColor(const QPalette::ColorRole role, const QColor q) ;

signals:

    void stateChanged(int);
    // QObject interface
public:
    bool eventFilter(QObject *watched, QEvent *event);
};

#endif // STYLEDSWITCHER_H
