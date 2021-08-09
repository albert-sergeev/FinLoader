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

#include <queue>
#include <utility>

#include "configwindow.h"
#include "importfinqotesform.h"
#include "workerloader.h"
#include "bar.h"
#include "marketslistmodel.h"
#include "tickerslistmodel.h"
#include "storage.h"
#include "threadpool.h"
#include "datafinloadtask.h"
#include "databuckgroundthreadanswer.h"
#include "blockfreequeue.h"
#include "bulbululator.h"
#include "styledswitcher.h"



QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    // for main window
    QAction * pacTickersBar;
    QAction * pacStatusBar;
    QAction * pacToolBar;
    QToolBar * tbrToolBar;
    bool bToolBarOnLoadIsHidden;
    bool bTickerBarOnLoadIsHidden;
    bool bStatusBarOnLoadIsHidden;
    //------------------------------------------------
    QMenu * m_mnuWindows;
    QMenu * m_mnuStyles;
    QMenu * m_mnuLangs;
    QSignalMapper * m_psigmapper;
    QSignalMapper * m_psigmapperStyle;
    QSignalMapper * m_psigmapperLang;
    QList<QString> lstStyles;
    QString m_sStyleName;
    QString m_Language;
    QTranslator m_translator;

    // thread manipulation
    BlockFreeQueue<dataFinLoadTask> queueFinQuotesLoad;
    BlockFreeQueue<dataBuckgroundThreadAnswer> queueTrdAnswers;
    workerLoader wrkrLoadFinQuotes;


    // global storage objects
    QSettings m_settings;
    Storage stStore;
    std::vector<Market> vMarketsLst;
    MarketsListModel m_MarketLstModel;

    // for condig subwindow
    int iDefaultTickerMarket;
    bool bConfigTickerShowByName;
    bool bConfigTickerSortByName;
    std::vector<Ticker> vTickersLst;
    TickersListModel m_TickerLstModel;

    // for inport FinQuotes subwindow
    QString qsDefaultOpenDir;
    char cImportDelimiter;

    // for docked bar
    StyledSwitcher * swtShowByName;
    StyledSwitcher * swtShowAll;
    StyledSwitcher * swtShowMarkets;
    TickerProxyListModel proxyTickerModel;


    std::vector<Bulbululator *> vBulbululators;
    std::queue<std::pair<int,std::time_t>> qActivityQueue;

    ///////////////////////////////////
    ThreadPool thrdPoolLoadFinQuotes;
    ///////////////////////////////////
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void SendToLog(QString);
    void SaveUnsavedConfigs();

protected:
    void InitAction();
    void SaveSettings();
    void LoadSettings();
    void LoadDataStorage();
    void SaveDataStorage();
    void InitDockBar();

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


protected slots: // for main window
    void slotNotImpl    ();
    void slotNewDoc     ();
    void slotNewLogWnd  ();
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

    void BulbululatorAddActive      (int TickerID);
    void BulbululatorRemoveActive   (int TickerID);
    void BulbululatorShowActivity   (int TickerID);

    void slotDocbarShowNyNameChanged(int);
    void slotDocbarShowAllChanged   (int);
    void slotDocbarShowMarketChanged(int);

    void slotToolBarStateChanged();
    void slotStatusBarStateChanged();
    void slotTickersBarStateChanged();

    void ListBewShowActivity(int TickerID);


private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
