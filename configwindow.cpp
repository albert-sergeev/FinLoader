#include "configwindow.h"
#include "ui_configwindow.h"

#include<QMessageBox>

//--------------------------------------------------------------------------------------------------------
ConfigWindow::ConfigWindow(QWidget *parent) :
    QWidget(parent)
    , modelMarket{nullptr}
    , ui(new Ui::ConfigWindow)
{
    ui->setupUi(this);

    try{
        stStore.Initialize();

    }
    //catch (std::runtime_error &e){
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
    //connect(ui->btnSaveMarket,SIGNAL(clicked()),this,SLOT(slotBtnCancelClicked()));
}
//--------------------------------------------------------------------------------------------------------
void ConfigWindow::slotBtnAddClicked()
{


    SendToMainLog(QString::fromStdString(stStore.GetCurrentPath()));
    SendToMainLog("\n");
    SendToMainLog(QString::fromStdString(stStore.GetDataPath()));
    SendToMainLog("\n");
    SendToMainLog(QString::fromStdString(stStore.GetStoragePath()));
    SendToMainLog("\n");

};
//--------------------------------------------------------------------------------------------------------
void ConfigWindow::slotBtnRemoveClicked(){};
void ConfigWindow::slotBtnSaveClicked(){};
void ConfigWindow::slotBtnCancelClicked(){};
void ConfigWindow::slotStateChanged(){};

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

    }
}
//--------------------------------------------------------------------------------------------------------
ConfigWindow::~ConfigWindow()
{
    delete ui;
}
//--------------------------------------------------------------------------------------------------------
