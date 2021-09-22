#ifndef MAINWINDOW_H
#define MAINWINDOW_H

//#include <QtWidgets>
#include <QMainWindow>
#include <QSignalMapper>
#include <QTranslator>
#include <QSettings>
#include <QStyle>
#include <QDebug>
#include <QToolBar>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QMdiSubWindow>
#include <QMessageBox>
#include <QStyleFactory>
#include <QFile>
#include <QTextEdit>
#include <QLCDNumber>

#include <queue>
#include <chrono>
#include <utility>

#include "configwindow.h"
#include "importfinqotesform.h"
#include "workerloader.h"
#include "bar.h"
#include "modelmarketslist.h"
#include "modeltickerslist.h"
#include "storage.h"
#include "threadpool.h"
#include "datafinloadtask.h"
#include "databuckgroundthreadanswer.h"
#include "blockfreequeue.h"
#include "bulbululator.h"
#include "styledswitcher.h"
#include "graphviewform.h"
#include "graphholder.h"
#include "amipipesform.h"
#include "amipipeholder.h"
#include "dataamipipeanswer.h"
#include "dataamipipetask.h"
#include "combindicator.h"

#ifdef _WIN32

#include<windows.h>

#endif




QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


using seconds=std::chrono::duration<double>;
using milliseconds=std::chrono::duration<double,
    std::ratio_multiply<seconds::period,std::milli>
    >;


