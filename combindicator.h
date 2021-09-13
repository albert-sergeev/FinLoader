#ifndef COMBINDICATOR_H
#define COMBINDICATOR_H

//#include <sstream>
#include <QWidget>

class CombIndicator : public QWidget
{
    Q_OBJECT
private:
    int iCurrentLevel;
    const int iMaxLevel;
public:
    explicit CombIndicator(int MaxLevel, QWidget *parent = nullptr);

    void setCurrentLevel(int Level)     {
        if (Level >= 0 && Level != iCurrentLevel ){
            iCurrentLevel = Level < iMaxLevel ? Level : iMaxLevel;
            this->setToolTip(QString(tr("Active processes: "))
                         +QString::number(iCurrentLevel)
                         +QString(tr(" from: "))
                         +QString::number(iMaxLevel)
                         );
            this->repaint();
        }
    };

signals:

protected:
    void paintEvent(QPaintEvent *event);

};

#endif // COMBINDICATOR_H
