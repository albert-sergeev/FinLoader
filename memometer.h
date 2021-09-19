#ifndef MEMOMETER_H
#define MEMOMETER_H

#include <QWidget>

class Memometer : public QWidget
{
    Q_OBJECT
private:
    QFont font;
    double dPercent;
    std::string sToolTip;
public:
    explicit Memometer(QWidget *parent = nullptr);

    inline void setToolTipText(const std::string &s){
        sToolTip = s;
        this->setToolTip(QString::fromStdString(sToolTip));
    };

    inline void setValue(double d){
        double dNewValue = d < 0 ? 0 : (d > 1 ? 1 : d);
        if (dPercent != dNewValue){
            dPercent = dNewValue;
            repaint();
        }
    }

signals:

protected:
    void paintEvent(QPaintEvent *event);
};

#endif // MEMOMETER_H
