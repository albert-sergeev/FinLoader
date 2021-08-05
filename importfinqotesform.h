#ifndef IMPORTFINAMFORM_H
#define IMPORTFINAMFORM_H

#include <QWidget>
#include <QDateTime>
#include<filesystem>

#include "marketslistmodel.h"
#include "tickerslistmodel.h"
#include "storage.h"
#include "bar.h"
#include "datafinloadtask.h"
#include "threadfreecout.h"




namespace Ui {
class ImportFinamForm;
}


class ImportFinQuotesForm : public QWidget
{
    Q_OBJECT

private:

    std::filesystem::path pathFile;
    std::filesystem::path pathDir;
    char cDelimiter{','};

    int iDefaultTickerMarket;

    MarketsListModel * const modelMarket;
    TickersListModel * const modelTicker;
    TickerProxyListModel proxyTickerModel;

    bool bInLoading{false};
    bool bInChecking{false};


    bool bReadyToImport{false};
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

public:
    explicit ImportFinQuotesForm(MarketsListModel *modelM, int DefaultTickerMarket,
                             TickersListModel *modelT,/*bool ShowByName,bool SortByName,*/
                             QWidget *parent = nullptr);
    ~ImportFinQuotesForm();

public:
    void SetDefaultOpenDir(QString &s);
    void SetDelimiter(char c);

public slots:

    void SetProgressBarValue(int);
    void slotLoadingHasBegun();
    void slotLoadingHasFinished(bool bSuccess, QString qsErr);
    void slotLoadingActivity();
    void slotTextInfo(QString qsStr);

signals:
    void OpenImportFilePathChanged(QString &);
    void NeedSaveDefaultTickerMarket(int);
    void DelimiterHasChanged(char c);
    void NeedParseImportFinQuotesFile(dataFinLoadTask &);
    void NeedToStopLoadings();

private slots:

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

private:

    void setMarketModel();
    void setTickerModel();

    Ui::ImportFinamForm *ui;
};





#endif // IMPORTFINAMFORM_H
