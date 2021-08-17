#ifndef GRAPHVIEWFORM_H
#define GRAPHVIEWFORM_H

#include <QWidget>
#include "ticker.h"
#include "graphholder.h"
#include "blockfreequeue.h"
#include "bargraphicsitem.h"
#include "styledswitcher.h"

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

    StyledSwitcher *swtCandle;

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
    double dHScale;
    std::map<int,double> mVScale;
    bool bOHLC;

    static const int iLeftShift{20};
    static const int iRightShift{50};

    static const int iViewPortHeight{1000};
    static const int iViewPortHighStrip{100};
    static const int iViewPortLowStrip{100};


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

    void slotSceneRectChanged( const QRectF &);
    void slotVerticalScrollBarValueChanged(int);
    //void slotHorisontalScrollBarValueChanged(int);


    void slotHScaleValueChanged(double);
    void slotVScaleValueChanged(double);
    void slotVVolumeScaleValueChanged(double);


private:
    Ui::GraphViewForm *ui;

protected:
    template<typename T>
    void SliderValueChanged(int i);

    template<typename T>
    bool RepainInvalidRange(RepainTask &);

    template<typename T>
    bool RollSliderToMidTime(std::time_t tMidPos);


    inline double realYtoViewPortY(double y) {return  ((y - dStoredLowMin) * (mVScale.at(iSelectedInterval))) + iViewPortLowStrip;};

    // QWidget interface
protected:
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);
};

#endif // GRAPHVIEWFORM_H
