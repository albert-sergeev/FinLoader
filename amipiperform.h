#ifndef AMIPIPERFORM_H
#define AMIPIPERFORM_H

#include <QWidget>
#include <QMenu>
#include <QStringListModel>
#include "modelmarketslist.h"
#include "modeltickerslist.h"
#include "amipipeholder.h"

namespace Ui {
class AmiPiperForm;
}

class AmiPiperForm : public QWidget
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
    AmiPipeHolder::pipes_type mFreePipes;

    AmiPipeHolder &pipes;

    std::vector<Ticker> &vTickersLst;

public:
    explicit AmiPiperForm(modelMarketsList *modelM, int DefaultTickerMarket,
                          modelTickersList *modelT,
                          AmiPipeHolder & p,
                          std::vector<Ticker> &v,
                          QWidget *parent = nullptr);
    ~AmiPiperForm();

public:
signals:
    void SendToMainLog(QString);
    void NeedSaveDefaultTickerMarket(int);

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

private:
    Ui::AmiPiperForm *ui;
};

#endif // AMIPIPERFORM_H
