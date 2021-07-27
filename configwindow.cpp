#include "configwindow.h"
#include "ui_configwindow.h"

#include<QMessageBox>
#include<QDebug>
#include <QKeyEvent>
#include <iostream>


//--------------------------------------------------------------------------------------------------------
ConfigWindow::ConfigWindow(QWidget *parent) :
    QWidget(parent)
    , modelMarket{nullptr}
    , bDataMarketChanged{false}
    , bAddingMarketRow{false}
    , bIsAboutMarkerChanged(false)
    , ui(new Ui::ConfigWindow)
{
    ui->setupUi(this);
    try{
        stStore.Initialize();
    }
    catch (std::exception &e){
        //
        int n=QMessageBox::critical(0,tr("Error during initialising data dir!"),e.what());
        if (n==QMessageBox::Ok){;}
        //
    }

    ///////////////////////////////////////////////////////////////////////
    // market-tab work

    //
    connect(ui->btnAddMaket,SIGNAL(clicked()),  this,SLOT(slotBtnAddMarketClicked()));
    connect(ui->btnDelMarket,SIGNAL(clicked()), this,SLOT(slotBtnRemoveMarketClicked()));
    connect(ui->btnSaveMarket,SIGNAL(clicked()),this,SLOT(slotBtnSaveMarketClicked()));
    connect(ui->btnCancel,SIGNAL(clicked()),this,SLOT(slotBtnCancelMarketClicked()));
    //


    connect(ui->edName,SIGNAL(textChanged(const QString &)),this,SLOT(slotMarketDataChanged(const QString &)));
    connect(ui->edSign,SIGNAL(textChanged(const QString &)),this,SLOT(slotMarketDataChanged(const QString &)));

    connect(ui->chkAutoLoad,SIGNAL(stateChanged(int)),this,SLOT(slotMarketDataChanged(int)));
    connect(ui->chkUpToSys,SIGNAL(stateChanged(int)),this,SLOT(slotMarketDataChanged(int)));
    connect(ui->dateTimeStart,SIGNAL(timeChanged(const QTime &)),this,SLOT(slotMarketTimeChanged(const QTime &)));
    connect(ui->dateTimeEnd,SIGNAL(timeChanged(const QTime &)),this,SLOT(slotMarketTimeChanged(const QTime &)));

    ///////////////////////////////////////////////////////////////////////
    // ticker-tab work

    //
    connect(ui->btnAddTicker,SIGNAL(clicked()),  this,SLOT(slotBtnAddTickerClicked()));
    connect(ui->btnRemoveTicker,SIGNAL(clicked()), this,SLOT(slotBtnRemoveTickerClicked()));
    connect(ui->btnSaveTicker,SIGNAL(clicked()),this,SLOT(slotBtnSaveTickerClicked()));
    connect(ui->btnCancelTicker,SIGNAL(clicked()),this,SLOT(slotBtnCancelTickerClicked()));
    //


    connect(ui->edTickerName,SIGNAL(textChanged(const QString &)),this,SLOT(slotTickerDataChanged(const QString &)));
    connect(ui->edTickerSign,SIGNAL(textChanged(const QString &)),this,SLOT(slotTickerDataChanged(const QString &)));
    connect(ui->edTickerSignFinam,SIGNAL(textChanged(const QString &)),this,SLOT(slotTickerDataChanged(const QString &)));
    connect(ui->edTickerSignQuik,SIGNAL(textChanged(const QString &)),this,SLOT(slotTickerDataChanged(const QString &)));

    connect(ui->chkAutoLoadTicker,SIGNAL(stateChanged(int)),this,SLOT(slotTickerDataChanged(int)));
    connect(ui->chkUpToSysTicker,SIGNAL(stateChanged(int)),this,SLOT(slotTickerDataChanged(int)));

    connect(ui->chkShowByName,SIGNAL(stateChanged(int)),this,SLOT(slotShowByNamesChecked(int)));
    connect(ui->chkSortByName,SIGNAL(stateChanged(int)),this,SLOT(slotSortByNamesChecked(int)));

    ui->listViewTicker->setFocus();
}
//--------------------------------------------------------------------------------------------------------
ConfigWindow::~ConfigWindow()
{
    delete ui;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///common work
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//--------------------------------------------------------------------------------------------------------
// process quit event - used for save unsaved changes
bool ConfigWindow::event(QEvent *event)
{
    if(event->type() == QEvent::Close){
        slotAboutQuit();
    }
    return QWidget::event(event);
}
//--------------------------------------------------------------------------------------------------------
// process ESC key event
void ConfigWindow::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Escape){
        slotBtnCancelMarketClicked();
        slotBtnSaveTickerClicked();
    }
}
//--------------------------------------------------------------------------------------------------------
// processer for quit event (see uper)
void ConfigWindow::slotAboutQuit()
{
    if(bDataMarketChanged){
        slotBtnSaveMarketClicked();
        slotBtnSaveTickerClicked();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///market-tab work
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//--------------------------------------------------------------------------------------------------------
///
/// \brief Process event when Market data changes: set enternal marker and set widgets enabled/disabled
/// \param Changed
///
void ConfigWindow::slotMarketDataChanged(bool Changed )
{
    if (!bIsAboutMarkerChanged){
        bDataMarketChanged = Changed;

        ui->btnAddMaket->setEnabled(!bDataMarketChanged);
        //ui->btnDelMarket->setEnabled(bDataChanged);
        ui->btnSaveMarket->setEnabled(bDataMarketChanged);
        ui->btnCancel->setEnabled(bDataMarketChanged);

        ui->listViewMarket->setEnabled(!bDataMarketChanged);
    }    
}
//--------------------------------------------------------------------------------------------------------
///
/// \brief Initiate process adding new market (need save or cancel later)
///
void ConfigWindow::slotBtnAddMarketClicked()
{
    bAddingMarketRow = true;

    Market m{"",""};
    int i = modelMarket->AddRow(m);

    QItemSelectionModel  *qml =ui->listViewMarket->selectionModel();

    auto indx(modelMarket->index(i,0));
    if(indx.isValid() && qml){
        qml->select(indx,QItemSelectionModel::SelectionFlag::ClearAndSelect) ;
        slotSetSelectedMarket(indx);
    }
    //
    slotMarketDataChanged(true);
    ui->edName->setFocus();
};
//--------------------------------------------------------------------------------------------------------
///
/// \brief Removing market (can be new or existing)
///
void ConfigWindow::slotBtnRemoveMarketClicked()
{
    int n=QMessageBox::warning(0,tr("Warning"),
                               tr("Do you want to remove market?"),
                               QMessageBox::Yes|QMessageBox::Cancel
                               );
    if (n==QMessageBox::Yes){
        auto qml(ui->listViewMarket->selectionModel());
        auto lst (qml->selectedIndexes());

// TODO: add check ticker data presents;

        if(lst.count() > 0){
            if(lst.count() > 1){
                qml->select(lst[0],QItemSelectionModel::SelectionFlag::ClearAndSelect) ;
            }
            //modelMarket->removeItem(lst[0].row());
            modelMarket->removeRow(lst[0].row());
            qml->select(lst[0],QItemSelectionModel::SelectionFlag::Clear);
            NeedSaveMarketsChanges();
            ClearMarketWidgetsValues();
            slotMarketDataChanged(false);

        }
    }
};

//--------------------------------------------------------------------------------------------------------
///
/// \brief Clear Market widget values. Used in other procedures
///
void ConfigWindow::ClearMarketWidgetsValues()
{
    ui->edName->setText("");
    ui->edSign->setText("");

    ui->chkAutoLoad->setCheckState  (Qt::CheckState::Unchecked);
    ui->chkUpToSys->setCheckState   (Qt::CheckState::Unchecked);

    const QTime tmS(0,0,0);
    const QTime tmE(0,0,0);

    ui->dateTimeStart->setTime(tmS);
    ui->dateTimeEnd->setTime(tmE);
}

//--------------------------------------------------------------------------------------------------------
///
/// \brief Main save market processer. Save new and existing markets. Can be used on exit form.
///
void ConfigWindow::slotBtnSaveMarketClicked()
{
    if(bDataMarketChanged){
        int n=QMessageBox::warning(0,tr("Warning"),
                               tr("Do you want to save changes?"),
                               QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel
                               );
        if (n==QMessageBox::Yes){

            auto qml(ui->listViewMarket->selectionModel());
            auto lst (qml->selectedIndexes());

            if(lst.count() > 0){
                  if(lst.count() > 1){
                      qml->select(lst[0],QItemSelectionModel::SelectionFlag::ClearAndSelect) ;
                  }

                  Market& m=modelMarket->getMarket(lst[0]);

                  m.SetMarketName(ui->edName->text().toStdString());
                  m.SetMarketSign(ui->edSign->text().toStdString());
                  m.SetAutoLoad(ui->chkAutoLoad->isChecked()? true:false);
                  m.SetUpToSys(ui->chkUpToSys->isChecked()? true:false);
                  //
                  const QTime tmS(ui->dateTimeStart->time());
                  std::tm tmSt;
                  {
                      tmSt.tm_year   = 2000 - 1900;
                      tmSt.tm_mon    = 1;
                      tmSt.tm_mday   = 1;
                      tmSt.tm_hour   = tmS.hour();
                      tmSt.tm_min    = tmS.minute();
                      tmSt.tm_sec    = tmS.second();
                      tmSt.tm_isdst  = 0;
                  }
                  std::time_t tS (std::mktime(&tmSt));
                  m.SetStartTime(tS);
                  //
                  const QTime tmE(ui->dateTimeEnd->time());
                  {
                      tmSt.tm_hour   = tmE.hour();
                      tmSt.tm_min    = tmE.minute();
                      tmSt.tm_sec    = tmE.second();
                  }
                  std::time_t tE (std::mktime(&tmSt));
                  m.SetEndTime(tE);
                  ///
                  NeedSaveMarketsChanges();
                  modelMarket->dataChanged(lst[0],lst[0]);

                  bAddingMarketRow = false;
                  slotMarketDataChanged(false);

                  //qDebug() << "choiceSave";
            }
        }
        else if (n==QMessageBox::Cancel){
            slotBtnCancelMarketClicked();
            bAddingMarketRow = false;
            //qDebug() << "choiceCancel";
        }
        else{
            //qDebug() << "choiceNo";
        }
    }
    else{
        bAddingMarketRow = false;
        slotMarketDataChanged(false);
        //qDebug() << "choiceNoChanges";
    }

};
//--------------------------------------------------------------------------------------------------------
///
/// \brief Undo changes or adding new Market
///
void ConfigWindow::slotBtnCancelMarketClicked()
{
    auto qml(ui->listViewMarket->selectionModel());
    auto lst (qml->selectedIndexes());

    if (!bAddingMarketRow){

        if(lst.count() > 0){
            if(lst.count() > 1){
                qml->select(lst[0],QItemSelectionModel::SelectionFlag::Select) ;
            }
            if(lst[0].isValid()){
                slotSetSelectedMarket(lst[0]);
            }
        }
    }
    else{
        for(const auto & r:lst)
        {
            modelMarket->removeRow(r.row());
            ClearMarketWidgetsValues();
            slotMarketDataChanged(false);

            qml->select(r,QItemSelectionModel::SelectionFlag::Clear) ;

            // sel first
            QItemSelectionModel  *qml =new QItemSelectionModel(modelMarket);
            auto first_i(modelMarket->index(0,0));
            if(first_i.isValid()){
                qml->select(first_i,QItemSelectionModel::SelectionFlag::Select) ;
                slotSetSelectedMarket(first_i);
            }

        }
    }

    bAddingMarketRow=false;
};
//--------------------------------------------------------------------------------------------------------
///
/// \brief Initialazing procedure. Set data for form. Mast be set during creating form (witn setTickerModel).
/// \param model
/// \param DefaultTickerMarket
///
void ConfigWindow::setMarketModel(MarketsListModel *model, int DefaultTickerMarket)
{
    modelMarket = model;
    ui->listViewMarket->setModel(model);


    ////////////////////////////////////////////////////
    {
        connect(ui->listViewMarket,SIGNAL(clicked(const QModelIndex&)),this,SLOT(slotSetSelectedMarket(const  QModelIndex&)));

        QItemSelectionModel  *qml =new QItemSelectionModel(modelMarket);
        ui->listViewMarket->setSelectionModel(qml);
        connect(qml,SIGNAL(currentRowChanged(const QModelIndex&,const QModelIndex&)),this,SLOT(slotSetSelectedMarket(const  QModelIndex&,const QModelIndex&)));
        auto first_i(modelMarket->index(0,0));
        if(first_i.isValid()){
            qml->select(first_i,QItemSelectionModel::SelectionFlag::Select) ;
            slotSetSelectedMarket(first_i);
        }
    }
    ////////////////////////////////////////////////////
    ui->cmbMarkets->setModel(model);

    {

        connect(ui->cmbMarkets,SIGNAL(activated(const int)),this,SLOT(slotSetSelectedTickersMarket(const  int)));

        bool bWas{false};
        for(int i = 0; i < modelMarket->rowCount(); i++){
            auto idx(modelMarket->index(i,0));
            if(idx.isValid()){
                if(modelMarket->getMarket(idx).MarketID() == DefaultTickerMarket){
                    //iDefaultTickerMarket = DefaultTickerMarket;
                    ui->cmbMarkets->setCurrentIndex(i);
                    slotSetSelectedTickersMarket(i);
                    bWas = true;
                    break;
                }
            }
        }
        if (!bWas){
            auto idx(modelMarket->index(0,0));
            if(idx.isValid()){
                iDefaultTickerMarket = modelMarket->getMarket(idx).MarketID();
                slotSetSelectedTickersMarket(0);
            }
        }
    }
    ////////////////////////////////////////////////////

}
//--------------------------------------------------------------------------------------------------------
///
/// \brief Process event user select Market. Fill widgets with data.
/// \param indx
///
void ConfigWindow::slotSetSelectedMarket(const  QModelIndex& indx)
{


    //qDebug()<<"Enter slotSetSelectedMarket";

    if (indx.isValid()){
        const Market& m=modelMarket->getMarket(indx);

        bIsAboutMarkerChanged=true;

        ui->edName->setText(QString::fromStdString(m.MarketName()));
        ui->edSign->setText(QString::fromStdString(m.MarketSign()));


        ui->chkAutoLoad->setCheckState  (m.AutoLoad() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
        ui->chkUpToSys->setCheckState   (m.UpToSys()  ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);



        std::time_t tS (m.StartTime());
        std::tm* tmSt=localtime(&tS);
        const QTime tmS(tmSt->tm_hour,tmSt->tm_min,0);
        std::time_t tE (m.EndTime());
        std::tm* tmEn=localtime(&tE);
        const QTime tmE(tmEn->tm_hour,tmEn->tm_min,0);


        ui->dateTimeStart->setTime(tmS);
        ui->dateTimeEnd->setTime(tmE);

        bIsAboutMarkerChanged=false;

        slotMarketDataChanged(false);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///ticker-tab work
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------------------------------------------
///
/// \brief Initialazing procedure. Set data for form. Mast be set during creating form (witn setMarketModel).
/// \param model
/// \param ShowByName
/// \param SortByName
///
void ConfigWindow::setTickerModel(TickersListModel *model,bool ShowByName,bool SortByName)
{

    modelTicker = model;
    proxyTickerModel.setDefaultMarket(iDefaultTickerMarket);
    proxyTickerModel.setSourceModel(model);
    ui->listViewTicker->setModel(&proxyTickerModel);

    ui->chkShowByName->setChecked(ShowByName);
    ui->chkSortByName->setChecked(SortByName);
    slotShowByNamesChecked(ShowByName);
    slotSortByNamesChecked(SortByName);


    connect(ui->listViewTicker,SIGNAL(clicked(const QModelIndex&)),this,SLOT(slotSetSelectedTicker(const  QModelIndex&)));

    /////////

    QItemSelectionModel  *qml =new QItemSelectionModel(&proxyTickerModel);
    //QItemSelectionModel  *qml =new QItemSelectionModel(model);
    ui->listViewTicker->setSelectionModel(qml);


    connect(qml,SIGNAL(currentRowChanged(const QModelIndex&,const QModelIndex&)),this,SLOT(slotSetSelectedTicker(const  QModelIndex&,const QModelIndex&)));


    auto first_i(proxyTickerModel.index(0,0));
    //auto first_i(model->index(0,0));
    if(first_i.isValid()){
        qml->select(first_i,QItemSelectionModel::SelectionFlag::Select) ;
        slotSetSelectedTicker(first_i);
    }
    else{
        slotTickerDataChanged(false);
        //
        setEnableTickerWidgets(false);
    }
    ////
}
//--------------------------------------------------------------------------------------------------------
///
/// \brief Process event user select Ticker. Fill widgets with data.
/// \param indx
///
void ConfigWindow::slotSetSelectedTicker(const  QModelIndex& indx)
{


    //qDebug()<<"Enter slotSetSelectedMarket";

    if (indx.isValid()){
        //const Ticker& t=modelTicker->getTicker(indx);
        const Ticker& t=proxyTickerModel.getTicker(indx);


        bIsAboutTickerChanged=true;

        ui->edTickerName->setText(QString::fromStdString(t.TickerName()));
        ui->edTickerSign->setText(QString::fromStdString(t.TickerSign()));
        ui->edTickerSignFinam->setText(QString::fromStdString(t.TickerSignFinam()));
        ui->edTickerSignQuik->setText(QString::fromStdString(t.TickerSignQuik()));


        ui->chkAutoLoadTicker->setCheckState  (t.AutoLoad() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
        ui->chkUpToSysTicker->setCheckState   (t.UpToSys()  ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);

        bIsAboutTickerChanged=false;

        //slotTickerDataChanged(false);

        setEnableTickerWidgets(true);
    }
    else{
        setEnableTickerWidgets(false);
    }
}
//--------------------------------------------------------------------------------------------------------
///
/// \brief Process event when Ticker data changes: set enternal marker and set widgets enabled/disabled
/// \param Changed
///
void ConfigWindow::slotTickerDataChanged(bool Changed )
{
    if (!bIsAboutTickerChanged){
        bDataTickerChanged = Changed;

        ui->btnAddTicker->setEnabled(!bDataTickerChanged);
        //ui->btnDelTicker->setEnabled(bDataTickerChanged);
        ui->btnSaveTicker->setEnabled(bDataTickerChanged);
        ui->btnCancelTicker->setEnabled(bDataTickerChanged);

        ui->listViewTicker->setEnabled(!bDataTickerChanged);
    }
}
//--------------------------------------------------------------------------------------------------------
///
/// \brief Process event user select Market for Ticker listview. Set filter for listview and save choise.
/// \param i
///
void ConfigWindow::slotSetSelectedTickersMarket(const  int i)
{
    //qDebug()<<"i: {"<<i<<"}";
    if(i < modelMarket->rowCount()){
        auto idx(modelMarket->index(i,0));
        if(idx.isValid()){
            if(modelMarket->getMarket(idx).MarketID() != iDefaultTickerMarket){
                iDefaultTickerMarket = modelMarket->getMarket(idx).MarketID();
                NeedSaveDefaultTickerMarket(iDefaultTickerMarket);
                proxyTickerModel.setDefaultMarket(iDefaultTickerMarket);
                // clear
                ClearTickerWidgetsValues();
                setEnableTickerWidgets(true);
                slotTickerDataChanged(false);
                // sel first item
                //QItemSelectionModel  *qml =new QItemSelectionModel(&proxyTickerModel);
                QItemSelectionModel  *qml = ui->listViewTicker->selectionModel();
                auto first_i(proxyTickerModel.index(0,0));
                if(first_i.isValid()){
                    qml->select(first_i,QItemSelectionModel::SelectionFlag::Select | QItemSelectionModel::Rows) ;
                    slotSetSelectedTicker(first_i);
                    ui->listViewTicker->setFocus();
                }

            }
        }
    }
}

//--------------------------------------------------------------------------------------------------------
///
/// \brief Begin process adding ticker
///
void ConfigWindow::slotBtnAddTickerClicked()
{
    bAddingTickerRow = true;
    //
    ClearTickerWidgetsValues();
    setEnableTickerWidgets(true);
    slotTickerDataChanged(true);

    ui->chkAutoLoadTicker->setChecked(true);
    ui->edTickerName->setFocus();
}
//--------------------------------------------------------------------------------------------------------
///
/// \brief Process event removing Ticker. Can be new or exists
///
void ConfigWindow::slotBtnRemoveTickerClicked()
{
    auto qml(ui->listViewTicker->selectionModel());
    auto lst (qml->selectedIndexes());
    QString qS;
    if(lst.count()>0){
          qS.fromStdString(proxyTickerModel.getTicker(lst[0]).TickerName());
    }

    int n=QMessageBox::warning(0,tr("Warning"),
                               tr("Do you want to remove ticker [") + qS +"] ?"
                               ,
                               QMessageBox::Yes|QMessageBox::Cancel
                               );
    if (n==QMessageBox::Yes){
        if(!bAddingTickerRow){


            // TODO: add check ticker data presents;

            for(auto el:lst){

                proxyTickerModel.removeRow(el.row());
                //modelTicker->removeRow(el.row());
            }
            ClearTickerWidgetsValues();
            slotTickerDataChanged(false);
            setEnableTickerWidgets(false);
        }
        else{
            slotBtnCancelTickerClicked();
        }
    }
};
//--------------------------------------------------------------------------------------------------------
///
/// \brief Main procedure for saving ticker changes. Can be new or existing. Can be used on exit form.
///
void ConfigWindow::slotBtnSaveTickerClicked(){

    if(bDataTickerChanged){
        int n=QMessageBox::warning(0,tr("Warning"),
                               tr("Do you want to save ticker changes?"),
                               QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel
                               );
        if (n==QMessageBox::Yes){
            if(ui->edTickerName->text().toStdString().size() <=0 ){
                    QMessageBox::warning(0,tr("Warning"),
                                               tr("Set ticker name!"),
                                               QMessageBox::Ok
                                               );
                    return;
            }

            if(bAddingTickerRow){
                Ticker t {  ui->edTickerName->text().toStdString(),
                            ui->edTickerSign->text().toStdString(),
                            iDefaultTickerMarket};
                t.SetTickerSignFinam(ui->edTickerSignFinam->text().toStdString());
                t.SetTickerSignQuik(ui->edTickerSignQuik->text().toStdString());
                t.SetAutoLoad(ui->chkAutoLoadTicker->isChecked()? true:false);
                t.SetUpToSys(ui->chkUpToSysTicker->isChecked()? true:false);

                //int i = modelTicker->AddRow(t);
                int i = proxyTickerModel.AddRow(t);


                bAddingTickerRow = false;


                QItemSelectionModel  *qml =ui->listViewTicker->selectionModel();

                //auto indx(modelTicker->index(i,0));
                auto indx(proxyTickerModel.index(i,0));
                if(indx.isValid() && qml){
                    qml->select(indx,QItemSelectionModel::SelectionFlag::ClearAndSelect | QItemSelectionModel::Rows) ;
                    slotSetSelectedTicker(indx);
                    }


                slotTickerDataChanged(false);
            }
            else{
                auto qml(ui->listViewTicker->selectionModel());
                auto lst (qml->selectedIndexes());
                if(lst.count() > 0){
                      if(lst.count() > 1){
                          qml->select(lst[0],QItemSelectionModel::SelectionFlag::ClearAndSelect | QItemSelectionModel::Rows) ;
                      }

                      //Ticker& t=modelTicker->getTicker(lst[0]);
                      Ticker t=proxyTickerModel.getTicker(lst[0]);

                      t.SetTickerName(ui->edTickerName->text().toStdString());
                      t.SetTickerSign(ui->edTickerSign->text().toStdString());
                      t.SetTickerSignFinam(ui->edTickerSignFinam->text().toStdString());
                      t.SetTickerSignQuik(ui->edTickerSignQuik->text().toStdString());
                      t.SetAutoLoad(ui->chkAutoLoadTicker->isChecked()? true:false);
                      t.SetUpToSys(ui->chkUpToSysTicker->isChecked()? true:false);
                      ///
                      proxyTickerModel.setData(lst[0],t,Qt::EditRole);
                }
                slotTickerDataChanged(false);
            }
        }
        else if (n==QMessageBox::Cancel){
            slotBtnCancelTickerClicked();
        }
        else{
            ;
        }
    }
    else{
        slotBtnCancelTickerClicked();
    }
};
//--------------------------------------------------------------------------------------------------------
///
/// \brief Undo ticker changes. Ticker can be new or existing
///
void ConfigWindow::slotBtnCancelTickerClicked()
{
    auto qml(ui->listViewTicker->selectionModel());
    auto lst (qml->selectedIndexes());

    if (!bAddingTickerRow){

        if(lst.count() > 0){
            if(lst.count() > 1){
                qml->select(lst[0],QItemSelectionModel::SelectionFlag::Select | QItemSelectionModel::Rows) ;
            }
            if(lst[0].isValid()){
                slotSetSelectedTicker(lst[0]);
                setEnableTickerWidgets(true);
                slotTickerDataChanged(false);
            }
            else{
                ClearTickerWidgetsValues();
                setEnableTickerWidgets(false);
                slotTickerDataChanged(false);
            }
        }
        else{
            ClearTickerWidgetsValues();
            setEnableTickerWidgets(false);
            slotTickerDataChanged(false);
        }
    }
    else{

        ClearTickerWidgetsValues();
        slotTickerDataChanged(false);
        setEnableTickerWidgets(false);
        bAddingTickerRow=false;

        auto qml(ui->listViewTicker->selectionModel());
        auto lst (qml->selectedIndexes());
        if(lst.count() > 0){
              if(lst.count() > 1){
                  qml->select(lst[0],QItemSelectionModel::SelectionFlag::ClearAndSelect) ;
              }
              slotSetSelectedTicker(lst[0]);
        }
    }


};
//--------------------------------------------------------------------------------------------------------
///
/// \brief Process event changing ticker listview show type
/// \param Checked
///
void ConfigWindow::slotShowByNamesChecked(int Checked)
{
    if (Checked){
        ui->listViewTicker->setModelColumn(0);
    }
    else{
        ui->listViewTicker->setModelColumn(2);
    }
    proxyTickerModel.invalidate();
    NeedSaveShowByNames(Checked);
}
//--------------------------------------------------------------------------------------------------------
///
/// \brief Process event changing ticker listview sort type
/// \param Checked
///
void ConfigWindow::slotSortByNamesChecked(int Checked)
{
    if (Checked){

        proxyTickerModel.sort(0);
    }
    else{
        proxyTickerModel.sort(2);
    }
    proxyTickerModel.invalidate();
    NeedSaveSortByNames(Checked);
}
//--------------------------------------------------------------------------------------------------------
///
/// \brief Set ticker widgets enable/disable. Used in other procedures
/// \param bEnable
///
void ConfigWindow::setEnableTickerWidgets(bool bEnable)
{
    ui->edTickerName->setEnabled(bEnable);
    ui->edTickerSign->setEnabled(bEnable);
    ui->edTickerSignFinam->setEnabled(bEnable);
    ui->edTickerSignQuik->setEnabled(bEnable);
    ui->chkAutoLoadTicker->setEnabled(bEnable);
    ui->chkUpToSysTicker->setEnabled(bEnable);
}

//--------------------------------------------------------------------------------------------------------
///
/// \brief Clear ticker widgets data. Used in other procedures
///
void ConfigWindow::ClearTickerWidgetsValues()
{

    ui->edTickerName->setText("");
    ui->edTickerSign->setText("");
    ui->edTickerSignFinam->setText("");
    ui->edTickerSignQuik->setText("");

    ui->chkAutoLoadTicker->setCheckState  (Qt::CheckState::Unchecked);
    ui->chkUpToSysTicker->setCheckState   (Qt::CheckState::Unchecked);

}

//--------------------------------------------------------------------------------------------------------
