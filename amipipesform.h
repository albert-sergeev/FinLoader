/****************************************************************************
*  This is part of FinLoader
*  Copyright (C) 2021  Albert Sergeyev
*  Contact: albert.s.sergeev@mail.ru
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <https://www.gnu.org/licenses/>.
****************************************************************************/

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
    dataAmiPipeTask::pipes_type mFreePipesAsked;

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

    void AskPipesNames(dataAmiPipeTask::pipes_type &pipesFree);

public slots:
    void slotInternalPanelsStateChanged(bool bLeft, bool bRight);
    void slotPipeNameReceived(std::string,std::string);

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
    void slotBindAllClicked();

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
