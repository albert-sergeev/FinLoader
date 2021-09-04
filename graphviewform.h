#ifndef GRAPHVIEWFORM_H
#define GRAPHVIEWFORM_H

#include <QWidget>
#include "ticker.h"
#include "graphholder.h"
#include "blockfreequeue.h"
#include "bargraphicsitem.h"
#include "styledswitcher.h"
#include "plusbutton.h"

namespace Ui {
class GraphViewForm;
}

struct RepainTask{
    std::time_t dtStart {0};
    std::time_t dtEnd   {0};
    bool bNeedToRescale {false};
};

class GraphViewForm : public QWidget
{
    Q_OBJECT

private:

    StyledSwitcher *swtCandle;

    PlusButton *  btnScaleHViewPlus;
    PlusButton *  btnScaleHViewMinus;

    PlusButton *  btnScaleVViewPlus;
    PlusButton *  btnScaleVViewMinus;

    PlusButton *  btnScaleVVolumePlus;
    PlusButton *  btnScaleVVolumeMinus;



    const int iTickerID;
    Ticker tTicker;
    std::vector<Ticker> & vTickersLst;

    BlockFreeQueue<RepainTask> queueRepaint;
    std::shared_ptr<GraphHolder> holder;

    QGraphicsScene *grScene;
    QGraphicsScene *grSceneScaleUpper;
    QGraphicsScene *grSceneVolume;
    //QGraphicsScene *grSceneScaleLower;
    QGraphicsScene *grSceneHorizScroll;
    QGraphicsScene *grSceneViewR1;
    QGraphicsScene *grSceneViewL1;
    QGraphicsScene *grSceneVertScroll;


    std::map<int,std::vector<BarGraphicsItem *>>    mShowedGraphicsBars;
    std::map<int,std::vector<QGraphicsItem *>>      mShowedVolumes;

    std::map<int,std::vector<QGraphicsItem *>>      mVFramesViewQuotes;
    std::map<int,std::vector<QGraphicsItem *>>      mVFramesScaleUpper;
    std::map<int,std::vector<QGraphicsItem *>>      mVFramesVolume;
    std::map<int,std::vector<QGraphicsItem *>>      mVFramesHorisSmallScale;

    std::vector<std::pair<QGraphicsItem *,double>>  vHorizFramesViewQuotes;
    std::vector<QGraphicsItem *>                    vHorizFramesScaleUpper;

    std::map<int,std::vector<QGraphicsItem *>>      mLeftFrames;
    std::map<int,std::vector<QGraphicsItem *>>      mRightFrames;

private:

    std::time_t tStoredRightPointPosition;
    int iStoredRightAggregate;

    std::time_t tStoredMinDate;
    std::time_t tStoredMaxDate;
    /*size_t*/ int  iStoredMaxSize;
    double dStoredLowMin;
    double dStoredHighMax;

    double dStoredVolumeLowMin;
    double dStoredVolumeHighMax;

    Bar::eInterval iSelectedInterval;
    //size_t iMaxGraphViewSize;
    double dHScale;
    std::map<int,double> mVScale;
    std::map<int,double> mVVolumeScale;

    std::vector<std::pair<double,bool>> vHLines;
    bool bOHLC;

    static const int iLeftShift{20};
    static const int iRightShift{50};

    static const int iViewPortHeight{600};
    static const int iViewPortHighStrip{20};
    static const int iViewPortLowStrip{20};

    static const int iVolumeViewPortHighStrip{3};

public:
signals:

    void SendToLog(QString);

public:
    explicit GraphViewForm(const int TickerID, std::vector<Ticker> &v, std::shared_ptr<GraphHolder> hldr, QWidget *parent = nullptr);
    ~GraphViewForm();

    inline int TickerID() const {return iTickerID;};

    double GetHScale() {return mVScale[iSelectedInterval];}

signals:
    void NeedLoadGraph(const  int iTickerID, const std::time_t tBegin, const std::time_t tEnd);

public slots:

    void slotInvalidateGraph(std::time_t dtDegin, std::time_t dtEnd, bool bNeedToRescale = false);
    void setFramesVisibility(std::tuple<bool,bool,bool,bool,bool>);

protected slots:
  //  void slotLoadGraphButton(); // for tests

    void slotSceneRectChanged( const QRectF &);
    void slotVerticalScrollBarValueChanged(int);
    void slotHorizontalScrollBarValueChanged(int iH);