inline std::once_flag mainwindow_test_call_once_flag;

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    // for main window

    int iTimerID;

    QMenu * m_mnuWindows;
    QMenu * m_mnuStyles;
    QMenu * m_mnuLangs;
    QMenu * m_mnuGraphViewConfig;
    QMenu * m_mnuAmiPipePanels;

    QMenu * pmnuFile;
    QMenu * pmnuTools;
    QMenu * pmnuSettings;
    QMenu * pmnuHelp;

    QAction * pacAmiPipe;
    QAction * pacAmiPipeBarNew;
    QAction * pacAmiPipeBarActive;
    QAction * pacTickersBar;
    QAction * pacStatusBar;
    QAction * pacToolBar;
    QAction * pacTickersBarButtonsHide;

    QAction * pacGVLeftSc;
    QAction * pacGVRightSc;
    QAction * pacGVUpperSc;
    QAction * pacGVLowerSc;
    QAction * pacGVVolumeSc;


    //QStatusBar * statusBarTickers;

    CombIndicator * wtCombIndicator;
    QLCDNumber * lcdN;
    std::time_t tLocalStoredPacketActivity{0};

    QToolBar * tbrToolBar;
    bool bToolBarOnLoadIsHidden;
    bool bTickerBarOnLoadIsHidden;
    bool bStatusBarOnLoadIsHidden;
    bool bTickerBarButtonsHidden;

    bool bGVLeftScOnLoadIsHidden;
    bool bGVRightScOnLoadIsHidden;
    bool bGVUpperScOnLoadIsHidden;
    bool bGVLowerScOnLoadIsHidden;
    bool bGVVolumeScOnLoadIsHidden;

    bool bFillNotAutoloadedTickers;
    bool bGrayColorFroNotAutoloadedTickers;
    int iDefaultMonthDepth;
    //------------------------------------------------
    std::map<int,std::shared_ptr<GraphHolder>> Holders;
    std::queue<std::pair<int,std::chrono::time_point<std::chrono::steady_clock>>> qActivityQueue;
    std::map<int,std::pair<bool,std::chrono::time_point<std::chrono::steady_clock>>> mBlinkedState;
    //------------------------------------------------

    QSignalMapper * m_psigmapper;
    QSignalMapper * m_psigmapperStyle;
    QSignalMapper * m_psigmapperLang;
    QList<QString> lstStyles;
    QString m_sStyleName;
    QString m_Language;
    QTranslator m_translator;

    std::chrono::time_point<std::chrono::steady_clock> dtCheckMemoryUsage;
    std::size_t iStoredUsedMemory;
    std::size_t iPhisicalMemory;

    // global storage objects
    QSettings m_settings;
    std::vector<Market> vMarketsLst;
    modelMarketsList m_MarketLstModel;
    QString qsStorageDirPath;
    bool bDefaultStoragePath;

    // for condig subwindow
    int iDefaultTickerMarket;
    bool bConfigTickerShowByName;
    bool bConfigTickerSortByName;
    std::vector<Ticker> vTickersLst;
    std::vector<Ticker> vTickersLstEtalon;
    modelTickersList m_TickerLstModel;

    // for inport FinQuotes subwindow
    QString qsDefaultOpenDir;
    char cImportDelimiter;

    // for GraphViewForm;


    // for AmiPipes
    AmiPipeHolder pipesHolder;
    std::chrono::time_point<std::chrono::steady_clock> dtCheckPipesActivity;
    std::map<int,int> mStoredUnconnected;
    FastTasksHolder fastHolder;
    AmiPipesForm  *pAmiPipeWindow;
    bool bAmiPipeShowByNameUnallocated;
    bool bAmiPipeShowByNameActive;
    bool bAmiPipeShowByNameOff;

    // for docked bar
    StyledSwitcher * swtShowByName;
    StyledSwitcher * swtShowAll;
    StyledSwitcher * swtShowMarkets;
    TickerProxyListModel proxyTickerModel;
    int iStoredLeftDocbarRightPos;
    int iStoredLeftDocbarWidth;
    bool bInResizingLeftToolbar;
    bool bLeftToolbarCursorOverriden;

    QDockWidget *wtAmiDockbar;
    int iStoredAmiPipeFormWidth;
    int iStoredTickerBarWidth;
    bool bAmiPipesFormShown;

    bool bAmiPipesNewWndShown;
    bool bAmiPipesActiveWndShown;


    std::vector<Bulbululator *> vBulbululators;

    // thread manipulation
    bool bWasClose{false};
    BlockFreeQueue<dataFinLoadTask> queueFinQuotesLoad;
    BlockFreeQueue<dataBuckgroundThreadAnswer> queueTrdAnswers;
    BlockFreeQueue<dataAmiPipeTask> queuePipeTasks;
    BlockFreeQueue<dataAmiPipeAnswer> queuePipeAnswers;
    BlockFreeQueue<dataFastLoadTask>  queueFastTasks;
    workerLoader wrkrLoadFinQuotes;

    /// TODO: delete for tests
    std::chrono::time_point<std::chrono::steady_clock> dtSpeedCounter;
    void initTestConst();

    ///////////////////////////////////
    Storage stStore;
    ///////////////////////////////////
    ThreadPool thrdPoolLoadFinQuotes;
    ThreadPool thrdPoolAmiClient;
    ThreadPool thrdPoolFastDataWork;
    ///////////////////////////////////
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();



signals:
    void SendToLog(QString);
    void SendToErrorLog(QString);
    void SaveUnsavedConfigs();

    void slotSendSignalToProcessRepaintQueue();

    void UsedMemoryChanged(size_t,size_t);

    void AmiPipeInternalPanelsStateChanged(bool bLeft, bool bRight);

    void PipeNameReceived(std::string,std::string);

protected:
    void InitAction();
    void SaveSettings();
    void LoadSettings();
    void LoadDataStorage();
    void SaveDataStorage();
    void InitDockBar();
    void InitHolders();

    bool event(QEvent *event) override;
    void timerEvent(QTimerEvent * event) override;

public slots: // for config window
    void slotSaveMarketDataStorage();
    void slotTickerDataStorageUpdate(const QModelIndex &,const QModelIndex &);
    void slotTickerDataStorageRemove(const Ticker & tT);
    void slotStoreDefaultTickerMarket(int indx)     {iDefaultTickerMarket = indx;};
    void slotStoreConfigTickerShowByName(bool b)    {bConfigTickerShowByName = b;};
    void slotStoreConfigTickerSortByName(bool b)    {bConfigTickerSortByName = b;};

