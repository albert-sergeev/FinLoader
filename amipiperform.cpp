#include "amipiperform.h"
#include "ui_amipiperform.h"
#include<QMessageBox>

//--------------------------------------------------------------------------------------------------------------------
AmiPiperForm::AmiPiperForm(modelMarketsList *modelM, int DefaultTickerMarket,
                           modelTickersList *modelT,
                           AmiPipeHolder & p,
                           QWidget *parent) :
    QWidget(parent),
    iDefaultTickerMarket{DefaultTickerMarket},
    modelMarket{modelM},
    modelTicker{modelT},
    pipes{p},
    ui(new Ui::AmiPiperForm)
{
    ui->setupUi(this);
    ///////////////////////////////////////////////////////////////////////



    ///////////////////////////////////////////////////////////////////////
    /// \brief connect

    connect(ui->btnCheck,SIGNAL(clicked()),this,SLOT(slotBtnCheckClicked()));

    SetMarketModel();
    SetTickerModel();

    connect(ui->btnOffAll,SIGNAL(clicked()),this,SLOT(slotOffAllClicked()));
    connect(ui->btnOffOne,SIGNAL(clicked()),this,SLOT(slotOffOneClicked()));
    connect(ui->btnOnAll,SIGNAL(clicked()),this,SLOT(slotOnAllClicked()));
    connect(ui->btnOnOne,SIGNAL(clicked()),this,SLOT(slotOnOneClicked()));

    connect(ui->btnDissociateActive,SIGNAL(clicked()),this,SLOT(slotActiveDissociateClicked()));
    connect(ui->btnDissociateOff,SIGNAL(clicked()),this,SLOT(slotOffDissociateClicked()));

    connect(ui->lstTickersActive,SIGNAL(doubleClicked(const QModelIndex&)),this,SLOT(slotDoubleClickedActive(const  QModelIndex&)));
    connect(ui->lstTickersOff,SIGNAL(doubleClicked(const QModelIndex&)),this,SLOT(slotDoubleClickedOff(const  QModelIndex&)));

    //ui->lstTickersActive->AnyKeyPressed
    ui->lstTickersActive->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
    ui->lstTickersOff->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
    ui->lstTickersUnallocated->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);

    connect(ui->lstTickersActive,SIGNAL(customContextMenuRequested(const QPoint &)),this,SLOT(slotActiveContextMenuRequested(const QPoint &)));
    connect(ui->lstTickersOff,SIGNAL(customContextMenuRequested(const QPoint &)),this,SLOT(slotOffContextMenuRequested(const QPoint &)));
    connect(ui->lstTickersUnallocated,SIGNAL(customContextMenuRequested(const QPoint &)),this,SLOT(slotUnallocatedContextMenuRequested(const QPoint &)));


}
//--------------------------------------------------------------------------------------------------------------------
AmiPiperForm::~AmiPiperForm()
{
    delete ui;
}
//--------------------------------------------------------------------------------------------------------------------
void AmiPiperForm::slotBtnCheckClicked()
{
    auto lstPipes = pipes.CheckActivePipes();

    //for (const auto & e:lstPipes){
        //ui->textEdit->append(QString::fromStdString(e));
    //}


    emit SendToMainLog("ami clicked");
}
//--------------------------------------------------------------------------------------------------------------------
void AmiPiperForm::SetMarketModel()
{
    //ui->viewTickets
    ui->cmbMarketsActive->setModel(modelMarket);
    ui->cmbMarketsOff->setModel(modelMarket);
    ui->cmbMarketsUnallocated->setModel(modelMarket);

    {

        connect(ui->cmbMarketsActive,SIGNAL(activated(int)),this,SLOT(slotSetSelectedTickersMarket(int)));
        connect(ui->cmbMarketsOff,SIGNAL(activated(int)),this,SLOT(slotSetSelectedTickersMarket(int)));
        connect(ui->cmbMarketsUnallocated,SIGNAL(activated(int)),this,SLOT(slotSetSelectedTickersMarket(int)));

        bool bWas{false};
        for(int i = 0; i < modelMarket->rowCount(); i++){
            auto idx(modelMarket->index(i,0));
            if(idx.isValid()){
                if(modelMarket->getMarket(idx).MarketID() == iDefaultTickerMarket){
                    //iDefaultTickerMarket = DefaultTickerMarket;
                    ui->cmbMarketsActive->setCurrentIndex(i);
                    ui->cmbMarketsOff->setCurrentIndex(i);
                    ui->cmbMarketsUnallocated->setCurrentIndex(i);
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
}
//--------------------------------------------------------------------------------------------------------------------
void AmiPiperForm::SetTickerModel()
{
    proxyTickerModelActive.setSourceModel(modelTicker);
    proxyTickerModelActive.setDefaultMarket(iDefaultTickerMarket);
    proxyTickerModelActive.sort(2);
    proxyTickerModelActive.setFilterByActive(true);
    proxyTickerModelActive.setFilterByUnallocate(true);
    ui->lstTickersActive->setModel(&proxyTickerModelActive);
    ui->lstTickersActive->setModelColumn(2);
    connect(ui->lstTickersActive,SIGNAL(clicked(const QModelIndex&)),this,SLOT(slotSetSelectedTickerActive(const QModelIndex&)));
    //
    proxyTickerModelOff.setSourceModel(modelTicker);
    proxyTickerModelOff.setDefaultMarket(iDefaultTickerMarket);
    proxyTickerModelOff.sort(2);
    proxyTickerModelOff.setFilterByOff(true);
    proxyTickerModelOff.setFilterByUnallocate(true);
    ui->lstTickersOff->setModel(&proxyTickerModelOff);
    ui->lstTickersOff->setModelColumn(2);
    connect(ui->lstTickersOff,SIGNAL(clicked(const QModelIndex&)),this,SLOT(slotSetSelectedTickerOff(const QModelIndex&)));
    //
    proxyTickerModelUnallocated.setSourceModel(modelTicker);
    proxyTickerModelUnallocated.setDefaultMarket(iDefaultTickerMarket);
    proxyTickerModelUnallocated.sort(2);
    proxyTickerModelUnallocated.setFilterByAllocate(true);
    ui->lstTickersUnallocated->setModel(&proxyTickerModelUnallocated);
    ui->lstTickersUnallocated->setModelColumn(2);
    connect(ui->lstTickersUnallocated,SIGNAL(clicked(const QModelIndex&)),this,SLOT(slotSetSelectedTickerUnallocated(const QModelIndex&)));


    ////////////////////////////////////////////
    QItemSelectionModel  *qml =new QItemSelectionModel(&proxyTickerModelActive);
    ui->lstTickersActive->setSelectionModel(qml);
    connect(qml,SIGNAL(currentRowChanged(const QModelIndex&,const QModelIndex&)),this,SLOT(slotSetSelectedTickerActive(const  QModelIndex&,const QModelIndex&)));
    auto first_i(proxyTickerModelActive.index(0,0));
    if(first_i.isValid() && qml){
        qml->select(first_i,QItemSelectionModel::SelectionFlag::ClearAndSelect | QItemSelectionModel::Rows) ;
        slotSetSelectedTickerActive(first_i,first_i);
    }
    ////////////////////////////////////////////
    qml =new QItemSelectionModel(&proxyTickerModelOff);
    ui->lstTickersOff->setSelectionModel(qml);
    connect(qml,SIGNAL(currentRowChanged(const QModelIndex&,const QModelIndex&)),this,SLOT(slotSetSelectedTickerOff(const  QModelIndex&,const QModelIndex&)));
    auto first_off_i(proxyTickerModelOff.index(0,0));
    if(first_off_i.isValid() && qml){
        qml->select(first_off_i,QItemSelectionModel::SelectionFlag::ClearAndSelect | QItemSelectionModel::Rows) ;
        slotSetSelectedTickerActive(first_off_i,first_off_i);
    }
    ////////////////////////////////////////////
    qml =new QItemSelectionModel(&proxyTickerModelUnallocated);
    ui->lstTickersUnallocated->setSelectionModel(qml);
    connect(qml,SIGNAL(currentRowChanged(const QModelIndex&,const QModelIndex&)),this,SLOT(slotSetSelectedTickerUnallocated(const  QModelIndex&,const QModelIndex&)));
    auto first_Un_i(proxyTickerModelUnallocated.index(0,0));
    if(first_Un_i.isValid() && qml){
        qml->select(first_Un_i,QItemSelectionModel::SelectionFlag::ClearAndSelect | QItemSelectionModel::Rows) ;
        slotSetSelectedTickerActive(first_Un_i,first_Un_i);
    }
}
//--------------------------------------------------------------------------------------------------------------------
void AmiPiperForm::slotSetSelectedTickersMarket(int i)
{
    if (modelMarket ==nullptr ||modelTicker ==nullptr ) return;
    //qDebug()<<"i: {"<<i<<"}";
    if( i < modelMarket->rowCount()){

        disconnect(ui->cmbMarketsActive,SIGNAL(activated(int)),this,SLOT(slotSetSelectedTickersMarket(int)));
        disconnect(ui->cmbMarketsOff,SIGNAL(activated(int)),this,SLOT(slotSetSelectedTickersMarket(int)));
        disconnect(ui->cmbMarketsUnallocated,SIGNAL(activated(int)),this,SLOT(slotSetSelectedTickersMarket(int)));

        if (ui->cmbMarketsActive->currentIndex() != i)          ui->cmbMarketsActive->setCurrentIndex(i);
        if (ui->cmbMarketsOff->currentIndex() != i)             ui->cmbMarketsOff->setCurrentIndex(i);
        if (ui->cmbMarketsUnallocated->currentIndex() != i)     ui->cmbMarketsUnallocated->setCurrentIndex(i);

        connect(ui->cmbMarketsActive,SIGNAL(activated(int)),this,SLOT(slotSetSelectedTickersMarket(int)));
        connect(ui->cmbMarketsOff,SIGNAL(activated(int)),this,SLOT(slotSetSelectedTickersMarket(int)));
        connect(ui->cmbMarketsUnallocated,SIGNAL(activated(int)),this,SLOT(slotSetSelectedTickersMarket(int)));

        auto idx(modelMarket->index(i,0));
        if(idx.isValid()){
            if(modelMarket->getMarket(idx).MarketID() != iDefaultTickerMarket){
                iDefaultTickerMarket = modelMarket->getMarket(idx).MarketID();
                emit NeedSaveDefaultTickerMarket(iDefaultTickerMarket);

                proxyTickerModelActive.setDefaultMarket(iDefaultTickerMarket);
                QItemSelectionModel  *qml = ui->lstTickersActive->selectionModel();
                auto first_i(proxyTickerModelActive.index(0,0));
                if(first_i.isValid() && qml){
                    qml->select(first_i,QItemSelectionModel::SelectionFlag::ClearAndSelect | QItemSelectionModel::Rows) ;
                }
                //
                proxyTickerModelOff.setDefaultMarket(iDefaultTickerMarket);
                qml = ui->lstTickersOff->selectionModel();
                auto first_Off_i(proxyTickerModelOff.index(0,0));
                if(first_Off_i.isValid() && qml){
                    qml->select(first_i,QItemSelectionModel::SelectionFlag::ClearAndSelect | QItemSelectionModel::Rows) ;
                }
                //
                proxyTickerModelUnallocated.setDefaultMarket(iDefaultTickerMarket);
                qml = ui->lstTickersUnallocated->selectionModel();
                auto first_Unallocated_i(proxyTickerModelUnallocated.index(0,0));
                if(first_Unallocated_i.isValid() && qml){
                    qml->select(first_Unallocated_i,QItemSelectionModel::SelectionFlag::ClearAndSelect | QItemSelectionModel::Rows) ;
                }
            }
        }
    }
}
//--------------------------------------------------------------------------------------------------------------------
void AmiPiperForm::slotSetSelectedTickerActive(const  QModelIndex& /*indx*/,const QModelIndex& /*indxEnd*/)
{}
//--------------------------------------------------------------------------------------------------------------------
void AmiPiperForm::slotSetSelectedTickerOff(const  QModelIndex& /*indx*/,const QModelIndex& /*indxEnd*/)
{}
//--------------------------------------------------------------------------------------------------------------------
void AmiPiperForm::slotSetSelectedTickerUnallocated(const  QModelIndex& /*indx*/,const QModelIndex& /*indxEnd*/)
{}
//--------------------------------------------------------------------------------------------------------------------
void AmiPiperForm::slotSetSelectedTickerActive(const QModelIndex& indx)
{
    slotSetSelectedTickerActive(indx,indx);
}
//--------------------------------------------------------------------------------------------------------------------
void AmiPiperForm::slotSetSelectedTickerOff(const QModelIndex& indx)
{
    slotSetSelectedTickerOff(indx,indx);
}
//--------------------------------------------------------------------------------------------------------------------
void AmiPiperForm::slotSetSelectedTickerUnallocated(const QModelIndex& indx)
{
    slotSetSelectedTickerUnallocated(indx,indx);
}
//--------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------
void AmiPiperForm::slotOffAllClicked()
{
    auto qml(ui->lstTickersActive->selectionModel());

    ui->lstTickersActive->selectAll();
    auto lst (qml->selectedIndexes());

    {
        ThreadFreeCout pcout;
        pcout <<"lst.size()"<<lst.size()<<"\n";
    }

    while(lst.size() > 0){
        Ticker t = proxyTickerModelActive.getTicker(lst[0]);
        t.SetAutoLoad(false);
        proxyTickerModelActive.setData(lst[0],t,Qt::EditRole);

        lst = qml->selectedIndexes();
    }
}
//--------------------------------------------------------------------------------------------------------------------
void AmiPiperForm::slotOffOneClicked()
{
    auto qml(ui->lstTickersActive->selectionModel());
    auto lst (qml->selectedIndexes());

    while(lst.size() > 0){
        Ticker t = proxyTickerModelActive.getTicker(lst[0]);
        t.SetAutoLoad(false);
        proxyTickerModelActive.setData(lst[0],t,Qt::EditRole);

        lst = qml->selectedIndexes();
    }
}
//--------------------------------------------------------------------------------------------------------------------
void AmiPiperForm::slotOnAllClicked()
{
    auto qml(ui->lstTickersOff->selectionModel());
    ui->lstTickersOff->selectAll();
    auto lst (qml->selectedIndexes());

    {
        ThreadFreeCout pcout;
        pcout <<"lst.size()"<<lst.size()<<"\n";
    }

    while(lst.size() > 0){
        Ticker t = proxyTickerModelOff.getTicker(lst[0]);
        t.SetAutoLoad(true);
        proxyTickerModelOff.setData(lst[0],t,Qt::EditRole);
        lst =qml->selectedIndexes();
    }
}
//--------------------------------------------------------------------------------------------------------------------
void AmiPiperForm::slotOnOneClicked()
{
    auto qml(ui->lstTickersOff->selectionModel());
    auto lst (qml->selectedIndexes());

    while(lst.size() > 0){
        Ticker t = proxyTickerModelOff.getTicker(lst[0]);
        t.SetAutoLoad(true);
        proxyTickerModelOff.setData(lst[0],t,Qt::EditRole);
        lst =qml->selectedIndexes();
    }
}
//--------------------------------------------------------------------------------------------------------------------
void AmiPiperForm::slotActiveDissociateClicked()
{
    int n=QMessageBox::warning(0,tr("Warning"),
                           tr("Do you want to dissiciate tickers?"),
                           QMessageBox::Yes|QMessageBox::No
                           );
    if (n==QMessageBox::Yes){
        auto qml(ui->lstTickersActive->selectionModel());
        auto lst (qml->selectedIndexes());

        while(lst.size() > 0){
            Ticker t = proxyTickerModelActive.getTicker(lst[0]);
            t.SetTickerSignQuik("");
            proxyTickerModelActive.setData(lst[0],t,Qt::EditRole);
            lst =qml->selectedIndexes();
        }
    }
}
//--------------------------------------------------------------------------------------------------------------------
void AmiPiperForm::slotOffDissociateClicked()
{
    int n=QMessageBox::warning(0,tr("Warning"),
                           tr("Do you want to dissiciate tickers?"),
                           QMessageBox::Yes|QMessageBox::No
                           );
    if (n==QMessageBox::Yes){
        auto qml(ui->lstTickersOff->selectionModel());
        auto lst (qml->selectedIndexes());

        while(lst.size() > 0){
            Ticker t = proxyTickerModelOff.getTicker(lst[0]);
            t.SetTickerSignQuik("");
            proxyTickerModelOff.setData(lst[0],t,Qt::EditRole);
            lst =qml->selectedIndexes();
        }
    }
}
//--------------------------------------------------------------------------------------------------------------------
void AmiPiperForm::slotDoubleClickedActive(const  QModelIndex& indx)
{
    if (indx.isValid()){
        Ticker t = proxyTickerModelActive.getTicker(indx);
        t.SetAutoLoad(false);
        proxyTickerModelActive.setData(indx,t,Qt::EditRole);
    }
}
//--------------------------------------------------------------------------------------------------------------------
void AmiPiperForm::slotDoubleClickedOff(const  QModelIndex& indx)
{
    if (indx.isValid()){
        Ticker t = proxyTickerModelOff.getTicker(indx);
        t.SetAutoLoad(true);
        proxyTickerModelOff.setData(indx,t,Qt::EditRole);
    }
}
//--------------------------------------------------------------------------------------------------------------------
void AmiPiperForm::slotActiveContextMenuRequested(const QPoint & pos)
{
    QPoint item = ui->lstTickersActive->mapToGlobal(pos);
    QMenu submenu(this);
    QAction *pOff = submenu.addAction("Off");
    QAction *pDissociate = submenu.addAction("Dissociate");
    QAction* rightClickItem = submenu.exec(item);
    if (rightClickItem && rightClickItem == pOff){
        auto qml(ui->lstTickersActive->selectionModel());
        auto lst (qml->selectedIndexes());

        while(lst.size() > 0){
            Ticker t = proxyTickerModelActive.getTicker(lst[0]);
            t.SetAutoLoad(false);
            proxyTickerModelActive.setData(lst[0],t,Qt::EditRole);
            lst =qml->selectedIndexes();
        }
    }
    if (rightClickItem && rightClickItem == pDissociate){
        int n=QMessageBox::warning(0,tr("Warning"),
                               tr("Do you want to dissiciate tickers?"),
                               QMessageBox::Yes|QMessageBox::No
                               );
        if (n==QMessageBox::Yes){
            auto qml(ui->lstTickersActive->selectionModel());
            auto lst (qml->selectedIndexes());

            while(lst.size() > 0){
                Ticker t = proxyTickerModelActive.getTicker(lst[0]);
                t.SetTickerSignQuik("");
                proxyTickerModelActive.setData(lst[0],t,Qt::EditRole);
                lst =qml->selectedIndexes();
            }
        }
    }
}
//--------------------------------------------------------------------------------------------------------------------
void AmiPiperForm::slotOffContextMenuRequested(const QPoint & pos)
{
    QPoint item = ui->lstTickersOff->mapToGlobal(pos);
    QMenu submenu(this);
    QAction *pOn = submenu.addAction("On");
    QAction *pDissociate = submenu.addAction("Dissociate");
    QAction* rightClickItem = submenu.exec(item);
    if (rightClickItem && rightClickItem == pOn){
        auto qml(ui->lstTickersOff->selectionModel());
        auto lst (qml->selectedIndexes());

        while(lst.size() > 0){
            Ticker t = proxyTickerModelOff.getTicker(lst[0]);
            t.SetAutoLoad(true);
            proxyTickerModelOff.setData(lst[0],t,Qt::EditRole);
            lst =qml->selectedIndexes();
        }
    }
    if (rightClickItem && rightClickItem == pDissociate){
        int n=QMessageBox::warning(0,tr("Warning"),
                               tr("Do you want to dissiciate tickers?"),
                               QMessageBox::Yes|QMessageBox::No
                               );
        if (n==QMessageBox::Yes){
            auto qml(ui->lstTickersOff->selectionModel());
            auto lst (qml->selectedIndexes());

            while(lst.size() > 0){
                Ticker t = proxyTickerModelOff.getTicker(lst[0]);
                t.SetTickerSignQuik("");
                proxyTickerModelOff.setData(lst[0],t,Qt::EditRole);
                lst =qml->selectedIndexes();
            }
        }
    }
}
//--------------------------------------------------------------------------------------------------------------------
void AmiPiperForm::slotUnallocatedContextMenuRequested(const QPoint & /*pos*/)
{
//    {
//        ThreadFreeCout pcout;
//        pcout<<"mnu expectad unallocated\n";
//    }
}
//--------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------
