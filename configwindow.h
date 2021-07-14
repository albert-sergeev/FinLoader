#ifndef CONFIGWINDOW_H
#define CONFIGWINDOW_H

//TODO: shrink includes
#include <QWidget>
#include <QTime>

#include "marketslistmodel.h"
#include "storage.h"

namespace Ui {
class ConfigWindow;
}

class ConfigWindow : public QWidget
{
    Q_OBJECT

    //////////////////////////////////////////
    /// common work part
public:

    explicit ConfigWindow(QWidget *parent = nullptr);
    ~ConfigWindow();
private:
    bool event(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

    //////////////////////////////////////////
    /// Market work part
    //////////////////////////////////////////
private:
    MarketsListModel *modelMarket;
    Storage stStore;
    bool bDataChanged;
    bool bAddingRow;
    bool bIsAboutChanged;

    void ClearWidgetsValues();


public:
    void setMarketModel(MarketsListModel *model);
signals:
    void SendToMainLog(QString);
    void NeedSaveMarketsChanges();
public slots:
    void slotBtnAddClicked();
    void slotBtnRemoveClicked();
    void slotBtnSaveClicked();
    void slotBtnCancelClicked();
protected slots:
    void slotSetSelectedMarket(const  QModelIndex& indx);
    void slotSetSelectedMarket(const  QModelIndex& indx,const  QModelIndex&) {slotSetSelectedMarket(indx);};
    void slotDataChanged(bool Changed=true);
    void slotDataChanged(int)               {slotDataChanged(true);};
    void slotDataChanged(const QString &)   {slotDataChanged(true);};
    void slotTimeChanged(const QTime &)     {slotDataChanged(true);};
    void slotAboutQuit();

    //////////////////////////////////////////
    /// Ticket work part
    //////////////////////////////////////////
private:
//    MarketsListModel *modelMarket;
//    Storage stStore;
//    bool bDataChanged;
//    bool bAddingRow;
//    bool bIsAboutChanged;

//    void ClearWidgetsValues();

public:
    //void setMarketModel(MarketsListModel *model);
signals:
//    void SendToMainLog(QString);
//    void NeedSaveMarketsChanges();
public slots:
//    void slotBtnAddClicked();
//    void slotBtnRemoveClicked();
//    void slotBtnSaveClicked();
//    void slotBtnCancelClicked();
protected slots:
//    void slotSetSelectedMarket(const  QModelIndex& indx);
//    void slotSetSelectedMarket(const  QModelIndex& indx,const  QModelIndex&) {slotSetSelectedMarket(indx);};
//    void slotDataChanged(bool Changed=true);
//    void slotDataChanged(int)               {slotDataChanged(true);};
//    void slotDataChanged(const QString &)   {slotDataChanged(true);};
//    void slotTimeChanged(const QTime &)     {slotDataChanged(true);};
//    void slotAboutQuit();


private:
    Ui::ConfigWindow *ui;
};

#endif // CONFIGWINDOW_H
