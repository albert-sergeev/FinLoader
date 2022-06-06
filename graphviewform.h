/****************************************************************************
*  This is part of FinLoader
*  Copyright (C) 2021  Albert Sergeyev
*  Contact: albert.s.sergeev@mail.ru
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <https://www.gnu.org/licenses/>.
****************************************************************************/

#ifndef GRAPHVIEWFORM_H
#define GRAPHVIEWFORM_H


#include <QWidget>
#include<QMdiSubWindow>
#include "ticker.h"
#include "graphholder.h"
#include "blockfreequeue.h"
#include "bargraphicsitem.h"
#include "styledswitcher.h"
#include "plusbutton.h"
#include "memometer.h"
#include "transparentbutton.h"

namespace Ui {
class GraphViewForm;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief RepainTask - used to transmit paint tasks in GraphViewForm
//////////////////////////////////////////////////////////////////////////////////////////////////

struct RepainTask{

    typedef unsigned int Type_type;

    enum eRepaintType:Type_type {InvalidateRepaint = 1, FastBars = 2, FastVolumes = 4, FastFrames = 8, FastAverages = 16, PaintViewport = 32};

    RepainTask(){;}
    RepainTask(std::time_t Start,std::time_t End,bool NeedToRescale){
        Type            = eRepaintType::InvalidateRepaint;

        dtStart         = Start;
        dtEnd           = End;
        bNeedToRescale  = NeedToRescale;
    }
    RepainTask(Type_type tp, int Start,int End,bool NeedToRescale){
        Type            = tp;

        iStart         = Start;
        iEnd           = End;
        bNeedToRescale  = NeedToRescale;
    }
    RepainTask(const RepainTask &o) {
        Type            = o.Type;

        holder          = o.holder;
        iStart          = o.iStart;
        iEnd            = o.iEnd;
        iLetShift       = o.iLetShift;
        bStoreRightPos  = o.bStoreRightPos;
        dtStart         = o.dtStart;
        dtEnd           = o.dtEnd;
        bNeedToRescale  = o.bNeedToRescale;
        bReplacementMode= o.bReplacementMode;
        bInvalidate     = o.bInvalidate;
        bRecalculateAverages = o.bRecalculateAverages;
    }

    Type_type Type{eRepaintType::InvalidateRepaint};

    std::shared_ptr<GraphHolder> holder;

    int         iStart{0};
    int         iEnd{0};
    int         iLetShift{0};
    bool        bReplacementMode{false};
    bool        bInvalidate{false};
    bool        bRecalculateAverages{false};

    bool bStoreRightPos{false};

    std::time_t dtStart {0};
    std::time_t dtEnd   {0};
    bool bNeedToRescale {false};
};
//////////////////////////////////////////////////////////////////////////////////////////////////
/// Cover for Invalidate counter

class InvalidateCounterDefender
{
    std::atomic<int> &aiCounter;
    std::atomic<bool> bFree;
public:
    InvalidateCounterDefender(std::atomic<int> &Counter):aiCounter{Counter},bFree{false}{
        int iCounter = aiCounter.load();
        int iCounterNew = iCounter + 1;
        while(!aiCounter.compare_exchange_weak(iCounter,iCounterNew)){;}
    };
    ~InvalidateCounterDefender(){
        free();
    }
    void free(){
        bool free = bFree.load();
        if (!free){
            while(!bFree.compare_exchange_weak(free,true )){
                if (free) return;
                free = bFree.load();
            }
            int iCounter = aiCounter.load();
            int iCounterNew = iCounter - 1;
            while(!aiCounter.compare_exchange_weak(iCounter,iCounterNew )){;}
        }
    }

};

//////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief The GraphViewForm class
//////////////////////////////////////////////////////////////////////////////////////////////////
///
inline std::once_flag GraphViewForm_init_consts_call_once_flag;

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

    Memometer  *  indicatorMemo;

    QPushButton  *btnHelp;
    QPushButton  *btnHelpR;

    TransparentButton *btnScaleHViewDefault;
    TransparentButton *btnScaleVViewDefault;
    TransparentButton *btnScaleVVolumeDefault;

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

    std::map<int,QPointF>      mMovingBlue;
    std::map<int,QPointF>      mMovingRed;
    std::map<int,QPointF>      mMovingGreen;

    std::chrono::time_point<std::chrono::steady_clock> dtFastShowAverageActivity;
    std::set<int> stFastShowAverages;

    QGraphicsPathItem *pathBlue;
    QGraphicsPathItem *pathRed;
    QGraphicsPathItem *pathGreen;

    std::map<int,std::vector<QGraphicsItem *>>      mVFramesViewQuotes;
    std::map<int,std::vector<QGraphicsItem *>>      mVFramesScaleUpper;
    std::map<int,std::vector<QGraphicsItem *>>      mVFramesVolume;
    std::map<int,std::vector<QGraphicsItem *>>      mVFramesHorisSmallScale;
    std::map<int,std::vector<QGraphicsItem *>>      mVFramesHorisSmallScaleExtremities;

