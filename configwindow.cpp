#include "configwindow.h"
#include "ui_configwindow.h"

#include<QMessageBox>

//--------------------------------------------------------------------------------------------------------
ConfigWindow::ConfigWindow(QWidget *parent) :
    QWidget(parent)
    , modelMarket{nullptr}
    , bDataChanged{false}
    , bAddingRow{false}
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
    //
    connect(ui->btnAddMaket,SIGNAL(clicked()),  this,SLOT(slotBtnAddClicked()));
    connect(ui->btnDelMarket,SIGNAL(clicked()), this,SLOT(slotBtnRemoveClicked()));
    connect(ui->btnSaveMarket,SIGNAL(clicked()),this,SLOT(slotBtnSaveClicked()));
    connect(ui->btnCancel,SIGNAL(clicked()),this,SLOT(slotBtnCancelClicked()));
    //
    connect(ui->edName,SIGNAL(textChanged()),this,SLOT(slotDataChanged()));
    connect(ui->edSign,SIGNAL(textChanged()),this,SLOT(slotDataChanged()));
    connect(ui->chkAutoLoad,SIGNAL(stateChanged(int)),this,SLOT(slotDataChanged(int)));
    connect(ui->chkUpToSys,SIGNAL(stateChanged(int)),this,SLOT(slotDataChanged(int)));
    connect(ui->dateTimeStart,SIGNAL(timeChanged(const QTime &)),this,SLOT(slotTimeChanged(const QTime &)));
    connect(ui->dateTimeEnd,SIGNAL(timeChanged(const QTime &)),this,SLOT(slotTimeChanged(const QTime &)));


}
//--------------------------------------------------------------------------------------------------------
bool ConfigWindow::event(QEvent *event)
{
    if(event->type() == QEvent::Close){
        slotAboutQuit();
    }
    return QWidget::event(event);
}
//--------------------------------------------------------------------------------------------------------
void ConfigWindow::slotAboutQuit()
{
    if(bDataChanged){
        slotBtnSaveClicked();
    }
}
//--------------------------------------------------------------------------------------------------------
void ConfigWindow::slotDataChanged(bool Changed )
{
    bDataChanged = Changed;

    ui->btnAddMaket->setEnabled(!bDataChanged);
    //ui->btnDelMarket->setEnabled(bDataChanged);
    ui->btnSaveMarket->setEnabled(bDataChanged);
    ui->btnCancel->setEnabled(bDataChanged);
    ui->listViewMarket->setEnabled(!bDataChanged);
}
//--------------------------------------------------------------------------------------------------------
void ConfigWindow::slotBtnAddClicked()
{
    bAddingRow = true;

    Market m{"",""};
    int i = modelMarket->AddRow(m);

    QItemSelectionModel  *qml =ui->listViewMarket->selectionModel();

    auto indx(modelMarket->index(i,0));
    if(indx.isValid() && qml){
        qml->select(indx,QItemSelectionModel::SelectionFlag::ClearAndSelect) ;
        slotSetSelectedMarket(indx);
    }
    //
    slotDataChanged(true);
};
//--------------------------------------------------------------------------------------------------------
void ConfigWindow::slotBtnRemoveClicked()
{
    int n=QMessageBox::warning(0,tr("Warning"),
                               tr("Do you want to remove market?"),
                               QMessageBox::Yes|QMessageBox::Cancel
                               );
    if (n==QMessageBox::Yes){
        auto qml(ui->listViewMarket->selectionModel());
        auto lst (qml->selectedIndexes());

        if(lst.count() > 0){
            if(lst.count() > 1){
                qml->select(lst[0],QItemSelectionModel::SelectionFlag::ClearAndSelect) ;
            }
            modelMarket->removeItem(lst[0].row());
            NeedSaveMarketsChanges();
            ClearWidgetsValues();
            slotDataChanged(false);
        }
    }
};

//--------------------------------------------------------------------------------------------------------
void ConfigWindow::ClearWidgetsValues()
{
    ui->edName->setPlainText("");
    ui->edSign->setPlainText("");

    ui->chkAutoLoad->setCheckState  (Qt::CheckState::Unchecked);
    ui->chkUpToSys->setCheckState   (Qt::CheckState::Unchecked);

    const QTime tmS(0,0,0);
    const QTime tmE(0,0,0);

    ui->dateTimeStart->setTime(tmS);
    ui->dateTimeEnd->setTime(tmE);
}

//--------------------------------------------------------------------------------------------------------
void ConfigWindow::slotBtnSaveClicked()
{
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
            //slotSetSelectedMarket(lst[0]);
            if(bDataChanged){

                  Market& m=modelMarket->getMarket(lst[0]);

                  m.SetMarketName(ui->edName->toPlainText().toStdString());
                  m.SetMarketSign(ui->edSign->toPlainText().toStdString());
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


            }
        }
        bAddingRow = false;
        slotDataChanged(false);
    }
    if (n==QMessageBox::Cancel){
        slotBtnCancelClicked();
    }
};
//--------------------------------------------------------------------------------------------------------
void ConfigWindow::slotBtnCancelClicked()
{
    auto qml(ui->listViewMarket->selectionModel());
    auto lst (qml->selectedIndexes());

    if (!bAddingRow){
        if(lst.count() > 0){
            if(lst.count() > 1){
                qml->select(lst[0],QItemSelectionModel::SelectionFlag::Select) ;
            }
            slotSetSelectedMarket(lst[0]);
        }
    }
    else{
        for(const auto & r:lst)
        {
            modelMarket->removeItem(r.row());
            ClearWidgetsValues();
            slotDataChanged(false);
        }
    }

    bAddingRow=false;
};
//--------------------------------------------------------------------------------------------------------
void ConfigWindow::setMarketModel(MarketsListModel *model)
{
    modelMarket = model;
    ui->listViewMarket->setModel(model);
    //connect(ui->listViewMarket,SIGNAL(activated(const QModelIndex&)),this,SLOT(slotSetSelectedMarket(const  QModelIndex&)));
    connect(ui->listViewMarket,SIGNAL(clicked(const QModelIndex&)),this,SLOT(slotSetSelectedMarket(const  QModelIndex&)));
    /////////


    QItemSelectionModel  *qml =new QItemSelectionModel(modelMarket);
    ui->listViewMarket->setSelectionModel(qml);

    auto first_i(modelMarket->index(0,0));
    if(first_i.isValid()){
        qml->select(first_i,QItemSelectionModel::SelectionFlag::Select) ;
        slotSetSelectedMarket(first_i);
    }
    ////
}
//--------------------------------------------------------------------------------------------------------

void ConfigWindow::slotSetSelectedMarket(const  QModelIndex& indx)
{
    if (indx.isValid()){
        const Market& m=modelMarket->getMarket(indx);
        ui->edName->setPlainText(QString::fromStdString(m.MarketName()));
        ui->edSign->setPlainText(QString::fromStdString(m.MarketSign()));

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

        slotDataChanged(false);
    }
}
//--------------------------------------------------------------------------------------------------------
ConfigWindow::~ConfigWindow()
{
    delete ui;
}
//--------------------------------------------------------------------------------------------------------
