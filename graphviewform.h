#ifndef GRAPHVIEWFORM_H
#define GRAPHVIEWFORM_H

#include <QWidget>
#include "ticker.h"
#include "graphholder.h"

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
    std::shared_ptr<GraphHolder> holder;

public:
    explicit GraphViewForm(const int TickerID, std::vector<Ticker> &v, std::shared_ptr<GraphHolder> hldr, QWidget *parent = nullptr);
    ~GraphViewForm();

    inline int TickerID() const {return iTickerID;};

signals:
    void NeedLoadGraph(const  int iTickerID, const std::time_t tBegin, const std::time_t tEnd);

public slots:

    void slotInvalidateGraph(std::time_t dtDegin, std::time_t dtEnd);

protected slots:
    void slotLoadGraphButton();

private:
    Ui::GraphViewForm *ui;
};

#endif // GRAPHVIEWFORM_H
