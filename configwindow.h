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
    bool bIsAboutChanged;

private:

    void ClearWidgetsValues();
    bool event(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

public:
    explicit ConfigWindow(QWidget *parent = nullptr);
    ~ConfigWindow();

    void setMarketModel(MarketsListModel *model);

public:

signals:

    void SendToMainLog(QString);
    void NeedSaveMarketsChanges();

protected slots:

    void slotSetSelectedMarket(const  QModelIndex& indx);
    void slotSetSelectedMarket(const  QModelIndex& indx,const  QModelIndex&) {slotSetSelectedMarket(indx);};

    void slotDataChanged(bool Changed=true);
    void slotDataChanged(int)               {slotDataChanged(true);};
    void slotDataChanged(const QString &)   {slotDataChanged(true);};
    void slotTimeChanged(const QTime &)     {slotDataChanged(true);};

    void slotAboutQuit();

public slots:

    void slotBtnAddClicked();
    void slotBtnRemoveClicked();
    void slotBtnSaveClicked();
    void slotBtnCancelClicked();

private:
    Ui::ConfigWindow *ui;
};

#endif // CONFIGWINDOW_H
