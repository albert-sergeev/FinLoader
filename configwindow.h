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

#ifndef CONFIGWINDOW_H
#define CONFIGWINDOW_H

//TODO: shrink includes
#include <QWidget>
#include <QTime>
#include <QSortFilterProxyModel>

#include "modelmarketslist.h"
#include "modeltickerslist.h"
#include "storage.h"
#include "styledswitcher.h"
#include "modelsessions.h"

namespace Ui {
class ConfigWindow;
}

class ConfigWindow : public QWidget
{
    Q_OBJECT

//////////////////////////////////////////
/// common work part
//////////////////////////////////////////
public:

    explicit ConfigWindow(modelMarketsList *modelM,int iDefaultTickerMarket,
                          modelTickersList *modelT, bool ShowByName,bool SortByName,
                          bool DefStoragePath, QString StoragePath,
                          Storage &stStore,
                          bool bFillNotAutoloadedTickers,
                          bool bGrayColorFroNotAutoloadedTickers,
                          int iDefaultMonthDepth,
                          bool bSaveLogToFile,
                          int iLogSize,
                          int iLogCount,
                          bool bSaveErrorLogToFile,
                          int iErrorLogSize,
                          int iErrorLogCount,
                          bool bInvertMouseWheel,
                          bool bShowHelpButtons,
                          bool bWhiteBackgtound,
                          bool bShowIntroductoryTips,
                          QWidget *parent = nullptr);
    ~ConfigWindow();
private:
    bool event(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:




    Storage &stStore;
    bool bDataMarketChanged;
    bool bAddingMarketRow;
    bool bIsAboutMarkerChanged;
    bool bDataMarketSessionTablesChanged;

    bool bDataTickerChanged;
    bool bAddingTickerRow;
    bool bIsAboutTickerChanged;

    bool bDataGeneralChanged;
    bool bDataGeneralOptionsChanged;
    bool bDefStoragePath;
    QString qsStoragePath;

    bool bFillNotAutoloadedTickers;
    bool bGrayColorFroNotAutoloadedTickers;
    int iDefaultMonthDepth;

    StyledSwitcher *swtAutoLoadTicker;
    StyledSwitcher *swtUpToSysTicker;
    StyledSwitcher *swtBulbululatorTicker;

    StyledSwitcher *swtShowByNameTicker;
    StyledSwitcher *swtSortByNameTicker;

    StyledSwitcher *swtAutoLoadWholeMarket;
    StyledSwitcher *swtUpToSysWholeMarket;

    StyledSwitcher *swtDefPath;
    StyledSwitcher *swtFillNotAutoloadedTickers;
    StyledSwitcher *swtGrayColorFroNotAutoloadedTickers;

    StyledSwitcher *swtSaveLogToFile;
    StyledSwitcher *swtSaveErrorLogToFile;
    StyledSwitcher *swtInvertMouseWheel;
    StyledSwitcher *swtShowHelpButtons;
    StyledSwitcher *swtWhiteBackgtound;
    StyledSwitcher *swtIntroductoryTips;

    bool bDefaultSaveLogToFile;
    int iDefaultLogSize;
    int iDefaultLogCount;
    bool bDefaultSaveErrorLogToFile;
    int iDefaultErrorLogSize;
    int iDefaultErrorLogCount;
    bool bDefaultInvertMouseWheel;
    bool bDefaultShowHelpButtons;
    bool bDefaultWhiteBackgtound;
    bool bDefaultShowIntroductoryTips;


    int iDefaultTickerMarket;
    modelMarketsList *modelMarket;
    modelTickersList *modelTicker;
    TickerProxyListModel proxyTickerModel;


    Market::SessionTable_type sessionTable;
    modelSessions       modelSessionTable;
    modelSessionsProxy  modelSessionTableProxy;

    Market::SessionTable_type sessionTableRepo;
    modelSessions       modelSessionTableRepo;
    modelSessionsProxy  modelSessionTableRepoProxy;


//////////////////////////////////////////
/// Market work part
//////////////////////////////////////////

public:

signals:
    void SendToMainLog(QString);
    void NeedSaveMarketsChanges();
    void NeedToReboot();
public slots:
    void slotBtnAddMarketClicked();
    void slotBtnRemoveMarketClicked();
    void slotBtnSaveMarketClicked();
    void slotBtnCancelMarketClicked();
protected slots:
    void slotSetSelectedMarket(const  QModelIndex& indx);
    void slotSetSelectedMarket(const  QModelIndex& indx,const  QModelIndex&) {slotSetSelectedMarket(indx);};
    void slotMarketDataChanged(bool Changed=true);
    void slotMarketDataChanged(int)               {slotMarketDataChanged(true);};
    void slotMarketDataChanged(const QString &)   {slotMarketDataChanged(true);};
    void slotMarketTimeChanged(const QTime &)     {slotMarketDataChanged(true);};
    void slotMarketDataChanged(const QModelIndex &,const QModelIndex &,const QVector<int>&);
    void ClearMarketWidgetsValues();
    void slotAboutQuit();

    void slotBtnSessionAddPeriodClicked();
    void slotBtnSessionInsertPeriodClicked();
    void slotBtnSessionDeletePeriodClicked();
    void slotBtnSessionAddTimeRangeClicked();
    void slotBtnSessionInsertTimeRangeClicked();
    void slotBtnSessionDeleteTimeRangeClicked();
    void slotBtnSessionSetDefaultClicked();

    void slotBtnRepoAddPeriodClicked();
    void slotBtnRepoInsertPeriodClicked();
    void slotBtnRepoDeletePeriodClicked();
    void slotBtnRepoAddTimeRangeClicked();
    void slotBtnRepoInsertTimeRangeClicked();
    void slotBtnRepoDeleteTimeRangeClicked();
    void slotBtnRepoSetDefaultClicked();



//////////////////////////////////////////
/// Ticker work part
//////////////////////////////////////////

public:

signals:

    void NeedSaveDefaultTickerMarket(int);
    void NeedSaveShowByNames(bool);
    void NeedSaveSortByNames(bool);

public slots:
    void slotBtnAddTickerClicked();
    void slotBtnRemoveTickerClicked();
    void slotBtnSaveTickerClicked();
    void slotBtnCancelTickerClicked();
protected slots:

    void slotSetSelectedTickersMarket(const  int i);

    void slotSetSelectedTicker(const  QModelIndex& indx);
    void slotSetSelectedTicker(const  QModelIndex& indx,const  QModelIndex&) {slotSetSelectedTicker(indx);};
    void slotTickerDataChanged(bool Changed=true);
    void slotTickerDataChanged(int)               {slotTickerDataChanged(true);};
    void slotTickerDataChanged(const QString &)   {slotTickerDataChanged(true);};
    void slotShowByNamesChecked(int Checked);
    void slotSortByNamesChecked(int Checked);
    void ClearTickerWidgetsValues();
    void setEnableTickerWidgets(bool);

//////////////////////////////////////////
/// General tab work part
//////////////////////////////////////////

public:

signals:
    void NeedChangeDefaultPath(bool,QString);
    void NeedSaveGeneralOptions(bool,bool,int, bool,int,int,bool,int,int,bool,bool,bool,bool);

protected slots:

    void slotDefPathChenged(int);
    void slotStoragePathChanged(const QString &);
    void slotSetPathVisibility();

    void slotGeneralSaveClicked();
    void slotGeneralCancelClicked();
    void slotGeneralOpenStorageDirClicked();

//    void slotFillNotAutoloadedChenged(int);
//    void slotGrayColorChenged(int);
//    void slotMonthDepthChenged(int);
    void slotGeneralOptionChenged(int);

//    void slotAboutQuit();


private:

    void setMarketModel();
    void setTickerModel();

    Ui::ConfigWindow *ui;
};

#endif // CONFIGWINDOW_H