public slots: // for import FinQuotes winow
    void slotDefaultOpenDirChanged(QString & s) {qsDefaultOpenDir = s;};
    void slotImportDelimiterChanged(char c)     {cImportDelimiter = c;};
    void slotParseImportFinQuotesFile(dataFinLoadTask &);
    void slotStopFinQuotesLoadings();

    void slotLoadGraph(const  int iTickerID);
    void slotLoadGraph(const  int iTickerID, const std::time_t tBegin, const std::time_t tEnd);

    void slotSaveNewDefaultPath(bool,QString);
    void slotSaveGeneralOptions(bool,bool,int);

public slots: // for amipipe form

    void slotAmiPipeSaveShowByNamesUnallocated(bool);
    void slotAmiPipeSaveShowByNamesActive(bool);
    void slotAmiPipeSaveShowByNamesOff(bool);

    void slotAmiPipeWidthWasChanged(int);

    void slotAmiPipeNewWndStateChanged(int);
    void slotAmiPipeActiveWndStateChanged(int);

    void slotAmiPipeHideClicked();

    void slotAmiPipeWndowNew();
    void slotAmiPipeWndowActive();

    void slotAskPipesNames(dataAmiPipeTask::pipes_type &pipesFree);

protected slots: // for main window
    void slotNotImpl    ();
    void slotNewDoc     ();
    void slotGraphViewWindow();
    void slotNewLogWnd  ();
    void slotNewErrLogWnd();
    void slotWindows    ();
    void slotStyles     ();
    void slotLanguages  ();
    void slotAbout      ();
    void slotSendTestText();
    void slotSetActiveSubWindow (QWidget*);
    void slotSetActiveStyle     (QString);
    void slotSetActiveLang      (QString);
    void slotConfigWndow ();
    void slotImportFinQuotesWndow ();
    void slotAmiPipeWndow();

    void BulbululatorAddActive      (int TickerID);
    void BulbululatorRemoveActive   (int TickerID);
    void BulbululatorShowActivity   (int TickerID);
    void BulbululatorSetState(int TickerID,Bulbululator::eTickerState State);

    void slotDocbarShowNyNameChanged(int);
    void slotDocbarShowAllChanged   (int);
    void slotDocbarShowMarketChanged(int);

    void slotToolBarStateChanged();
    void slotStatusBarStateChanged();
    void slotTickersBarStateChanged();
    void slotTickersBarButtonsStateChanged();

    void ListViewShowActivity(int TickerID);
    void ListViewActivityTermination();

    void slotSetSelectedTicker(const  QModelIndex&);
    void slotSetSelectedTicker(const  int iTickerID);

    void slotSendSignalToInvalidateGraph(int TickerID, std::time_t dtDegin, std::time_t dtEnd);
    void slotSendSignalToFastShow(int TickerID, std::time_t tBegin, std::time_t tEnd,std::shared_ptr<GraphHolder> ptrHolder);


    //void slotTestPvBars(std::shared_ptr<std::vector<std::vector<BarTick>>> pvBars); // TODO: delete. for tests

    void slotGVFramesVisibilityStateChanged();

    void slotAmiPipeFormWasClosed();

    void CheckActivePipes();
    void CheckActiveProcesses();
    void CheckLastPacketTime();
    void CheckUsedMemory();

    void BuildSessionsTableForFastTasks(FastTasksHolder &);

    void slotSaveTickerConigRef(const Ticker & tT, bool bFull = false);
    void slotSaveTickerConig(const Ticker tT, const bool bFull);

    std::size_t getPhisicalMemory();

    void ResizingLeftToolBars();

private:
    //std::vector<std::vector<Bar>> testPvBars; // TODO: delete. for tests

    Ui::MainWindow *ui;


    // QWidget interface


    // QObject interface
public:
    bool eventFilter(QObject *watched, QEvent *event);
    bool eventTickerBar(QObject *watched, QEvent *event);
};
#endif // MAINWINDOW_H
