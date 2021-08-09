#ifndef GRAPHVIEWFORM_H
#define GRAPHVIEWFORM_H

#include <QWidget>
#include "ticker.h"

namespace Ui {
class GraphViewForm;
}

class GraphViewForm : public QWidget
{
    Q_OBJECT

private:
    const int iTickerID;
    Ticker tTicker;
    std::vector<Ticker> & vTickersLst;

public:
    explicit GraphViewForm(const int TickerID, std::vector<Ticker> &v, QWidget *parent = nullptr);
    ~GraphViewForm();

    inline int TickerID() const {return iTickerID;};

private:
    Ui::GraphViewForm *ui;
};

#endif // GRAPHVIEWFORM_H