    void slotHScaleQuotesClicked(bool);
    void slotVScaleQuotesClicked(bool);
    void slotVScaleVolumeClicked(bool);

    void dateTimeBeginChanged(const QDateTime&);
    void dateTimeEndChanged(const QDateTime&);

    void slotPeriodButtonChanged();


private:
    Ui::GraphViewForm *ui;

protected:

    void PaintViewPort               (bool bFrames,bool bBars,bool bVolumes, bool bStoreRightPos);
    void PaintViewPort               (int iStart, int iEnd,bool bFrames ,bool bBars,bool bVolumes, bool bStoreRightPos);
    void PaintHorizontalScales       ();
    void PaintHorizontalFrames       (int iStart, int iEnd);
    void PaintVerticalSideScales     ();
    void PaintVerticalFrames         (int iStart, int iEnd);

    template<typename T>
    void PaintBars       (int iStart, int iEnd, bool bPaintBars, bool bPaintVolumes, bool bStoreRightPos);

    template<typename T>
    void PainVerticalFramesT         (int iStart, int iEnd);


    template<typename T>
    bool RepainInvalidRange(RepainTask &);


    void SetSliderToPos (std::time_t tRightPos, int iRightAggregate);
    template<typename T>
    void SetSliderToPosT(std::time_t tRightPos, int iRightAggregate);


    void DrawLineToScene(const int idx,const  qreal x1,const  qreal y1,const qreal x2,const  qreal y2,
                         std::map<int,std::vector<QGraphicsItem *>>& mM, QGraphicsScene *scene, const QPen & pen,
                         const  std::time_t t = 0, bool bHasTooltip = false);
    void DrawLineToScene(const int idx,const  qreal x1,const  qreal y1,const qreal x2,const  qreal y2,
                         std::map<int,std::vector<QGraphicsItem *>>& mM, QGraphicsScene *scene, const QPen & pen,
                         const  std::string sToolTip, bool bHasTooltip = false);
    void DrawIntermittentLineToScene(const int idx,const  qreal x1,const  qreal y1,const qreal x2,const  qreal y2,
                         std::map<int,std::vector<QGraphicsItem *>>& mM, QGraphicsScene *scene, const QPen & pen,
                         const  std::time_t t, bool bHasTooltip);
    void DrawIntermittentLineToScene(const int idx,const  qreal x1,const  qreal y1,const qreal x2,const  qreal y2,
                         std::map<int,std::vector<QGraphicsItem *>>& mM, QGraphicsScene *scene, const QPen & pen,
                         const  std::string sToolTip, bool bHasTooltip = false);



    void DrawTimeToScene(const int idx,const  qreal x,const  qreal y,const  std::tm &,
                                        std::map<int,std::vector<QGraphicsItem *>>& mM, QGraphicsScene *scene, const QFont & font);
    void DrawIntToScene(const int idx,const  qreal x,const  qreal y,const  int n, Qt::AlignmentFlag alignH, Qt::AlignmentFlag alignV,
                                        std::map<int,std::vector<QGraphicsItem *>>& mM, QGraphicsScene *scene, const QFont & font);
    void DrawDoubleToScene(const int idx,const  qreal x ,const  qreal y,const double n, Qt::AlignmentFlag alignH, Qt::AlignmentFlag alignV,
                           std::map<int,std::vector<QGraphicsItem *>>& mM, QGraphicsScene *scene, const QFont & font);


    void InvalidateScenes();

    std::tuple<int,int,int,int> getHPartStep(double realH, double viewportH);


    void SetMinMaxDateToControls();
    void RepositionPlusMinusButtons();
    void SetSelectedIntervalToControls();

    template<typename T>
    void EraseLinesUpper(T& mM, int iStart, QGraphicsScene *);
    template<typename T>
    void EraseLinesLower(T& mM, int iEnd, QGraphicsScene *);
    template<typename T>
    void EraseLinesMid(T& mM, int iStart,int iEnd, QGraphicsScene *);


    void Erase();
    void EraseFrames();
    void EraseBars();
    void EraseVolumes();


    inline double realYtoSceneY      (double y) {return  ((y - dStoredLowMin)       * (mVScale.at      (iSelectedInterval))) + iViewPortLowStrip;};
    inline double realYtoSceneYVolume(double y) {return  ((y - 0) * (mVVolumeScale.at(iSelectedInterval))) + iVolumeViewPortHighStrip;};

    // QWidget interface
protected:
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);
};

#endif // GRAPHVIEWFORM_H
