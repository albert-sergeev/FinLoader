#include "configwindow.h"
#include "ui_configwindow.h"

#include<QMessageBox>
#include<QStandardPaths>
#include<QDebug>
#include<QFileDialog>
#include <QKeyEvent>
#include <iostream>


//--------------------------------------------------------------------------------------------------------
ConfigWindow::ConfigWindow(modelMarketsList *modelM,int DefaultTickerMarket,
                           modelTickersList *modelT, bool ShowByName,bool SortByName,
                           bool DefStoragePath, QString StoragePath,
                           Storage &st,
                           QWidget *parent) :
    QWidget(parent)
    , stStore{st}
    , bDataMarketChanged{false}
    , bAddingMarketRow{false}
    , bIsAboutMarkerChanged(false)
    , bDataTickerChanged{false}
    , bAddingTickerRow {false}
    , bIsAboutTickerChanged{false}
    , bDataGeneralChanged{false}
    ,iDefaultTickerMarket{DefaultTickerMarket}
    , modelMarket{modelM}
    , modelTicker{modelT}
    , ui(new Ui::ConfigWindow)
{
    ui->setupUi(this);
    try{
        QString appDataFolder;
        if (DefStoragePath){
            //appDataFolder = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
            auto path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
            if (path.isEmpty()){
                throw std::runtime_error("Unable determine standart application data path");
            }
            QDir d{path};
            if (!d.mkpath(d.absolutePath())){
                throw std::runtime_error("Error creating standart application data path");
                }
            appDataFolder = d.absolutePath();
        }
        else{
            appDataFolder = StoragePath;
        }
        stStore.Initialize(appDataFolder.toStdString());
    }
    catch (std::exception &e){
        //
        int n=QMessageBox::critical(0,tr("Error during initialising data dir!"),e.what());
        if (n==QMessageBox::Ok){;}
        //
    }

    //-------------------------------------------------------------
    QColor colorDarkGreen(0, 100, 52,50);
    QColor colorDarkRed(31, 53, 200,40);
    //-------------------------------------------------------------
    // for ticker
    //-------------------------------------------------------------
    QHBoxLayout *lt1 = new QHBoxLayout();
    lt1->setMargin(0);
    ui->wtAutoLoadTicker->setLayout(lt1);
    swtAutoLoadTicker = new StyledSwitcher(tr("On "),tr(" Off"),true,10,this);
    lt1->addWidget(swtAutoLoadTicker);
    //-------------------------------------------------------------
    QHBoxLayout *lt2 = new QHBoxLayout();
    lt2->setMargin(0);
    ui->wtUpToSysTicker->setLayout(lt2);
    swtUpToSysTicker = new StyledSwitcher(tr("On "),tr(" Off"),true,10,this);
    lt2->addWidget(swtUpToSysTicker);
    //-------------------------------------------------------------
    QHBoxLayout *lt3 = new QHBoxLayout();
    lt3->setMargin(0);
    ui->wtBulblulatorTicker->setLayout(lt3);
    swtBulbululatorTicker = new StyledSwitcher(tr("On "),tr(" Off"),true,10,this);
    lt3->addWidget(swtBulbululatorTicker);
    //-------------------------------------------------------------
    QHBoxLayout *lt4 = new QHBoxLayout();
    lt4->setMargin(0);
    ui->wtShowByNameTicker->setLayout(lt4);
    swtShowByNameTicker = new StyledSwitcher(tr("Show by name"),tr("Show by ticker"),true,10,this);
    lt4->addWidget(swtShowByNameTicker);
    swtShowByNameTicker->SetOnColor(QPalette::Window,colorDarkGreen);
    swtShowByNameTicker->SetOffColor(QPalette::Window,colorDarkRed);
    //-------------------------------------------------------------
    QHBoxLayout *lt5 = new QHBoxLayout();
    lt5->setMargin(0);
    ui->wtSortByNameTicker->setLayout(lt5);
    swtSortByNameTicker = new StyledSwitcher(tr("Sort by name"),tr("Sort by ticker"),true,10,this);
    lt5->addWidget(swtSortByNameTicker);
    swtSortByNameTicker->SetOnColor(QPalette::Window,colorDarkGreen);
    swtSortByNameTicker->SetOffColor(QPalette::Window,colorDarkRed);
    //-------------------------------------------------------------
    //-------------------------------------------------------------
    // for market
    //-------------------------------------------------------------
    //-------------------------------------------------------------
    QHBoxLayout *lt6 = new QHBoxLayout();
    lt6->setMargin(0);
    ui->wtAutoLoadWholeMarket->setLayout(lt6);
    swtAutoLoadWholeMarket = new StyledSwitcher(tr("On "),tr(" Off"),true,10,this);
    lt6->addWidget(swtAutoLoadWholeMarket);
    //-------------------------------------------------------------
    QHBoxLayout *lt7 = new QHBoxLayout();
    lt7->setMargin(0);
    ui->wtUpToSysWholeMarket->setLayout(lt7);
    swtUpToSysWholeMarket = new StyledSwitcher(tr("On "),tr(" Off"),true,10,this);
    lt7->addWidget(swtUpToSysWholeMarket);
    //-------------------------------------------------------------
    //-------------------------------------------------------------
    // for general config
    //-------------------------------------------------------------
    //-------------------------------------------------------------
    QHBoxLayout *lt8 = new QHBoxLayout();
    lt8->setMargin(0);
    ui->wtDefPath->setLayout(lt8);
    swtDefPath = new StyledSwitcher(tr("On "),tr(" Off"),true,10,this);
    lt8->addWidget(swtDefPath);
    //-------------------------------------------------------------




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

    connect(swtAutoLoadWholeMarket,SIGNAL(stateChanged(int)),this,SLOT(slotMarketDataChanged(int)));
    connect(swtUpToSysWholeMarket,SIGNAL(stateChanged(int)),this,SLOT(slotMarketDataChanged(int)));
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

    connect(swtAutoLoadTicker,SIGNAL(stateChanged(int)),this,SLOT(slotTickerDataChanged(int)));
    connect(swtUpToSysTicker,SIGNAL(stateChanged(int)),this,SLOT(slotTickerDataChanged(int)));
    connect(swtBulbululatorTicker,SIGNAL(stateChanged(int)),this,SLOT(slotTickerDataChanged(int)));


    connect(swtShowByNameTicker,SIGNAL(stateChanged(int)),this,SLOT(slotShowByNamesChecked(int)));
    connect(swtSortByNameTicker,SIGNAL(stateChanged(int)),this,SLOT(slotSortByNamesChecked(int)));

    swtShowByNameTicker->setChecked(ShowByName);
    swtSortByNameTicker->setChecked(SortByName);

    ///////////////////////////////////////////////////////////////////////
    // general-tab work

    connect(swtDefPath,SIGNAL(stateChanged(int)),this,SLOT(slotDefPathChenged(int)));
    connect(ui->edStoragePath,SIGNAL(textChanged(const QString &)),this,SLOT(slotStoragePathChanged(const QString &)));

    connect(ui->btnGeneralSave,SIGNAL(clicked()),  this,SLOT(slotGeneralSaveClicked()));
    connect(ui->btnGeneralCancel,SIGNAL(clicked()),  this,SLOT(slotGeneralCancelClicked()));

    connect(ui->btnSelectStoragePath,SIGNAL(clicked()),  this,SLOT(slotGeneralOpenStorageDirClicked()));

    bDefStoragePath = DefStoragePath;
    qsStoragePath   = StoragePath;
    slotGeneralCancelClicked();

    ///////////////////////////////////////////////////////////////////////
    // common

    setMarketModel();
    setTickerModel();


    slotMarketDataChanged(false);
    slotTickerDataChanged(false);

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
        slotGeneralCancelClicked();
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
    if (bDataGeneralChanged){
        slotGeneralSaveClicked();
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
            emit NeedSaveMarketsChanges();
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


    swtAutoLoadWholeMarket->setCheckState  (Qt::CheckState::Unchecked);
    swtUpToSysWholeMarket->setCheckState   (Qt::CheckState::Unchecked);

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
                               tr("Do you want to save market changes?"),
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

                  m.SetMarketName   (trim(ui->edName->text().toStdString()));
                  m.SetMarketSign   (trim(ui->edSign->text().toStdString()));
                  m.SetAutoLoad(swtAutoLoadWholeMarket->isChecked()? true:false);
                  m.SetUpToSys(swtUpToSysWholeMarket->isChecked()? true:false);
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
                  std::time_t tS (mktime_gm(&tmSt));
                  m.SetStartTime(tS);
                  //
                  const QTime tmE(ui->dateTimeEnd->time());
                  {
                      tmSt.tm_hour   = tmE.hour();
                      tmSt.tm_min    = tmE.minute();
                      tmSt.tm_sec    = tmE.second();
                  }
                  std::time_t tE (mktime_gm(&tmSt));
                  m.SetEndTime(tE);
                  ///
                  emit NeedSaveMarketsChanges();
                  emit modelMarket->dataChanged(lst[0],lst[0]);

                  bAddingMarketRow = false;
                  slotMarketDataChanged(false);

                  ui->listViewMarket->setFocus();

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
    ui->listViewMarket->setFocus();


    bAddingMarketRow=false;
};
//--------------------------------------------------------------------------------------------------------
///
/// \brief Initialazing procedure. Set data for form. Mast be set during creating form (witn setTickerModel).
/// \param model
/// \param DefaultTickerMarket
///
void ConfigWindow::setMarketModel()
{
    ui->listViewMarket->setModel(modelMarket);


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
    ui->cmbMarkets->setModel(modelMarket);

    {

        connect(ui->cmbMarkets,SIGNAL(activated(const int)),this,SLOT(slotSetSelectedTickersMarket(const  int)));

        bool bWas{false};
        for(int i = 0; i < modelMarket->rowCount(); i++){
            auto idx(modelMarket->index(i,0));
            if(idx.isValid()){
                if(modelMarket->getMarket(idx).MarketID() == iDefaultTickerMarket){
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


        swtAutoLoadWholeMarket->setCheckState  (m.AutoLoad() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
        swtUpToSysWholeMarket->setCheckState   (m.UpToSys()  ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);



        std::time_t tS (m.StartTime());
        std::tm* tmSt=threadfree_gmtime(&tS);
        const QTime tmS(tmSt->tm_hour,tmSt->tm_min,0);
        std::time_t tE (m.EndTime());
        std::tm* tmEn=threadfree_gmtime(&tE);
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
void ConfigWindow::setTickerModel()//TickersListModel *model,bool ShowByName,bool SortByName
{


    proxyTickerModel.setDefaultMarket(iDefaultTickerMarket);
    proxyTickerModel.setSourceModel(modelTicker);
    ui->listViewTicker->setModel(&proxyTickerModel);

    if(swtShowByNameTicker->isChecked())
        ui->listViewTicker->setModelColumn(0);
    else
        ui->listViewTicker->setModelColumn(2);

    if(swtSortByNameTicker->isChecked())
        proxyTickerModel.sort(0);
    else
        proxyTickerModel.sort(2);

    proxyTickerModel.invalidate();



    connect(ui->listViewTicker,SIGNAL(clicked(const QModelIndex&)),this,SLOT(slotSetSelectedTicker(const  QModelIndex&)));

    /////////

    QItemSelectionModel  *qml =new QItemSelectionModel(&proxyTickerModel);
    ui->listViewTicker->setSelectionModel(qml);


    connect(qml,SIGNAL(currentRowChanged(const QModelIndex&,const QModelIndex&)),this,SLOT(slotSetSelectedTicker(const  QModelIndex&,const QModelIndex&)));


    auto first_i(proxyTickerModel.index(0,0));
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


        swtAutoLoadTicker->setCheckState  (t.AutoLoad() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
        swtUpToSysTicker->setCheckState   (t.UpToSys()  ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);

        swtBulbululatorTicker->setCheckState  (t.Bulbululator() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);

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

    swtAutoLoadTicker->setCheckState( Qt::CheckState::Checked);
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
                Ticker t {              trim(ui->edTickerName->text().toStdString()),
                                        trim(ui->edTickerSign->text().toStdString()),
                                        iDefaultTickerMarket};
                t.SetTickerSignFinam    (trim(ui->edTickerSignFinam->text().toStdString()));
                t.SetTickerSignQuik     (trim(ui->edTickerSignQuik->text().toStdString()));
                t.SetAutoLoad(swtAutoLoadTicker->isChecked()? true:false);
                t.SetUpToSys(swtUpToSysTicker->isChecked()? true:false);
                t.SetBulbululator(swtBulbululatorTicker->isChecked()? true:false);

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

                      t.SetTickerName           (trim(ui->edTickerName->text().toStdString()));
                      t.SetTickerSign           (trim(ui->edTickerSign->text().toStdString()));
                      t.SetTickerSignFinam      (trim(ui->edTickerSignFinam->text().toStdString()));
                      t.SetTickerSignQuik       (trim(ui->edTickerSignQuik->text().toStdString()));
                      t.SetAutoLoad             (swtAutoLoadTicker->isChecked()? true:false);
                      t.SetUpToSys              (swtUpToSysTicker->isChecked()? true:false);
                      t.SetBulbululator         (swtBulbululatorTicker->isChecked()? true:false);
                      ///
                      proxyTickerModel.setData(lst[0],t,Qt::EditRole);
                }

                slotTickerDataChanged(false);
            }
            ui->listViewTicker->setFocus();
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
    ui->listViewTicker->setFocus();


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
    swtAutoLoadTicker->setEnabled(bEnable);
    swtUpToSysTicker->setEnabled(bEnable);
    swtBulbululatorTicker->setEnabled(bEnable);
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

    swtAutoLoadTicker->setCheckState  (Qt::CheckState::Unchecked);
    swtUpToSysTicker->setCheckState   (Qt::CheckState::Unchecked);
    swtBulbululatorTicker->setCheckState   (Qt::CheckState::Unchecked);

}

//--------------------------------------------------------------------------------------------------------
void ConfigWindow::slotDefPathChenged(int /*iDef*/)
{
    slotSetPathVisibility();
    if(swtDefPath->isChecked() && bDefStoragePath){
        slotGeneralCancelClicked();
    }
    else{
        slotStoragePathChanged({});
    }
}
//--------------------------------------------------------------------------------------------------------
void ConfigWindow::slotStoragePathChanged(const QString &)
{
    bDataGeneralChanged = true;
    ui->btnGeneralSave ->setEnabled(true);
    ui->btnGeneralCancel ->setEnabled(true);

}
//--------------------------------------------------------------------------------------------------------
void ConfigWindow::slotGeneralCancelClicked()
{
    swtDefPath->setChecked(bDefStoragePath);
    ui->edStoragePath->setText(qsStoragePath);
    ui->btnGeneralSave ->setEnabled(false);
    ui->btnGeneralCancel ->setEnabled(false);
    slotSetPathVisibility();

    bDataGeneralChanged = false;
}
//--------------------------------------------------------------------------------------------------------
void ConfigWindow::slotSetPathVisibility()
{
    if(!swtDefPath->isChecked()){
        ui->btnSelectStoragePath->setEnabled(true);
        ui->edStoragePath->setEnabled(true);
    }
    else{
        ui->btnSelectStoragePath->setEnabled(false);
        ui->edStoragePath->setEnabled(false);
    }
}
//--------------------------------------------------------------------------------------------------------
void ConfigWindow::slotGeneralSaveClicked()
{
    if (bDataGeneralChanged){
        int n=QMessageBox::warning(0,tr("Warning"),
                               tr("To change the path to the storage, programm need to be restarted. Continue?"),
                               QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel
                               );
        if (n==QMessageBox::Yes){
            emit NeedChangeDefaultPath(swtDefPath->isChecked(),ui->edStoragePath->text());
        }
        else if (n==QMessageBox::Cancel){
            slotGeneralCancelClicked();
        }
    }
}
//--------------------------------------------------------------------------------------------------------
void ConfigWindow::slotGeneralOpenStorageDirClicked()
{
    QString str=QFileDialog::getExistingDirectory(0,"Open dialog","");
    ui->edStoragePath->setText(str);
}
//--------------------------------------------------------------------------------------------------------
