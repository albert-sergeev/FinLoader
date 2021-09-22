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
#include "transparentbutton.h"


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

    TransparentButton *trbtnLeft;
    TransparentButton *trbtnRight;

    int iStoredWidthLeft{0};
    int iStoredWidthRight{0};
    int iStoredMousePos{0};

    bool bInResizingLeftLine;    
    bool bCursorOverrided;

    int iStoredWidthNew{0};
    int iStoredWidthActive{0};

public:
    explicit AmiPipesForm(modelMarketsList *modelM, int DefaultTickerMarket,
                          modelTickersList *modelT,
                          AmiPipeHolder & p,
                          std::vector<Ticker> &v,
                          bool ShowByNameUnallocated,
                          bool ShowByNameActive,
                          bool ShowByNameOff,
                          bool bAmiPipesNewWndShown,
                          bool bAmiPipesActiveWndShown,
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

    void WidthWasChanged(int);

    void NewWndStateChanged(int);
    void ActiveWndStateChanged(int);

    void buttonHideClicked();

public slots:
    void slotInternalPanelsStateChanged(bool bLeft, bool bRight);

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
    void RepositionTransparentButtons();

    void slotTransparentBtnLeftStateChanged(int);
    void slotTransparentBtnRightStateChanged(int);

    int CalculatedMimimum();

private:
    Ui::AmiPipesForm *ui;

    // QWidget interface
protected:
    void showEvent(QShowEvent *event);
    void resizeEvent(QResizeEvent *event);
    void closeEvent(QCloseEvent *event);

    bool eventFilter(QObject *watched, QEvent *event);
};

#endif // AMIPIPERFORM_H
