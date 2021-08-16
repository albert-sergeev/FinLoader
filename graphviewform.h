#ifndef GRAPHVIEWFORM_H
#define GRAPHVIEWFORM_H

#include <QWidget>
#include "ticker.h"
#include "graphholder.h"
#include "blockfreequeue.h"
#include "bargraphicsitem.h"

namespace Ui {
class GraphViewForm;
}

struct RepainTask{
    std::time_t dtStart;
    std::time_t dtEnd;
};

class GraphViewForm : public QWidget
{
    Q_OBJECT

private:
    const int iTickerID;
    Ticker tTicker;
    std::vector<Ticker> & vTickersLst;

    BlockFreeQueue<RepainTask> queueRepaint;
    std::shared_ptr<GraphHolder> holder;

    QGraphicsScene *grScene;
    std::vector<BarGraphicsItem *> vShowedGraphicsBars;
    std::time_t tMiddleDate{0};

private:

    std::time_t tStoredMiddlePointPosition;
    std::time_t tStoredMinDate;
    std::time_t tStoredMaxDate;
    size_t  iStoredMaxSize;
    double dStoredLowMin;
    double dStoredHighMax;

    Bar::eInterval iSelectedInterval;
    size_t iMaxGraphViewSize;
    double dVScale;
    double dHScale;
    double dTailFreeZone;
    bool bOHLC;

    static const int iLeftShift{20};
    static const int iRightShift{100};

public:
signals:

    void SendToLog(QString);

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
    void slotLoadGraphButton2();

    //void slotAddBarTicksToView(GraphHolder::Iterator<BarTick> ItBeg, GraphHolder::Iterator<BarTick> ItEnd);
    //void slotAddBarsToView(GraphHolder::Iterator<Bar> ItBeg, GraphHolder::Iterator<Bar> ItEnd);

    void slotSliderValueChanged(int i);

private:
    Ui::GraphViewForm *ui;

protected:
    template<typename T>
    void SliderValueChanged(int i);

    template<typename T>
    bool RepainInvalidRange(RepainTask &);

    // QWidget interface
protected:
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);
};

#endif // GRAPHVIEWFORM_H
