#ifndef PLUSBUTTON_H
#define PLUSBUTTON_H

#include <QWidget>
#include <QLabel>

class PlusButton : public QWidget
{
    Q_OBJECT

private:
    const bool bPlus;
    bool bPushed{false};
public:
    explicit PlusButton(bool Plus, QWidget *parent = nullptr);

    //virtual void paint(QPainter* painter,  QWidget*);

public:
signals:

    void clicked();
    void clicked(bool);

    // QWidget interface
protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
};

#endif // PLUSBUTTON_H