    std::vector<std::pair<QGraphicsItem *,double>>  vHorizFramesViewQuotes;
    std::vector<QGraphicsItem *>                    vHorizFramesScaleUpper;

    std::map<int,std::vector<QGraphicsItem *>>      mLeftFrames;
    std::map<int,std::vector<QGraphicsItem *>>      mRightFrames;

    std::map<int,std::pair<std::time_t,bool>>       mTimesScale;

    QFont fontTime;
    QFont fontNumb;

    static int iConstWidthNumb1;
    static int iConstWidthNumb2;
    static int iConstWidthTime;


    std::atomic<int> aiInvalidateCounter;

    friend class InvalidateCounterDefender;

    InvalidateCounterDefender defInit;



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
    double dStoredVValue;
    double dHScale;
    std::map<int,double> mVScale;
    std::map<int,double> mVVolumeScale;

    std::vector<std::pair<double,bool>> vHLines;
    bool bOHLC;

    bool bInvertMouseWheel;

    static const int iLeftShift{20};
    static const int iRightShift{50};

    static const int iViewPortHeight{600};
    static const int iViewPortHighStrip{20};
    static const int iViewPortLowStrip{20};

    static const int iVolumeViewPortHighStrip{3};

public:
    explicit GraphViewForm(const int TickerID, std::vector<Ticker> &v, std::shared_ptr<GraphHolder> hldr, QWidget *parent = nullptr);
    ~GraphViewForm();

    inline int TickerID() const {return iTickerID;};

    double GetHScale() {return mVScale[iSelectedInterval];}

    bool IsOHLC() const {return bOHLC;};

public:
signals:

    void SendToLog(QString);
    void NeedLoadGraph(const  int iTickerID, const std::time_t tBegin, const std::time_t tEnd);
    void NeedSaveTickerConig(const Ticker tT, const bool bFull);


public slots:

    void slotInvalidateGraph(std::time_t dtDegin, std::time_t dtEnd, bool bNeedToRescale = false);
    void slotProcessRepaintQueue();
    void setFramesVisibility(std::tuple<bool,bool,bool,bool,bool>);
    void slotFastShowEvent(std::shared_ptr<GraphHolder> ptrHolder);

    void slotSaveUnsavedConfigs();

    void slotUsedMemoryChanged(size_t,size_t);

    void slotInvertMouseWheelChanged(bool);
    void slotShowHelpButtonsChanged(bool);

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

    void slotCandleStateChanged(int);

    void slotScaleHViewDefaultClicked();
    void slotScaleVViewDefaultClicked();
    void slotScaleVVolumeDefaultClicked();

    void slotSetNewHScaleQuotes(double dNewScale);
    void slotSetNewVScaleQuotes(double dNewScale);
    void slotSetNewVScaleVolume(double dNewScale);

private:
    Ui::GraphViewForm *ui;

protected:

    //-----------------------------------------------------------------------------------------------
    // globals
//    template<typename T>
//    void PaintBarsFastT(std::time_t tBegin, std::time_t tEnd,std::shared_ptr<GraphHolder> ptrHolder);

    template<typename T>
    bool RepainInvalidRange(RepainTask &);

    void SetSliderToVertPos(double dPos);

    void SetSliderToPos (std::time_t tRightPos, int iRightAggregate);
    template<typename T>
    void SetSliderToPosT(std::time_t tRightPos, int iRightAggregate);

    void InvalidateScenes();


    //-----------------------------------------------------------------------------------------------
    // intelectual paint procedures

    std::pair<int,int> getViewPortRangeToHolder();

    bool PaintViewPort               (bool bFrames,bool bBars,bool bVolumes, bool bStoreRightPos, bool bInvalidate);
    void PaintViewPort               (int iStart, int iEnd,bool bFrames ,bool bBars,bool bVolumes, bool bStoreRightPos, bool bInvalidate);

    bool FastLoadHolder(RepainTask &);
    bool FastPaintBars(RepainTask &);
    bool FastPaintFrames(RepainTask &);
    bool FastPaintAverages(RepainTask &);

    void checkFastShowAverages(int iStart, int iEnd);

    template<typename T>
    bool PaintBars       (std::shared_ptr<GraphHolder> holder, int iStart, int iEnd,
                                    bool bPaintBars, bool bPaintVolumes,
                                    bool bStoreRightPos, bool bReplacementMode);

    bool PaintMovingAverages (std::shared_ptr<GraphHolder> local_holder,
                                                int iStartI, int iEndI,
                                                bool bReplacementMode);

    bool PaintHorizontalScales       ();
    bool PaintHorizontalFrames       ();
    bool PaintVerticalSideScales     ();
    bool PaintVerticalFrames         (std::shared_ptr<GraphHolder> local_holder,int iStart, int iEnd,
                                             int iLeftStock, bool bReplacementMode, bool bInvalidate);

