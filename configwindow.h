#ifndef CONFIGWINDOW_H
#define CONFIGWINDOW_H

//TODO: shrink includes
#include <QWidget>
#include <QTime>
#include <QSortFilterProxyModel>

#include "marketslistmodel.h"
#include "tickerslistmodel.h"
#include "storage.h"
#include "styledswitcher.h"

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

    explicit ConfigWindow(MarketsListModel *modelM,int iDefaultTickerMarket,
                          TickersListModel *modelT, bool ShowByName,bool SortByName,
                          QWidget *parent = nullptr);
    ~ConfigWindow();
private:
    bool event(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:




    Storage stStore;
    bool bDataMarketChanged;
    bool bAddingMarketRow;
    bool bIsAboutMarkerChanged;

    bool bDataTickerChanged;
    bool bAddingTickerRow;
    bool bIsAboutTickerChanged;

    StyledSwitcher *swtAutoLoadTicker;
    StyledSwitcher *swtUpToSysTicker;
    StyledSwitcher *swtBulbululatorTicker;

    StyledSwitcher *swtShowByNameTicker;
    StyledSwitcher *swtSortByNameTicker;

    StyledSwitcher *swtAutoLoadWholeMarket;
    StyledSwitcher *swtUpToSysWholeMarket;

    int iDefaultTickerMarket;
    MarketsListModel *modelMarket;
    TickersListModel *modelTicker;
    TickerProxyListModel proxyTickerModel;



//////////////////////////////////////////
/// Market work part
//////////////////////////////////////////

public:

signals:
    void SendToMainLog(QString);
    void NeedSaveMarketsChanges();
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
    void ClearMarketWidgetsValues();
    void slotAboutQuit();

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

//    void slotAboutQuit();


private:

    void setMarketModel();
    void setTickerModel();

    Ui::ConfigWindow *ui;
};

#endif // CONFIGWINDOW_H
