#ifndef CONFIGWINDOW_H
#define CONFIGWINDOW_H

#include <QWidget>
#include "marketslistmodel.h"

namespace Ui {
class ConfigWindow;
}

class ConfigWindow : public QWidget
{
    Q_OBJECT

private:
    MarketsListModel *modelMarket;

public:
    explicit ConfigWindow(QWidget *parent = nullptr);
    ~ConfigWindow();

    void setMarketModel(MarketsListModel *model);

public slots:

    void slotSetSelectedMarket(const  QModelIndex& indx);



private:
    Ui::ConfigWindow *ui;
};

#endif // CONFIGWINDOW_H
