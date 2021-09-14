#ifndef AMIPIPERFORM_H
#define AMIPIPERFORM_H

#include <QWidget>
#include <QMenu>
#include <QStringListModel>

#include<fstream>

#include "modelmarketslist.h"
#include "modeltickerslist.h"
#include "amipipeholder.h"

#include "styledswitcher.h"

namespace Ui {
class AmiPipesForm;
}

class AmiPipesForm : public QWidget
{
    Q_OBJECT

protected:

    int iDefaultTickerMarket;

    modelMarketsList * const modelMarket;
    modelTickersList * const modelTicker;
    TickerProxyListModel proxyTickerModelUnallocated;
    TickerProxyListModel proxyTickerModelActive;
    TickerProxyListModel proxyTickerModelOff;

    QStringListModel    *modelNew;
    dataAmiPipeTask::pipes_type mFreePipes;

    AmiPipeHolder &pipes;

    std::vector<Ticker> &vTickersLst;

    StyledSwitcher *swtShowByNameUnallocated;
    StyledSwitcher *swtShowByNameActive;
    StyledSwitcher *swtShowByNameOff;

    bool bShowByNameUnallocated;
    bool bShowByNameActive;
    bool bShowByNameOff;

public:
    explicit AmiPipesForm(modelMarketsList *modelM, int DefaultTickerMarket,
                          modelTickersList *modelT,
                          AmiPipeHolder & p,
                          std::vector<Ticker> &v,
                          bool ShowByNameUnallocated,
                          bool ShowByNameActive,
                          bool ShowByNameOff,
                          QWidget *parent = nullptr);
    ~AmiPipesForm();

public:
signals:
    void SendToMainLog(QString);
    void NeedSaveDefaultTickerMarket(int);
    void WasCloseEvent();

    void NeedSaveShowByNamesUnallocated(bool);
    void NeedSaveShowByNamesActive(bool);
    void NeedSaveShowByNamesOff(bool);


protected:
    void SetMarketModel();
    void SetTickerModel();


protected slots:
    void slotBtnCheckClicked();
    void slotSetSelectedTickersMarket(int i);

    void slotSetSelectedTickerActive        (const  QModelIndex&,const QModelIndex&);
    void slotSetSelectedTickerOff           (const  QModelIndex&,const QModelIndex&);
    void slotSetSelectedTickerUnallocated   (const  QModelIndex&,const QModelIndex&);

    void slotSetSelectedTickerActive        (const QModelIndex&);
    void slotSetSelectedTickerOff           (const QModelIndex&);
    void slotSetSelectedTickerUnallocated   (const QModelIndex&);

    void slotOffAllClicked();
    void slotOffOneClicked();
    void slotOnAllClicked();
    void slotOnOneClicked();

    void slotActiveDissociateClicked();
    void slotOffDissociateClicked();

    void slotDoubleClickedActive(const  QModelIndex&);
    void slotDoubleClickedOff(const  QModelIndex&);
    void slotDoubleClickedUnallocated(const  QModelIndex&);
    void slotDoubleClickedNew(const  QModelIndex&);

    void slotActiveContextMenuRequested(const QPoint &);
    void slotOffContextMenuRequested(const QPoint &);
    void slotUnallocatedContextMenuRequested(const QPoint &);

    void slotSelectNewTicker(const  QModelIndex&);
    void slotBindClicked();

    void slotShowByNamesUnallocatedChecked(int Checked);
    void slotShowByNamesActiveChecked(int Checked);
    void slotShowByNamesOffChecked(int Checked);

    void slotBtnQuitClicked();

private:
    Ui::AmiPipesForm *ui;

    // QWidget interface
protected:
    void showEvent(QShowEvent *event);

    // QWidget interface
protected:
    void closeEvent(QCloseEvent *event);
};

#endif // AMIPIPERFORM_H