    template<typename T>
    bool PainVerticalFramesT         (std::shared_ptr<GraphHolder> local_holder,int iStart, int iEnd,
                                            int iLeftStock, bool bReplacementMode, bool bInvalidate);
    //-----------------------------------------------------------------------------------------------
    // drawed objects manipulation functions

    template<typename T>
    void EraseLinesUpper(T& mM, int iStart, QGraphicsScene *);
    template<typename T>
    void EraseLinesLower(T& mM, int iEnd, QGraphicsScene *);
    template<typename T>
    void EraseLinesMid(T& mM, int iStart,int iEnd, QGraphicsScene *);

    void Erase();
    void EraseTimeScale();
    void EraseBars();
    void EraseMovingAverages();
    void EraseVolumes();
    void EraseFrames();

    void EraseInvariantFrames      (bool bHorizontal,bool bRepain);
    //-----------------------------------------------------------------------------------------------
    // draw primitives

    void DrawLineToScene(const int idx,const  qreal x1,const  qreal y1,const qreal x2,const  qreal y2,
                         std::map<int,std::vector<QGraphicsItem *>>& mM, QGraphicsScene *scene, const QPen & pen,
                         const  std::time_t t = 0, bool bHasTooltip = false, const qreal zvalue = 0);
    void DrawLineToScene(const int idx,const  qreal x1,const  qreal y1,const qreal x2,const  qreal y2,
                         std::map<int,std::vector<QGraphicsItem *>>& mM, QGraphicsScene *scene, const QPen & pen,
                         const  std::string sToolTip, bool bHasTooltip = false, const qreal zvalue = 0);
    void DrawIntermittentLineToScene(const int idx,const  qreal x1,const  qreal y1,const qreal x2,const  qreal y2,
                         std::map<int,std::vector<QGraphicsItem *>>& mM, QGraphicsScene *scene, const QPen & pen,
                         const  std::time_t t, bool bHasTooltip, const qreal zvalue = 0);
    void DrawIntermittentLineToScene(const int idx,const  qreal x1,const  qreal y1,const qreal x2,const  qreal y2,
                         std::map<int,std::vector<QGraphicsItem *>>& mM, QGraphicsScene *scene, const QPen & pen,
                         const  std::string sToolTip, bool bHasTooltip = false, const qreal zvalue = 0);



    void DrawTimeToScene(const int idx,const  qreal x,const  qreal y,const  std::tm &,
                                        std::map<int,std::vector<QGraphicsItem *>>& mM, QGraphicsScene *scene, const QFont & font, const qreal zvalue = 0);
    void DrawIntToScene(const int idx,const  qreal x,const  qreal y,const  int n, Qt::AlignmentFlag alignH, Qt::AlignmentFlag alignV,
                                        std::map<int,std::vector<QGraphicsItem *>>& mM, QGraphicsScene *scene, const QFont & font, const qreal zvalue = 0);
    void DrawDoubleToScene(const int idx,const  qreal x ,const  qreal y,const double n, Qt::AlignmentFlag alignH, Qt::AlignmentFlag alignV,
                           std::map<int,std::vector<QGraphicsItem *>>& mM, QGraphicsScene *scene, const QFont & font, const qreal zvalue = 0);

    //-----------------------------------------------------------------------------------------------
    // curve line functions

    float distance(const QPointF& pt1, const QPointF& pt2);
    QPointF getLineStart(const QPointF& pt1, const QPointF& pt2);
    QPointF getLineEnd(const QPointF& pt1, const QPointF& pt2);
    QPainterPath smoothOut(const std::map<int,QPointF> &map, const float& factor);

    //-----------------------------------------------------------------------------------------------
    // utility functions

    void init_const();


    void ReDoHLines();
    void RefreshHLines();
    std::tuple<int,int,int,int> getHPartStep(double realH, double viewportH);
    std::string MemoSizeToStr(size_t iSize);

    void SetMinMaxDateToControls();
    void RepositionPlusMinusButtons();
    void SetSelectedIntervalToControls();
    void ResizeMemometer();


    inline double realYtoSceneY      (double y) {return  ((y - dStoredLowMin)       * (mVScale.at      (iSelectedInterval))) + iViewPortLowStrip;};
    inline double realYtoSceneYVolume(double y) {return  ((y - 0) * (mVVolumeScale.at(iSelectedInterval))) + iVolumeViewPortHighStrip;};


    inline double sceneYtoRealY      (double yScene) {
        return   (yScene - iViewPortLowStrip) / (mVScale.at      (iSelectedInterval)) + dStoredLowMin;

    };

    //-----------------------------------------------------------------------------------------------

    // QWidget interface
protected:
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);

    // QObject interface
public:
    bool eventFilter(QObject *watched, QEvent *event);
    virtual bool event(QEvent *event);
};

inline int GraphViewForm::iConstWidthNumb1;
inline int GraphViewForm::iConstWidthNumb2;
inline int GraphViewForm::iConstWidthTime;

#endif // GRAPHVIEWFORM_H
