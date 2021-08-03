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

#include "configwindow.h"
#include "importfinamform.h"
#include "workerloaderfinam.h"

#include "bar.h"
#include "marketslistmodel.h"
#include "tickerslistmodel.h"
#include "storage.h"
#include "threadpool.h"
#include "datafinamloadtask.h"
#include "databuckgroundthreadanswer.h"
#include "blockfreequeue.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    // for main window
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
    ThreadPool thrdPoolLoadFinam;
    BlockFreeQueue<dataFinamLoadTask> queueFilamLoad;
    BlockFreeQueue<dataBuckgroundThreadAnswer> queueTrdAnswers;
    workerLoaderFinam wrkrLoadFinam;


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

    // for inport finam subwindow
    QString qsDefaultOpenDir;
    char cImportDelimiter;



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

    bool event(QEvent *event) override;
    void timerEvent(QTimerEvent * event) override;

public slots: // for config window
    void slotSaveMarketDataStorage();
    void slotTickerDataStorageUpdate(const QModelIndex &,const QModelIndex &);
    void slotTickerDataStorageRemove(const Ticker & tT);
    void slotStoreDefaultTickerMarket(int indx)     {iDefaultTickerMarket = indx;};
    void slotStoreConfigTickerShowByName(bool b)    {bConfigTickerShowByName = b;};
    void slotStoreConfigTickerSortByName(bool b)    {bConfigTickerSortByName = b;};

public slots: // for import finam winow
    void slotDefaultOpenDirChanged(QString & s) {qsDefaultOpenDir = s;};
    void slotImportDelimiterChanged(char c)     {cImportDelimiter = c;};
    void slotParseImportFinamFile(dataFinamLoadTask &);


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
    void slotImportFinamWndow ();


private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
