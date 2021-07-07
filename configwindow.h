#ifndef CONFIGWINDOW_H
#define CONFIGWINDOW_H

//TODO: shrink includes
#include <QWidget>

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

public:
    explicit ConfigWindow(QWidget *parent = nullptr);
    ~ConfigWindow();

    void setMarketModel(MarketsListModel *model);

public:

signals:

    void SendToMainLog(QString);

public slots:

    void slotSetSelectedMarket(const  QModelIndex& indx);

    void slotStateChanged();
    void slotBtnAddClicked();
    void slotBtnRemoveClicked();
    void slotBtnSaveClicked();
    void slotBtnCancelClicked();



private:
    Ui::ConfigWindow *ui;
};

#endif // CONFIGWINDOW_H
