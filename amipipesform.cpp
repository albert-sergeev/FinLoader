#include "amipipesform.h"
#include "ui_amipipesform.h"
#include<QMessageBox>
#include<QMouseEvent>

//--------------------------------------------------------------------------------------------------------------------
AmiPipesForm::AmiPipesForm(modelMarketsList *modelM, int DefaultTickerMarket,
                           modelTickersList *modelT,
                           AmiPipeHolder & p,
                           std::vector<Ticker> &v,
                           bool ShowByNameUnallocated,
                           bool ShowByNameActive,
                           bool ShowByNameOff,
                           bool bAmiPipesNewWndShown,
                           bool bAmiPipesActiveWndShown,
                           QWidget *parent) :
    QWidget(parent,Qt::Tool |Qt::FramelessWindowHint),//,Qt::Window | Qt::WindowStaysOnTopHint Qt::ToolQt::Sheet
    iDefaultTickerMarket{DefaultTickerMarket},
    modelMarket{modelM},
    modelTicker{modelT},
    pipes{p},
    vTickersLst{v},
    bShowByNameUnallocated{ShowByNameUnallocated},
    bShowByNameActive{ShowByNameActive},
    bShowByNameOff{ShowByNameOff},
    ui(new Ui::AmiPipesForm)
{
    ui->setupUi(this);


    ui->lineLeft->installEventFilter(this);
    ui->lineDivider->installEventFilter(this);


    ///////////////////////////////////////////////////////////////////////
//    trbtnLeft   = new TransparentButton(">","<",true,this);
//    trbtnRight  = new TransparentButton("<",">",true,this);
    trbtnLeft   = new TransparentButton(">",this);
    trbtnRight  = new TransparentButton("<",this);

    connect(trbtnLeft,SIGNAL(stateChanged(int)),this,SLOT(slotTransparentBtnLeftStateChanged(int)));
    connect(trbtnRight,SIGNAL(stateChanged(int)),this,SLOT(slotTransparentBtnRightStateChanged(int)));
    ///////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------
    QColor colorDarkGreen(0, 100, 52,50);
    QColor colorDarkRed(31, 53, 200,40);
    //-------------------------------------------------------------
    QHBoxLayout *lt1 = new QHBoxLayout();
    lt1->setMargin(0);
    ui->wtShowByNameUnallocated->setLayout(lt1);
    swtShowByNameUnallocated = new StyledSwitcher(tr("Show by name"),tr("Show by ticker"),true,10,this);
    lt1->addWidget(swtShowByNameUnallocated);
    swtShowByNameUnallocated->SetOnColor(QPalette::Window,colorDarkGreen);
    swtShowByNameUnallocated->SetOffColor(QPalette::Window,colorDarkRed);
    //-------------------------------------------------------------
    QHBoxLayout *lt2 = new QHBoxLayout();
    lt2->setMargin(0);
    ui->wtShowByNameActive->setLayout(lt2);
    swtShowByNameActive = new StyledSwitcher(tr("Show by name"),tr("Show by ticker"),true,10,this);
    lt2->addWidget(swtShowByNameActive);
    swtShowByNameActive->SetOnColor(QPalette::Window,colorDarkGreen);
    swtShowByNameActive->SetOffColor(QPalette::Window,colorDarkRed);
    //-------------------------------------------------------------
    QHBoxLayout *lt3 = new QHBoxLayout();
    lt3->setMargin(0);
    ui->wtShowByNameOff->setLayout(lt3);
    swtShowByNameOff = new StyledSwitcher(tr("Show by name"),tr("Show by ticker"),true,10,this);
    lt3->addWidget(swtShowByNameOff);
    swtShowByNameOff->SetOnColor(QPalette::Window,colorDarkGreen);
    swtShowByNameOff->SetOffColor(QPalette::Window,colorDarkRed);
    //-------------------------------------------------------------

    if(!bAmiPipesNewWndShown){
        slotTransparentBtnLeftStateChanged(0);
    }
    else if(!bAmiPipesActiveWndShown){
        slotTransparentBtnRightStateChanged(0);
    }

    ///////////////////////////////////////////////////////////////////////

     modelNew = new QStringListModel(this);
     ui->lstTickersNew->setModel(modelNew);
     ui->lstTickersNew->setEditTriggers(QAbstractItemView::NoEditTriggers);

     connect(ui->lstTickersNew,SIGNAL(clicked(const QModelIndex&)),this,SLOT(slotSelectNewTicker(const  QModelIndex&)));

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


    connect(ui->btnBind,SIGNAL(clicked()),this,SLOT(slotBindClicked()));
    connect(ui->btnBindAll,SIGNAL(clicked()),this,SLOT(slotBindAllClicked()));

    connect(ui->lstTickersUnallocated,SIGNAL(doubleClicked(const QModelIndex&)),this,SLOT(slotDoubleClickedUnallocated(const  QModelIndex&)));
    connect(ui->lstTickersNew,SIGNAL(doubleClicked(const QModelIndex&)),this,SLOT(slotDoubleClickedNew(const  QModelIndex&)));

    connect(swtShowByNameUnallocated,SIGNAL(stateChanged(int)),this,SLOT(slotShowByNamesUnallocatedChecked(int)));
    connect(swtShowByNameActive,SIGNAL(stateChanged(int)),this,SLOT(slotShowByNamesActiveChecked(int)));
    connect(swtShowByNameOff,SIGNAL(stateChanged(int)),this,SLOT(slotShowByNamesOffChecked(int)));

    swtShowByNameUnallocated->setChecked(bShowByNameUnallocated);
    swtShowByNameActive->setChecked(bShowByNameActive);
    swtShowByNameOff->setChecked(bShowByNameOff);


    connect(ui->btnQuit,SIGNAL(clicked()),this,SLOT(slotBtnQuitClicked()));


}
//--------------------------------------------------------------------------------------------------------------------
AmiPipesForm::~AmiPipesForm()
{
    if(swtShowByNameUnallocated)    {delete swtShowByNameUnallocated;   swtShowByNameUnallocated = nullptr;}
    if(swtShowByNameActive)         {delete swtShowByNameActive;        swtShowByNameActive = nullptr;}
    if(swtShowByNameOff)            {delete swtShowByNameOff;           swtShowByNameOff = nullptr;}

    if(trbtnLeft)   {delete trbtnLeft;  trbtnLeft = nullptr;}
    if(trbtnRight)  {delete trbtnRight; trbtnRight = nullptr;}

    disconnect(ui->btnBind,SIGNAL(clicked()),this,SLOT(slotBindClicked()));
    delete ui;
}
//--------------------------------------------------------------------------------------------------------------------
void AmiPipesForm::slotBtnCheckClicked()
{
    dataAmiPipeTask::pipes_type mBindedPipes;
    dataAmiPipeTask::pipes_type mBindedPipesOff;
    dataAmiPipeTask::pipes_type mTmpFree;
    dataAmiPipeTask::pipes_type mAsk;

    std::vector<int> vUnconnected;
    std::vector<int> vInformants;

    pipes.CheckPipes(vTickersLst,mBindedPipes,mBindedPipesOff,mTmpFree,vUnconnected,vInformants);


    for (const auto & e:mTmpFree){
        if (mFreePipes.find(e.first) == mFreePipes.end()){
            if (mFreePipesAsked.find(e.first) == mFreePipesAsked.end()){
                mAsk[e.first] = e.second;
                mFreePipesAsked[e.first] = e.second;
            }
        }
    }


    if (mAsk.size() > 0){
        emit AskPipesNames(mAsk);
    }

//    modelNew->removeRows(0,modelNew->rowCount());


//    for (const auto & e:mFreePipes){
//        modelNew->insertRow(modelNew->rowCount());
//        QModelIndex indx =  modelNew->index(modelNew->rowCount()-1);
//        modelNew->setData(indx,QString::fromStdString(e.first));
////        ThreadFreeCout pcout;
////        pcout <<"bind = ["<<e.first<<"]\n";
////        pcout <<"pipe = ["<<std::get<0>(e.second.second)<<"]\n";
////        pcout <<"sign = ["<<std::get<1>(e.second.second)<<"]\n";
////        pcout <<"path = ["<<std::get<2>(e.second.second).path().string()<<"]\n";
//    }
}
//--------------------------------------------------------------------------------------------------------------------
void AmiPipesForm::SetMarketModel()
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
void AmiPipesForm::SetTickerModel()
{
    proxyTickerModelActive.setSourceModel(modelTicker);
    proxyTickerModelActive.setDefaultMarket(iDefaultTickerMarket);
    proxyTickerModelActive.sort(2);
    proxyTickerModelActive.setFilterByActive(true);
    proxyTickerModelActive.setFilterByUnallocate(true);
    ui->lstTickersActive->setModel(&proxyTickerModelActive);
    ui->lstTickersActive->setModelColumn(2);
    slotShowByNamesActiveChecked(bShowByNameActive);
    connect(ui->lstTickersActive,SIGNAL(clicked(const QModelIndex&)),this,SLOT(slotSetSelectedTickerActive(const QModelIndex&)));
    //
    proxyTickerModelOff.setSourceModel(modelTicker);
    proxyTickerModelOff.setDefaultMarket(iDefaultTickerMarket);
    proxyTickerModelOff.sort(2);
    proxyTickerModelOff.setFilterByOff(true);
    proxyTickerModelOff.setFilterByUnallocate(true);
    ui->lstTickersOff->setModel(&proxyTickerModelOff);
    ui->lstTickersOff->setModelColumn(2);
    slotShowByNamesOffChecked(bShowByNameOff);
    connect(ui->lstTickersOff,SIGNAL(clicked(const QModelIndex&)),this,SLOT(slotSetSelectedTickerOff(const QModelIndex&)));
    //
    proxyTickerModelUnallocated.setSourceModel(modelTicker);
    proxyTickerModelUnallocated.setDefaultMarket(iDefaultTickerMarket);
    proxyTickerModelUnallocated.sort(2);
    proxyTickerModelUnallocated.setFilterByAllocate(true);
    ui->lstTickersUnallocated->setModel(&proxyTickerModelUnallocated);
    ui->lstTickersUnallocated->setModelColumn(2);
    slotShowByNamesUnallocatedChecked(bShowByNameUnallocated);
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
//    qml =new QItemSelectionModel(&proxyTickerModelUnallocated);
//    ui->lstTickersUnallocated->setSelectionModel(qml);
//    connect(qml,SIGNAL(currentRowChanged(const QModelIndex&,const QModelIndex&)),this,SLOT(slotSetSelectedTickerUnallocated(const  QModelIndex&,const QModelIndex&)));
//    auto first_Un_i(proxyTickerModelUnallocated.index(0,0));
//    if(first_Un_i.isValid() && qml){
//        qml->select(first_Un_i,QItemSelectionModel::SelectionFlag::ClearAndSelect | QItemSelectionModel::Rows) ;
//        slotSetSelectedTickerActive(first_Un_i,first_Un_i);
//    }
}
//--------------------------------------------------------------------------------------------------------------------
void AmiPipesForm::slotSetSelectedTickersMarket(int i)
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
void AmiPipesForm::slotSetSelectedTickerActive(const  QModelIndex& /*indx*/,const QModelIndex& /*indxEnd*/)
{}
//--------------------------------------------------------------------------------------------------------------------
void AmiPipesForm::slotSetSelectedTickerOff(const  QModelIndex& /*indx*/,const QModelIndex& /*indxEnd*/)
{}
//--------------------------------------------------------------------------------------------------------------------
void AmiPipesForm::slotSetSelectedTickerUnallocated(const  QModelIndex& /*indx*/,const QModelIndex& /*indxEnd*/)
{}
//--------------------------------------------------------------------------------------------------------------------
void AmiPipesForm::slotSetSelectedTickerActive(const QModelIndex& indx)
{
    slotSetSelectedTickerActive(indx,indx);
}
//--------------------------------------------------------------------------------------------------------------------
void AmiPipesForm::slotSetSelectedTickerOff(const QModelIndex& indx)
{
    slotSetSelectedTickerOff(indx,indx);
}
//--------------------------------------------------------------------------------------------------------------------
void AmiPipesForm::slotSetSelectedTickerUnallocated(const QModelIndex& indx)
{
    slotSetSelectedTickerUnallocated(indx,indx);
}
//--------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------
void AmiPipesForm::slotOffAllClicked()
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
void AmiPipesForm::slotOffOneClicked()
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
void AmiPipesForm::slotOnAllClicked()
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
void AmiPipesForm::slotOnOneClicked()
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
void AmiPipesForm::slotActiveDissociateClicked()
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
            auto It (mFreePipes.find(t.TickerSign()));
            if (It != mFreePipes.end()) mFreePipes.erase(It);
            auto ItAsked (mFreePipesAsked.find(t.TickerSign()));
            if (ItAsked != mFreePipesAsked.end()) mFreePipesAsked.erase(It);
        }
    }
}
//--------------------------------------------------------------------------------------------------------------------
void AmiPipesForm::slotOffDissociateClicked()
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
            auto It (mFreePipes.find(t.TickerSign()));
            if (It != mFreePipes.end()) mFreePipes.erase(It);
            auto ItAsked (mFreePipesAsked.find(t.TickerSign()));
            if (ItAsked != mFreePipesAsked.end()) mFreePipesAsked.erase(It);
        }
    }
}
//--------------------------------------------------------------------------------------------------------------------
void AmiPipesForm::slotDoubleClickedActive(const  QModelIndex& indx)
{
    if (indx.isValid()){
        Ticker t = proxyTickerModelActive.getTicker(indx);
        t.SetAutoLoad(false);
        proxyTickerModelActive.setData(indx,t,Qt::EditRole);
    }
}
//--------------------------------------------------------------------------------------------------------------------
void AmiPipesForm::slotDoubleClickedOff(const  QModelIndex& indx)
{
    if (indx.isValid()){
        Ticker t = proxyTickerModelOff.getTicker(indx);
        t.SetAutoLoad(true);
        proxyTickerModelOff.setData(indx,t,Qt::EditRole);
    }
}
//--------------------------------------------------------------------------------------------------------------------
void AmiPipesForm::slotActiveContextMenuRequested(const QPoint & pos)
{
    auto qml(ui->lstTickersActive->selectionModel());
    auto lst (qml->selectedIndexes());
    QString sBind;

    int i = 0;
    int iLastIndx{-1};
    while (i < lst.size()){
//        {
//            ThreadFreeCout pcout;
//            pcout <<"lst[i].row() = "<<lst[i].row()<<"\n";
//        }
        if (lst[i].row() == iLastIndx) {
            i++;
            continue;
        }
        iLastIndx = i;
        //////////////////
        if (sBind.size() > 0 ){
            sBind = tr("Dissociate");
            break;
        }
        const Ticker  &t =proxyTickerModelActive.getTicker(lst[i]);
        sBind = QString::fromStdString(t.TickerSignQuik());
        i++;
    }
    if (sBind.size() == 0 ){
        sBind = tr("Dissociate");
    }
    //////////////////////


    QPoint item = ui->lstTickersActive->mapToGlobal(pos);
    QMenu submenu(this);
    QAction *pOff = submenu.addAction(tr("Off"));
    QAction *pDissociate = submenu.addAction(sBind);
    QAction* rightClickItem = submenu.exec(item);
    if (rightClickItem && rightClickItem == pOff){


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

            while(lst.size() > 0){
                Ticker t = proxyTickerModelActive.getTicker(lst[0]);
                t.SetTickerSignQuik("");
                proxyTickerModelActive.setData(lst[0],t,Qt::EditRole);
                lst =qml->selectedIndexes();
                auto It (mFreePipes.find(t.TickerSign()));
                if (It != mFreePipes.end()) mFreePipes.erase(It);
            }
        }
    }
}
//--------------------------------------------------------------------------------------------------------------------
void AmiPipesForm::slotOffContextMenuRequested(const QPoint & pos)
{
    auto qml(ui->lstTickersOff->selectionModel());
    auto lst (qml->selectedIndexes());
    QString sBind;

    int i = 0;
    int iLastIndx{-1};
    while (i < lst.size()){
//        {
//            ThreadFreeCout pcout;
//            pcout <<"lst[i].row() = "<<lst[i].row()<<"\n";
//        }
        if (lst[i].row() == iLastIndx) {
            i++;
            continue;
        }
        iLastIndx = i;
        //////////////////
        if (sBind.size() > 0 ){
            sBind = tr("Dissociate");
            break;
        }
        const Ticker  &t =proxyTickerModelOff.getTicker(lst[i]);
        sBind = QString::fromStdString(t.TickerSignQuik());
        i++;
    }
    if (sBind.size() == 0 ){
        sBind = tr("Dissociate");
    }
    //////////////////////


    QPoint item = ui->lstTickersOff->mapToGlobal(pos);
    QMenu submenu(this);
    QAction *pOn = submenu.addAction(tr("On"));
    QAction *pDissociate;
    pDissociate = submenu.addAction(sBind);


    QAction* rightClickItem = submenu.exec(item);
    if (rightClickItem && rightClickItem == pOn){


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

            while(lst.size() > 0){
                Ticker t = proxyTickerModelOff.getTicker(lst[0]);
                t.SetTickerSignQuik("");
                proxyTickerModelOff.setData(lst[0],t,Qt::EditRole);
                lst =qml->selectedIndexes();
                auto It (mFreePipes.find(t.TickerSign()));
                if (It != mFreePipes.end()) mFreePipes.erase(It);
            }
        }
    }
}
//--------------------------------------------------------------------------------------------------------------------
void AmiPipesForm::slotUnallocatedContextMenuRequested(const QPoint & pos)
{
    auto qml(ui->lstTickersUnallocated->selectionModel());
    auto lst (qml->selectedIndexes());

    if (lst.size()>0){
        const Ticker  &t =proxyTickerModelUnallocated.getTicker(lst[0]);
        QPoint item = ui->lstTickersUnallocated->mapToGlobal(pos);
        QAction *pOn{nullptr};
        QAction *pOff{nullptr};
        QMenu submenu(this);
        if (t.AutoLoad()){
            pOff = submenu.addAction(tr("Off"));
        }
        else{
            pOn = submenu.addAction(tr("On"));
        }

        QAction* rightClickItem = submenu.exec(item);
        if (rightClickItem && pOn && rightClickItem == pOn){
            Ticker t = proxyTickerModelUnallocated.getTicker(lst[0]);
            t.SetAutoLoad(true);
            proxyTickerModelUnallocated.setData(lst[0],t,Qt::EditRole);
        }
        if (rightClickItem && pOff && rightClickItem == pOff){
            Ticker t = proxyTickerModelUnallocated.getTicker(lst[0]);
            t.SetAutoLoad(false);
            proxyTickerModelUnallocated.setData(lst[0],t,Qt::EditRole);
        }
    }
}
//--------------------------------------------------------------------------------------------------------------------
void AmiPipesForm::slotSelectNewTicker(const  QModelIndex& indx)
{
    bool bWas {false};
    if (indx.isValid()){
        QString str = modelNew->itemData(indx)[0].value<QString>();
        {
            auto It (mFreePipes.find(str.toStdString()));
            if (It != mFreePipes.end()){

                QModelIndex indx;
                std::string sSign = std::get<1>(It->second.second);
                if (proxyTickerModelUnallocated.searchTickerByPureSign(sSign,indx)){
                    QItemSelectionModel *qml = ui->lstTickersUnallocated->selectionModel();
                    qml->select(indx,QItemSelectionModel::SelectionFlag::ClearAndSelect | QItemSelectionModel::SelectionFlag::Rows);
                    bWas = true;
                }
            }
        }
    }
    if (!bWas){
        QItemSelectionModel *qml = ui->lstTickersUnallocated->selectionModel();
        qml->clearSelection();
    }
}
//--------------------------------------------------------------------------------------------------------------------
void AmiPipesForm::slotBindClicked()
{
    QItemSelectionModel *qmlNew = ui->lstTickersNew->selectionModel();
    auto lst (qmlNew->selectedIndexes());

    QItemSelectionModel *qmlUn = ui->lstTickersUnallocated->selectionModel();
    auto lstUn (qmlUn->selectedIndexes());


    std::string sSign;
    std::string sName;
    std::string sBind;

    if(lst.size() >0 ){
        QString str = modelNew->itemData(lst[0])[0].value<QString>();
        sBind = str.toStdString();
        sSign = sBind;
        sName = sSign;
        auto ItFree(mFreePipes.find(sBind));
        if ( ItFree != mFreePipes.end()){
            sSign = std::get<1>(ItFree->second.second);

            if((std::get<5>(ItFree->second.second)).size() > 0){
                sName = std::get<5>(ItFree->second.second);
            }
        }

        //
        if (lstUn.size()>0){
            Ticker t=proxyTickerModelUnallocated.getTicker(lstUn[0]);


            t.SetTickerSignQuik       (trim(sBind));
            t.SetAutoLoad             (true);
            //t.SetBulbululator         (true);
            if (t.TickerName() == t.TickerSign()){
                t.SetTickerName(sName);
            }
            ///
            proxyTickerModelUnallocated.setData(lstUn[0],t,Qt::EditRole);

            modelNew->removeRows(lst[0].row(),1);
        }
        else{

            QString sQuestion;
            sQuestion = tr("No ticker with <") + QString::fromStdString(sSign) + "> " +
                    tr("sign were found in the database for the current market! Create new and bind it with {")+
                    QString::fromStdString(sBind)+
                    tr("} data source?\n");
            int n=QMessageBox::warning(0,tr("Warning"),sQuestion,QMessageBox::Yes | QMessageBox::No);
            if (n==QMessageBox::Yes){
                Ticker t {              trim(sName),
                                        trim(sSign),
                                        iDefaultTickerMarket};
                t.SetTickerSignFinam    ("");
                t.SetTickerSignQuik     (trim(sBind));
                t.SetAutoLoad(true);
                t.SetUpToSys(false);
                t.SetBulbululator(true);

                (void) proxyTickerModelUnallocated.AddRow(t);

                modelNew->removeRows(lst[0].row(),1);
            }
        }
    }
}
//--------------------------------------------------------------------------------------------------------------------
void AmiPipesForm::slotDoubleClickedUnallocated(const  QModelIndex&)
{


}
//--------------------------------------------------------------------------------------------------------------------
void AmiPipesForm::slotDoubleClickedNew(const  QModelIndex& indx)
{
    QItemSelectionModel *qmlUn = ui->lstTickersUnallocated->selectionModel();
    auto lstUn (qmlUn->selectedIndexes());


    std::string sSign;
    std::string sBind;
    std::string sName;

    if(indx.isValid()){
        QString str = modelNew->itemData(indx)[0].value<QString>();
        sBind = str.toStdString();
        sSign = sBind;
        sName = sSign;
        auto ItFree(mFreePipes.find(sBind));
        if ( ItFree != mFreePipes.end()){
            sSign = std::get<1>(ItFree->second.second);

            if((std::get<5>(ItFree->second.second)).size() > 0){
                sName = std::get<5>(ItFree->second.second);
            }
        }
        //
        if (lstUn.size()>0){
            Ticker t=proxyTickerModelUnallocated.getTicker(lstUn[0]);



            t.SetTickerSignQuik       (trim(sBind));
            t.SetAutoLoad             (true);
            //t.SetBulbululator         (true);
            if (t.TickerName() == t.TickerSign()){

                t.SetTickerName(trim(sName));
            }
            ///
            proxyTickerModelUnallocated.setData(lstUn[0],t,Qt::EditRole);

            modelNew->removeRows(indx.row(),1);
        }
        else{

            QString sQuestion;
            sQuestion = tr("No ticker with <") + QString::fromStdString(sSign) + "> " +
                    tr("sign were found in the database for the current market! Create new and bind it with {")+
                    QString::fromStdString(sBind)+
                    tr("} data source?\n");
            int n=QMessageBox::warning(0,tr("Warning"),sQuestion,QMessageBox::Yes | QMessageBox::No);
            if (n==QMessageBox::Yes){
                Ticker t {              trim(sName),
                                        trim(sSign),
                                        iDefaultTickerMarket};
                t.SetTickerSignFinam    ("");
                t.SetTickerSignQuik     (trim(sBind));
                t.SetAutoLoad(true);
                t.SetUpToSys(false);
                t.SetBulbululator(true);

                //int i = modelTicker->AddRow(t);
                (void) proxyTickerModelUnallocated.AddRow(t);

                modelNew->removeRows(indx.row(),1);
            }
        }
    }
}
//--------------------------------------------------------------------------------------------------------------------
void AmiPipesForm::showEvent(QShowEvent */*event*/)
{
    RepositionTransparentButtons();
    slotBtnCheckClicked();
}
//--------------------------------------------------------------------------------------------------------------------
void AmiPipesForm::resizeEvent(QResizeEvent */*event*/)
{
//    {
//        ThreadFreeCout pcout;
//        pcout <<"event->size().width(): "<<event->size().width()<<"\n";
//    }
//    emit WidthWasChanged(event->size().width());
    RepositionTransparentButtons();
}
//--------------------------------------------------------------------------------------------------------------------
void AmiPipesForm::closeEvent(QCloseEvent */*event*/)
{
    emit WasCloseEvent();
}
//--------------------------------------------------------------------------------------------------------------------
void AmiPipesForm::slotShowByNamesUnallocatedChecked(int Checked)
{
    if (Checked){
        ui->lstTickersUnallocated->setModelColumn(0);
    }
    else{
        ui->lstTickersUnallocated->setModelColumn(2);
    }

    proxyTickerModelUnallocated.invalidate();
    NeedSaveShowByNamesUnallocated(Checked);
}
//--------------------------------------------------------------------------------------------------------------------
void AmiPipesForm::slotShowByNamesActiveChecked(int Checked)
{
    if (Checked){
        ui->lstTickersActive->setModelColumn(0);
    }
    else{
        ui->lstTickersActive->setModelColumn(2);
    }

    proxyTickerModelActive.invalidate();
    NeedSaveShowByNamesActive(Checked);
}
//--------------------------------------------------------------------------------------------------------------------
void AmiPipesForm::slotShowByNamesOffChecked(int Checked)
{
    if (Checked){
        ui->lstTickersOff->setModelColumn(0);
    }
    else{
        ui->lstTickersOff->setModelColumn(2);
    }

    proxyTickerModelOff.invalidate();
    emit NeedSaveShowByNamesOff(Checked);
}
//--------------------------------------------------------------------------------------------------------------------
void AmiPipesForm::slotBtnQuitClicked()
{
    //this->close();
    emit buttonHideClicked();
}
//--------------------------------------------------------------------------------------------------------------------
void AmiPipesForm::RepositionTransparentButtons()
{
    QPoint pPos = ui->lineDivider->pos();

    if (trbtnLeft) trbtnLeft->move     (pPos.x() - trbtnLeft->width(), pPos.y() + 2 );
    if (trbtnRight) trbtnRight->move   (pPos.x() /*+ trbtnRight->width()*/+8, pPos.y() + 2 );
}
//--------------------------------------------------------------------------------------------------------------------
void AmiPipesForm::slotTransparentBtnLeftStateChanged(int /*iState*/)
{
    if(!ui->wtNew->isHidden() && !ui->wtActivities->isHidden()){
        QPoint pPos = ui->lineDivider->pos();
        iStoredWidthLeft = this->width() - pPos.x() + ui->lineDivider->width();
        iStoredWidthNew = ui->wtNew->width();

        QRect rect = this->geometry();

        rect.setLeft(rect.left() + iStoredWidthLeft);
        ui->wtNew->hide();
        this->setGeometry(rect);

        trbtnLeft->hide();
        RepositionTransparentButtons();
        emit WidthWasChanged(rect.width());
        trbtnRight->setText(">");
        emit NewWndStateChanged(0);

    }

    if(ui->wtActivities->isHidden()){
        QRect rect = this->geometry();
        rect.setWidth(rect.width() + iStoredWidthRight);

        if (ui->wtActivities->isHidden()) ui->wtActivities->show();
        this->setGeometry(rect);

        trbtnRight->show();
        RepositionTransparentButtons();
        emit WidthWasChanged(rect.width());
        trbtnLeft->setText(">");
        emit ActiveWndStateChanged(1);
    }
}
//--------------------------------------------------------------------------------------------------------------------
void AmiPipesForm::slotTransparentBtnRightStateChanged(int /*iState*/)
{
    if(!ui->wtNew->isHidden() && !ui->wtActivities->isHidden()){
        QPoint pPos = ui->lineDivider->pos();
        iStoredWidthRight = this->width() - pPos.x() - ui->lineDivider->width();
        iStoredWidthActive = ui->wtActivities->width();

        QRect rect = this->geometry();
        rect.setWidth(rect.width() - iStoredWidthRight);

        ui->wtActivities->hide();
        this->setGeometry(rect);

        trbtnRight->hide();
        RepositionTransparentButtons();
        emit WidthWasChanged(rect.width());
        trbtnLeft->setText("<");
        emit ActiveWndStateChanged(0);
    }

    if(ui->wtNew->isHidden()){
        QRect rect = this->geometry();
        //rect.setWidth(rect.width() + iStoredWidthLeft);
        rect.setLeft(rect.left() - iStoredWidthLeft);

        if (ui->wtNew->isHidden()) ui->wtNew->show();
        this->setGeometry(rect);

        trbtnLeft->show();
        RepositionTransparentButtons();
        emit WidthWasChanged(rect.width());
        trbtnRight->setText("<");
        emit NewWndStateChanged(1);
    }
}
//--------------------------------------------------------------------------------------------------------------------
int AmiPipesForm::CalculatedMimimum(){
    const int iMinmumWidth{570}; // miminum was fixed, so...
    int iCalculatedMimimum{iMinmumWidth};
    if(ui->wtNew->isHidden()){
        iCalculatedMimimum -= iStoredWidthNew;
    }
    if(ui->wtActivities->isHidden()){
        iCalculatedMimimum -= iStoredWidthActive;
    }
    if (iCalculatedMimimum > iStoredWidthRight){
        iCalculatedMimimum = iStoredWidthRight;
    }
    return iCalculatedMimimum;
}
//--------------------------------------------------------------------------------------------------------------------
bool AmiPipesForm::eventFilter(QObject *watched, QEvent *event)
{

    if (watched == ui->lineLeft){
        if (event->type() == QEvent::MouseMove){
            if (bInResizingLeftLine){
                QMouseEvent *pe = (QMouseEvent *)event;
                int iStoped = pe->globalX();//pe->pos().x();

                int iNewWidth = iStoredWidthRight + (iStoped - iStoredMousePos);
                if (CalculatedMimimum() < iNewWidth)
                {
                      emit WidthWasChanged(iNewWidth);
                }
            }
        }
        else if (event->type() == QEvent::MouseButtonPress){

            bInResizingLeftLine = true;
            if (!bCursorOverrided){
                QApplication::setOverrideCursor(Qt::CursorShape::SizeHorCursor);
                bCursorOverrided = true;
            }

            QMouseEvent *pe = (QMouseEvent *)event;
            iStoredMousePos = pe->globalX();//pe->pos().x();
            iStoredWidthRight = this->width();

        }
        else if (event->type() == QEvent::MouseButtonRelease){
            bInResizingLeftLine = false;
            if (bCursorOverrided) QApplication::restoreOverrideCursor();
            bCursorOverrided = false;

            QMouseEvent *pe = (QMouseEvent *)event;

            int iStoped = pe->globalX();//pe->pos().x();
            int iNewWidth = iStoredWidthRight + (iStoped - iStoredMousePos);

            if (CalculatedMimimum() < iNewWidth)
            {
                  emit WidthWasChanged(iNewWidth);
                //resizeDocks({ui->dkActiveTickers},{iNewWidth},Qt::Orientation::Horizontal);
            }
        }
        else if (event->type() == QEvent::Enter){
            if (!bCursorOverrided){
                QApplication::setOverrideCursor(Qt::CursorShape::SizeHorCursor);
                bCursorOverrided = true;
            }
        }
        else if (event->type() == QEvent::Leave){
            if (bCursorOverrided) QApplication::restoreOverrideCursor();
            bCursorOverrided = false;
        }
    }
    if (watched == ui->lineDivider){

    }
    return QObject::eventFilter(watched, event);
}
//--------------------------------------------------------------------------------------------------------------------
void AmiPipesForm::slotInternalPanelsStateChanged(bool bLeft, bool bRight)
{
    {
        ThreadFreeCout pcout;
        pcout <<"receive: {"<<bLeft<<":"<<bRight<<"}\n";
    }
    if (bLeft && bRight){
        {
            ThreadFreeCout pcout;
            pcout <<"do show all\n";
        }
        if (ui->wtNew->isHidden()){
            {
                ThreadFreeCout pcout;
                pcout <<"restore left\n";
            }
            slotTransparentBtnRightStateChanged(0);
        }
        else if (ui->wtActivities->isHidden()){
            {
                ThreadFreeCout pcout;
                pcout <<"restore right\n";
            }
            slotTransparentBtnLeftStateChanged(0);
        }
    }
    else if (!bLeft){
        {
            ThreadFreeCout pcout;
            pcout <<"do hide left\n";
        }
        if (!ui->wtNew->isHidden() && !ui->wtActivities->isHidden()){
            slotTransparentBtnLeftStateChanged(0);
        }
        else if (ui->wtActivities->isHidden()){
            slotTransparentBtnLeftStateChanged(0);
            slotTransparentBtnLeftStateChanged(0);
        }
    }
    else if (!bRight){
        {
            ThreadFreeCout pcout;
            pcout <<"do hide right\n";
        }
        if (!ui->wtNew->isHidden() && !ui->wtActivities->isHidden()){
            slotTransparentBtnRightStateChanged(0);
        }
        else if (ui->wtNew->isHidden()){
            slotTransparentBtnRightStateChanged(0);
            slotTransparentBtnRightStateChanged(0);
        }
    }
}
//--------------------------------------------------------------------------------------------------------------------
void AmiPipesForm::slotPipeNameReceived(std::string sBind,std::string sName)
{
    auto It (mFreePipesAsked.find(sBind));
    if ( It != mFreePipesAsked.end()){
        if (mFreePipes.find(sBind) == mFreePipes.end()){
            mFreePipes[sBind] = {It->second.first,{
                                     std::get<0>(It->second.second),
                                     std::get<1>(It->second.second),
                                     std::get<2>(It->second.second),
                                     std::get<3>(It->second.second),
                                     std::get<4>(It->second.second),
                                     sName
                                 }};


        }
    }

    modelNew->removeRows(0,modelNew->rowCount());

    for (const auto & e:mFreePipes){
        modelNew->insertRow(modelNew->rowCount());
        QModelIndex indx =  modelNew->index(modelNew->rowCount()-1);
        modelNew->setData(indx,QString::fromStdString(e.first));
//        ThreadFreeCout pcout;
//        pcout <<"bind = ["<<e.first<<"]\n";
//        pcout <<"pipe = ["<<std::get<0>(e.second.second)<<"]\n";
//        pcout <<"sign = ["<<std::get<1>(e.second.second)<<"]\n";
//        pcout <<"path = ["<<std::get<2>(e.second.second)<<"]\n";
//        pcout <<"name = ["<<std::get<5>(e.second.second)<<"]\n";
    }
}
//--------------------------------------------------------------------------------------------------------------------
void AmiPipesForm::slotBindAllClicked()
{
    QString sQuestion;
    sQuestion = tr("Do you want to bind all pipes to tickers (create new ticker if needed)?\n");
    int n=QMessageBox::warning(0,tr("Warning"),sQuestion,QMessageBox::Yes | QMessageBox::No);
    if (n==QMessageBox::No){
        return;
    }
    /////////////////////////////////////////////////
    std::string sSign;
    std::string sName;
    std::string sBind;

    for (const auto & e:mFreePipes){
        QModelIndex indx;
        sBind = e.first;
        sSign = sBind;
        sName = sBind;

        std::string sSign = std::get<1>(e.second.second);

        if((std::get<5>(e.second.second)).size() > 0){
            sName = std::get<5>(e.second.second);
        }


        //if (proxyTickerModelUnallocated.searchTickerByPureSign(sSign,indx)){
        //    Ticker t=proxyTickerModelUnallocated.getTicker(indx);
        if (modelTicker->searchTickerByPureSign(sSign,indx)){
            Ticker t=modelTicker->getTicker(indx);

            t.SetTickerSignQuik       (trim(sBind));
            t.SetAutoLoad             (true);
            //t.SetBulbululator         (true);
            if (t.TickerName() == t.TickerSign()){
                t.SetTickerName(sName);
            }
            ///
            //proxyTickerModelUnallocated.setData(indx,t,Qt::EditRole);
            modelTicker->setData(indx,t,Qt::EditRole);
        }
        else{
            Ticker t {              trim(sName),
                                    trim(sSign),
                                    iDefaultTickerMarket};
            t.SetTickerSignFinam    ("");
            t.SetTickerSignQuik     (trim(sBind));
            t.SetAutoLoad(true);
            t.SetUpToSys(false);
            t.SetBulbululator(true);

            //int i = modelTicker->AddRow(t);
            //(void) proxyTickerModelUnallocated.AddRow(t);
            (void) modelTicker->AddRow(t);
        }
    }
    modelNew->removeRows(0,modelNew->rowCount());
    mFreePipes.clear();
    mFreePipesAsked.clear();
}
//--------------------------------------------------------------------------------------------------------------------
