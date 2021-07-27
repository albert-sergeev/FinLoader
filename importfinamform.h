#ifndef IMPORTFINAMFORM_H
#define IMPORTFINAMFORM_H

#include <QWidget>
#include<filesystem>

#include "marketslistmodel.h"
#include "tickerslistmodel.h"
#include "storage.h"
#include "bar.h"




namespace Ui {
class ImportFinamForm;
}

class ImportFinamForm : public QWidget
{
    Q_OBJECT

private:
    //std::filesystem::directory_entry drCurr;
    std::filesystem::path pathFile;
    std::filesystem::path pathDir;
    char cDelimiter{','};

    MarketsListModel *modelMarket;
    TickersListModel *modelTicker;
    TickerProxyListModel proxyTickerModel;

    int iDefaultTickerMarket;

    enum fieldType:int {TICKER,PER,DATE,TIME,OPEN,HIGH,LOW,CLOSE,VOL,LAST};

public:
    explicit ImportFinamForm(QWidget *parent = nullptr);
    ~ImportFinamForm();

public:
    void SetDefaultOpenDir(QString &s);
    void SetDelimiter(char c);
    void setMarketModel(MarketsListModel *model, int DefaultTickerMarket);
    void setTickerModel(TickersListModel *model,bool ShowByName,bool SortByName);

signals:
    void OpenImportFilePathChanged(QString &);
    void DelimiterHasChanged(char c);

private slots:

    void slotBtnOpenClicked();
    void slotBtnCreateClicked();
    void slotBtnTestClicked();
    void slotBtnImportClicked();
    void slotEditDelimiterWgtChanged(const QString &);
    void slotShowByNamesChecked(int Checked);

    void slotSetSelectedTickersMarket(const  int i);
    void slotSetSelectedTicker(const  QModelIndex& indx);

    void slotPreparseImportFile();
    bool slotParseLine(std::vector<int> & fieldsType, std::istringstream & issLine, std::istringstream & issTmp, std::ostringstream & ossErr, Bar &b, int ColMax, int DefaultInterval = (-1));

    void showInterval(int Interval);
    void clearShowAreaOfFields();

private:
    Ui::ImportFinamForm *ui;
};

#endif // IMPORTFINAMFORM_H
