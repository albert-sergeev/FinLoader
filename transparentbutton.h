#ifndef TRANSPARENTBUTTON_H
#define TRANSPARENTBUTTON_H

#include <QWidget>

class TransparentButton : public QWidget
{
    Q_OBJECT

public:
    enum eMode:int {Button, CheckBox};

protected:
    QString strText;
    QString strTextAlternate;
    bool bState;
    eMode modeMode;
    bool bPushed;

    QFont font;
    int iWidth;
    int iHeight;

public:
    explicit TransparentButton(QString Text, QWidget *parent = nullptr);
    explicit TransparentButton(QString Text,QString TextAlternate, bool State, QWidget *parent = nullptr);

    void setText(QString Text)              {strText = Text;};
    void setTextAlternate(QString Text)     {strTextAlternate = Text;};
    void setState(bool State)               {bState = State;};
    void setFont(const QFont &f)            {font = f;}

    eMode Mode()        const   {return modeMode;};

    bool IsChecked()    const   {return modeMode == eMode::CheckBox ? bState: true;};
    void setChecked(bool b)     {   if (modeMode == eMode::CheckBox)  bState =b; repaint();};

public:
signals:
    void clicked();
    void stateChanged(int);

    // QWidget interface
protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void showEvent(QShowEvent *event);

};

#endif // TRANSPARENTBUTTON_H
