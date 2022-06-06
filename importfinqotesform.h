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

#ifndef IMPORTFINAMFORM_H
#define IMPORTFINAMFORM_H

#include <QWidget>
#include <QDateTime>
#include<filesystem>

#include "modelmarketslist.h"
#include "modeltickerslist.h"
#include "storage.h"
#include "bar.h"
#include "datafinloadtask.h"
#include "threadfreecout.h"
#include "configwindow.h"




namespace Ui {
class ImportFinamForm;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Form for import data from files
///
class ImportFinQuotesForm : public QWidget
{
    Q_OBJECT

private:

    //----------------------------------------------------------------
    // interface elements data

    StyledSwitcher * swtShowByName;

//    QMenu   *pImportContextMenu;
//    QAction * pacCheck;
//    QAction * pacImport;
    bool bPacCheckEnabled;
    bool bPacCheckInMemoryEnabled;
    bool bPacImport;


    //----------------------------------------------------------------
    // core data

    // filesystem path
    std::filesystem::path pathFile;
    std::filesystem::path pathDir;
    char cDelimiter{','};

    // stored selected market
    int iDefaultTickerMarket;

    // data to display market & ticker data
    modelMarketsList * const modelMarket;
    modelTickersList * const modelTicker;
    TickerProxyListModel proxyTickerModel;

    // form state
    bool bInLoading{false};
    bool bInChecking{false};

    //-----------------------------------------------
    // elements used during parsing:

    bool bReadyToImport{false};
    bool bCheckingInMemory{false};
    int iTimePeriod{0};
    int iSelectedTickerId{0};
    int iFoundTickerId{0};
    std::string sFoundTickerSignFinam{""};
    std::string sSelectedTickerSignFinam{""};
    bool bFoundTicker{false};
    bool bFoundTickerFinam{false};
    dataFinQuotesParse parseDataReady;

    QDateTime qdtMin;
    QDateTime qdtMax;
    //-----------------------------------------------

public:
    explicit ImportFinQuotesForm(modelMarketsList *modelM, int DefaultTickerMarket,
                             modelTickersList *modelT,/*bool ShowByName,bool SortByName,*/
                             QWidget *parent = nullptr);
    ~ImportFinQuotesForm();

public:
    // used to initialize

    void SetDefaultOpenDir(QString &s);
    void SetDelimiter(char c);

public slots:

    // used to receive events from main form

    void SetProgressBarValue(int);
    void slotLoadingHasBegun();
    void slotLoadingHasFinished(bool bSuccess, QString qsErr);
    void slotLoadingActivity();
    void slotTextInfo(QString qsStr);
    void slotShowHelpButtonsChanged(bool);

signals:

    // used to send changes/tasks to mainform (and then store settings):

    void OpenImportFilePathChanged(QString &);
    void NeedSaveDefaultTickerMarket(int);
    void DelimiterHasChanged(char c);
    void NeedParseImportFinQuotesFile(dataFinLoadTask &);
    void NeedToStopLoadings();

private slots:

    // interface slots

    void slotBtnOpenClicked();
    void slotBtnCreateClicked();
    void slotBtnTestClicked();
    void slotBtnImportClicked();
    void slotEditDelimiterWgtChanged(const QString &);
    void slotShowByNamesChecked(int Checked);

    void slotDateTimeStartChanged(const QDateTime &);
    void slotDateTimeEndChanged(const QDateTime &);


    void slotSetSelectedTickersMarket(const  int i);
    void slotSetSelectedTicker(const  QModelIndex& indx);
    void slotSetSelectedTicker(const  QModelIndex& indx,const  QModelIndex& ) {slotSetSelectedTicker(indx);};

    void slotPreparseImportFile();

    void showInterval(int Interval);
    void clearShowAreaOfFields();

    void slotSetWidgetsInLoadState(bool);

    void slotSetToCheck();
    void slotSetToCheckInMemory();
    void slotSetToImport();

private:

    void setMarketModel();
    void setTickerModel();

    Ui::ImportFinamForm *ui;

    // QObject interface
public:
    bool eventFilter(QObject *watched, QEvent *event);
};





#endif // IMPORTFINAMFORM_H
