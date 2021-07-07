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

private:
    MarketsListModel *modelMarket;
    Storage stStore;
    bool bDataChanged;
    bool bAddingRow;

private:

    void ClearWidgetsValues();
    bool event(QEvent *event) override;

public:
    explicit ConfigWindow(QWidget *parent = nullptr);
    ~ConfigWindow();

    void setMarketModel(MarketsListModel *model);

public:

signals:

    void SendToMainLog(QString);
    void NeedSaveMarketsChanges();

public slots:

    void slotSetSelectedMarket(const  QModelIndex& indx);

    void slotBtnAddClicked();
    void slotBtnRemoveClicked();
    void slotBtnSaveClicked();
    void slotBtnCancelClicked();

    void slotDataChanged(bool Changed=true);
    void slotDataChanged(int)               {slotDataChanged(true);};
    void slotTimeChanged(const QTime &)     {slotDataChanged(true);};

    void slotAboutQuit();



private:
    Ui::ConfigWindow *ui;
};

#endif // CONFIGWINDOW_H
