#include "mainwindow.h"
#include "ui_mainwindow.h"

#include<QListView>
#include<QDir>
#include<QHBoxLayout>
#include<QStandardPaths>
#include<QProcess>
#include<QMouseEvent>
#include<QEvent>
#include<QTimer>



using seconds=std::chrono::duration<double>;
using milliseconds=std::chrono::duration<double,
    std::ratio_multiply<seconds::period,std::milli>
    >;


//--------------------------------------------------------------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , lcdN{nullptr}
    ,iStoredUsedMemory{0}
    , vMarketsLst{}   
    , m_MarketLstModel{vMarketsLst,this}
    , vTickersLst{}
    , m_TickerLstModel{vTickersLst,vMarketsLst,mBlinkedState,this}
    , pAmiPipeWindow{nullptr}
    , thrdPoolLoadFinQuotes{(int) (2 * double(std::thread::hardware_concurrency())/3.0)}
    , thrdPoolAmiClient{1}
    , thrdPoolFastDataWork{(int)std::thread::hardware_concurrency()/2}
    , ui(new Ui::MainWindow)
{

    ui->setupUi(this);

    //==============================================================================================================================
    // init widgets part
    //-------------------------------------------------------------
    ui->statusbar->setSizeGripEnabled(false);
    //
    lcdN = new QLCDNumber;
    lcdN->setDigitCount(8);
    lcdN->setMaximumHeight(16);
    lcdN->setMinimumWidth(120);
    lcdN->display(QString("00:00:00"));
    lcdN->setToolTip(tr("server time of the last packet"));
    ui->statusbar->addWidget(lcdN);
    //ui->statusbar->addPermanentWidget(lcdN);
    //
    wtCombIndicator = new CombIndicator(int(thrdPoolLoadFinQuotes.MaxThreads()+
                                        thrdPoolAmiClient.MaxThreads()+
                                        thrdPoolFastDataWork.MaxThreads())
                                        );
    //ui->statusbar->addWidget(wtCombIndicator);
    ui->statusbar->addPermanentWidget(wtCombIndicator);
    //-------------------------------------------------------------

    //-------------------------------------------------------------
    QColor colorDarkGreen(0, 100, 52,50);
    QColor colorDarkRed(31, 53, 200,40);
    //-------------------------------------------------------------
    QHBoxLayout *lt1 = new QHBoxLayout();
    lt1->setMargin(0);
    ui->wtShowAll->setLayout(lt1);
    swtShowAll = new StyledSwitcher(tr("Show all"),tr(" Active"),true,10,this);
    lt1->addWidget(swtShowAll);
    swtShowAll->SetOnColor(QPalette::Window,colorDarkGreen);
    swtShowAll->SetOffColor(QPalette::Window,colorDarkRed);
    //-------------------------------------------------------------
    QHBoxLayout *lt2 = new QHBoxLayout();
    lt2->setMargin(0);
    ui->wtShowMarket->setLayout(lt2);
    swtShowMarkets = new StyledSwitcher(tr("Show market "),tr(" Hide market"),true,10,this);
    lt2->addWidget(swtShowMarkets);
    swtShowMarkets->SetOnColor(QPalette::Window,colorDarkGreen);
    swtShowMarkets->SetOffColor(QPalette::Window,colorDarkRed);
    //-------------------------------------------------------------
    QHBoxLayout *lt3 = new QHBoxLayout();
    lt3->setMargin(0);
    ui->wtShowByName->setLayout(lt3);
    swtShowByName = new StyledSwitcher(tr("Show name "),tr(" Show ticker"),true,10,this);
    lt3->addWidget(swtShowByName);
    swtShowByName->SetOnColor(QPalette::Window,colorDarkGreen);
    swtShowByName->SetOffColor(QPalette::Window,colorDarkRed);
    //-------------------------------------------------------------
    ui->dkActiveTickers->setTitleBarWidget(new QWidget());
//    //ui->lineDragRight->installEventFilter(this);
    ui->wtTickerBar->installEventFilter(this);
    ui->wtTickerBar->setAttribute(Qt::WA_Hover, true);

    bInResizingLeftToolbar = false;
    bLeftToolbarCursorOverriden = false;


    //==============================================================================================================================
    // init data part


    LoadSettings();
    slotSetActiveLang (m_Language);
    slotSetActiveStyle(m_sStyleName);
    m_TickerLstModel.setGrayColorForInformants(bGrayColorFroNotAutoloadedTickers);

    InitAction();

    LoadDataStorage();

    std::copy(vTickersLst.begin(),vTickersLst.end(),std::back_inserter(vTickersLstEtalon));

    pipesHolder.setCurrentPath(stStore.GetCurrentPath());
    BuildSessionsTableForFastTasks(fastHolder);
    iPhisicalMemory = getPhisicalMemory();

    InitDockBar();

    connect(&m_TickerLstModel,SIGNAL(dataChanged(const QModelIndex &,const QModelIndex &)), this,SLOT(slotTickerDataStorageUpdate(const QModelIndex &,const QModelIndex &)));
    connect(&m_TickerLstModel,SIGNAL(dataRemoved(const Ticker &)), this,SLOT(slotTickerDataStorageRemove(const Ticker &)));



    connect(swtShowByName,SIGNAL(stateChanged(int)),this,SLOT(slotDocbarShowNyNameChanged(int)));
    connect(swtShowAll,SIGNAL(stateChanged(int)),this,SLOT(slotDocbarShowAllChanged(int)));
    connect(swtShowMarkets,SIGNAL(stateChanged(int)),this,SLOT(slotDocbarShowMarketChanged(int)));

    connect(this,SIGNAL(SendToLog(QString)),this,SLOT(slotSendToLog(QString)));
    connect(this,SIGNAL(SendToErrorLog(QString)),this,SLOT(slotSendToErrorLog(QString)));


    ////////////////////////////////////////////////////////////////////
    iTimerID = startTimer(100); // timer to process GUID events
    //
    InitHolders();
    //
    dtCheckPipesActivity = std::chrono::steady_clock::now();
    dtCheckMemoryUsage = std::chrono::steady_clock::now();
    CheckActivePipes();
    ////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////
    thrdPoolAmiClient.AddTask([&](){
        workerLoader::workerAmiClient(queueFinQuotesLoad,queueTrdAnswers,
                                      queuePipeTasks,queuePipeAnswers,
                                      queueFastTasks,
                                      pipesHolder);
        });


    ////////////////////////////////////////////////////////////////////////////////////////////
    size_t i = 0;
    while(i < thrdPoolFastDataWork.MaxThreads()){
            thrdPoolFastDataWork.AddTask([&](){
                        workerLoader::workerFastDataWork (  queueFastTasks,
                                                            queuePipeAnswers,
                                                            fastHolder,
                                                            stStore,
                                                            Holders);
                        });
        ++i;
    }
    ////////////////////////////////////////////////////////////////////////////////////////////
}
//--------------------------------------------------------------------------------------------------------------------------------
MainWindow::~MainWindow()
{
    SaveSettings();

//    delete m_mnuWindows;
//    delete m_mnuStyles;
//    delete m_mnuLangs;
//    delete m_mnuGraphViewConfig;

//    delete pmnuFile;
//    delete pmnuTools;
//    delete pmnuSettings;
//    delete pmnuHelp;

    if(swtShowByName)   {delete swtShowByName;  swtShowByName = nullptr;}
    if(swtShowAll)      {delete swtShowAll;     swtShowAll = nullptr;}
    if(swtShowMarkets)  {delete swtShowMarkets; swtShowMarkets = nullptr;}

    delete ui;
    //
    {
        ThreadFreeCout pcout;
        pcout <<"~MainWindow()\n";
    }
    bWasClose = true;
    thrdPoolLoadFinQuotes.Halt();
    thrdPoolAmiClient.Halt();
    thrdPoolFastDataWork.Halt();
    this_thread_flagInterrup.set();

    std::this_thread::yield();
    std::this_thread::sleep_for(milliseconds(100));
    std::this_thread::yield();

    queueFastTasks.clear();
    queueFinQuotesLoad.clear();
    queueTrdAnswers.clear();
    queuePipeTasks.clear();
    queuePipeAnswers.clear();

    {
        std::shared_lock lk(mutexMainHolder);
        for(auto &h:Holders) h.second->clear();
    }

    std::this_thread::yield();
    std::this_thread::sleep_for(milliseconds(100));
    std::this_thread::yield();
}
//--------------------------------------------------------------------------------------------------------------------------------
bool MainWindow::event(QEvent *event)
{
    if(event->type() == QEvent::Close){
        this->killTimer(iTimerID);
        {
            ThreadFreeCout pcout;
            pcout <<"Received close event\n";
        }
        if(pAmiPipeWindow != nullptr){
            pAmiPipeWindow->close();
            delete pAmiPipeWindow;
            pAmiPipeWindow = nullptr;
        }

        emit SaveUnsavedConfigs();

        for(auto w: vBulbululators){
            if(w!=nullptr){
                w->close();
            }
        }


    }
    return QWidget::event(event);
}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::initTestConst(){
    dtSpeedCounter = std::chrono::steady_clock::now();
}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::timerEvent(QTimerEvent * event)
{
    bool bSuccess{false};
    bool bSuccessAmi{false};
    auto pdata (queueTrdAnswers.Pop(bSuccess));
    auto pdataAmiPipe (queuePipeAnswers.Pop(bSuccessAmi));
    ImportFinQuotesForm * wnd{nullptr};
    while((bSuccess || bSuccessAmi) && !bWasClose){

        if (bSuccessAmi){
            auto data(*pdataAmiPipe.get());
            switch (data.AnswerType()) {
            case dataAmiPipeAnswer::eAnswerType::Nop:
                break;
            case dataAmiPipeAnswer::eAnswerType::PipeConnected:
                m_TickerLstModel.setTickerState(data.TickerID(),modelTickersList::eTickerState::Connected);
                BulbululatorAddActive(data.TickerID());
                BulbululatorSetState(data.TickerID(),Bulbululator::eTickerState::Connected);
                break;
            case dataAmiPipeAnswer::eAnswerType::PipeDisconnected:
                m_TickerLstModel.setTickerState(data.TickerID(),modelTickersList::eTickerState::NeededPipe);
                BulbululatorRemoveActive(data.TickerID());
                BulbululatorSetState(data.TickerID(),Bulbululator::eTickerState::NeededPipe);
                break;
            case dataAmiPipeAnswer::eAnswerType::PipeHalted:
                m_TickerLstModel.setTickerState(data.TickerID(),modelTickersList::eTickerState::Halted);
                BulbululatorSetState(data.TickerID(),Bulbululator::eTickerState::Halted);
                break;
            case dataAmiPipeAnswer::eAnswerType::PipeOff:
                m_TickerLstModel.setTickerState(data.TickerID(),modelTickersList::eTickerState::Informant);
                BulbululatorRemoveActive(data.TickerID());
                BulbululatorSetState(data.TickerID(),Bulbululator::eTickerState::Informant);
                break;
            case dataAmiPipeAnswer::eAnswerType::ProcessNewComplite:
                break;
            case dataAmiPipeAnswer::eAnswerType::TextMessage:
                emit SendToLog(QString::fromStdString(data.GetTextInfo()));
                break;
            case dataAmiPipeAnswer::eAnswerType::ErrMessage:
                emit SendToErrorLog(QString::fromStdString(data.GetErrString()));
                break;
            case dataAmiPipeAnswer::eAnswerType::testTimeEvent:
                //emit SendToErrorLog(QString::fromStdString(data.GetErrString()));
                if (lcdN){
//                    std::time_t t = data.Time();
//                    std::string s = threadfree_gmtime_time_to_str(&t);
//                    lcdN->display(QString::fromStdString(s));

//                    std::call_once(mainwindow_test_call_once_flag,&MainWindow::initTestConst,this);
//                    milliseconds msCount = std::chrono::steady_clock::now() - dtSpeedCounter;
//                    lcdN->display(QString::number((int)msCount.count()));

                }
                break;
            case dataAmiPipeAnswer::eAnswerType::FastShowEvent:
                slotSendSignalToFastShow(data.TickerID(), data.tBegin, data.tEnd, data.ptrHolder);
                break;
            case dataAmiPipeAnswer::eAnswerType::AskNameAnswer:
                emit PipeNameReceived(data.GetBind(),data.GetPipeName());
                break;
            }
            //////////////////////////////////////////////
            pdataAmiPipe = queuePipeAnswers.Pop(bSuccessAmi);
        }
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //**************************************************************************************************************************************//
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        if (bSuccess){
            auto data(*pdata.get());
            QList<QMdiSubWindow*> lst = ui->mdiArea->subWindowList();
            for(int i = 0; i < lst.size(); ++i){
                if(lst.at(i)->widget() == data.GetParentWnd()) {
                    wnd = qobject_cast<ImportFinQuotesForm *>(lst[i]->widget());
                    if(wnd){
                        break;
                    }
                }
            }
            ////////////////
            switch(data.AnswerType()){
            case dataBuckgroundThreadAnswer::eAnswerType::cloneThread:
                if (thrdPoolLoadFinQuotes.ActiveThreads() < thrdPoolLoadFinQuotes.MaxThreads()){
                    thrdPoolLoadFinQuotes.AddTask([&](){
                        workerLoader::workerDataBaseWork(queueFinQuotesLoad,queueTrdAnswers,stStore);
                        });
                }
                break;
            case dataBuckgroundThreadAnswer::eAnswerType::famImportCurrent:
                if(wnd) wnd->SetProgressBarValue(data.Percent());
                break;
            case dataBuckgroundThreadAnswer::eAnswerType::famImportBegin:
                if(wnd) wnd->slotLoadingHasBegun();
                BulbululatorAddActive(data.TickerID());
                break;
            case dataBuckgroundThreadAnswer::eAnswerType::famImportEnd:
                if(wnd) wnd->slotLoadingHasFinished(data.Successfull(),QString::fromStdString(data.GetErrString()));
                BulbululatorRemoveActive(data.TickerID());
                break;
            case dataBuckgroundThreadAnswer::eAnswerType::LoadActivity:
                BulbululatorShowActivity(data.TickerID());
                break;
            case dataBuckgroundThreadAnswer::eAnswerType::TextInfoMessage:
                if(wnd) wnd->slotTextInfo(QString::fromStdString(data.GetTextInfo()));
                break;
            case dataBuckgroundThreadAnswer::eAnswerType::storagLoadFromStorageGraphBegin:
                {
                    std::stringstream ss;
                    ss << "Load from storage begins [" << data.TickerID()<<"]";
                    emit SendToLog(QString::fromStdString(ss.str()));
                }
                break;
            case dataBuckgroundThreadAnswer::eAnswerType::storagLoadFromStorageGraphEnd:
                {
                    std::stringstream ss;
                    ss << "Load from storage ends [" << data.TickerID()<<"] success: ["<<data.Successfull()<<"]";
                    if(!data.Successfull()){
                        ss <<"TickerID["<< data.TickerID()<<"]\n";
                        ss <<data.GetErrString();
                        emit SendToErrorLog(QString::fromStdString(ss.str()));
                        int n=QMessageBox::critical(0,tr("Critical error"),
                                           QString::fromStdString(data.GetErrString()),
                                           QMessageBox::Ok
                                           );
                        if (n==QMessageBox::Ok){
                            ;
                        }

                    }
                    emit SendToLog(QString::fromStdString(ss.str()));
    //                {
    //                    ThreadFreeCout pcout;
    //                    pcout<<ss.str();
    //                }
                }
                break;
            case dataBuckgroundThreadAnswer::eAnswerType::storagLoadToGraphBegin:
                {
                    std::stringstream ss;
                    ss << "Load to Graph begins [" << data.TickerID()<<"]";
                    emit SendToLog(QString::fromStdString(ss.str()));
                }
                break;
            case dataBuckgroundThreadAnswer::eAnswerType::storagLoadToGraphEnd:
                {
                    std::stringstream ss;
                    ss << "Load to Graph ends [" << data.TickerID()<<"]";
                    if(!data.Successfull()){
                        ss <<"\n"<<data.GetErrString();
                        emit SendToErrorLog(QString::fromStdString(ss.str()));
                    }
                    else{
                        slotSendSignalToInvalidateGraph(data.TickerID(), data.BeginDate(), data.EndDate());

//                        if (data.TickerID() == 9){ // TODO:delete. for test
//                            std::shared_lock lk(mutexMainHolder);
//                            auto hl =  Holders.at(data.TickerID());
//                            size_t tSz = hl->getViewGraphSize(Bar::eInterval::pTick);
//                            ThreadFreeCout pcout;
//                            pcout << "!!!!!!! total graph["<<data.TickerID()<<"] size = {"<<tSz<<"}   !!!!!!!";

//                            unsigned long iV{0};
//                            for (size_t i = 0; i < hl->getViewGraphSize(Bar::eInterval::pMonth); ++i){
//                                iV += hl->getByIndex<Bar>(Bar::eInterval::pMonth,i).Volume();
//                            }
//                            pcout << "    total volume = {"<<iV<<"}   !!!!!!!\n";
//                        }
                    }
                    emit SendToLog(QString::fromStdString(ss.str()));
                }
                break;
            case dataBuckgroundThreadAnswer::eAnswerType::logText:
                emit SendToLog(QString::fromStdString(trim(data.GetTextInfo())));
                break;
            case dataBuckgroundThreadAnswer::eAnswerType::logCriticalError:
                emit SendToLog(QString::fromStdString(trim(data.GetErrString())));
                {
                    emit SendToErrorLog(QString::fromStdString(data.GetErrString()));
                    int n=QMessageBox::critical(0,tr("Critical error"),
                                       QString::fromStdString(data.GetErrString()),
                                       QMessageBox::Ok
                                       );
                    if (n==QMessageBox::Ok){
                        ;
                    }
                }
                break;
            case dataBuckgroundThreadAnswer::testPvBars: // for tests
                //slotTestPvBars(data.pvBars);
                break;
            case dataBuckgroundThreadAnswer::storageOptimisationBegin:
                {
                    std::stringstream ss;
                    ss << "Data optimization begins [" << data.TickerID()<<"]";
                    emit SendToLog(QString::fromStdString(ss.str()));
                }
                break;
            case dataBuckgroundThreadAnswer::storageOptimisationEnd:
                {
                    std::stringstream ss;
                    ss << "Data optimization ends [" << data.TickerID()<<"]";
                    if(!data.Successfull()){
                        ss <<"\n"<<data.GetErrString();
                        emit SendToErrorLog(QString::fromStdString(ss.str()));
                    }
                    emit SendToLog(QString::fromStdString(ss.str()));
                }
                break;
            default:
                break;
            }
            //////////////////////////////////////////////
            CheckActivePipes();
            //////////////////////////////////////////////
            pdata = queueTrdAnswers.Pop(bSuccess);
        }
    }
    //////////////////
    CheckActiveProcesses();
    CheckLastPacketTime();
    CheckActivePipes();
    ListViewActivityTermination();
    CheckUsedMemory();
    //
    emit slotSendSignalToProcessRepaintQueue();
    //
    QWidget::timerEvent(event);
}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::CheckLastPacketTime()
{
    std::time_t tLast = lastTimePacketReceived.load();
    if (tLast > tLocalStoredPacketActivity){
        tLocalStoredPacketActivity = tLast;
        auto str = threadfree_gmtime_time_to_str(&tLocalStoredPacketActivity);
        lcdN->display(QString::fromStdString(str));
    }
}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::CheckActiveProcesses()
{
    int iCount = aActiveProcCounter.load();
    wtCombIndicator->setCurrentLevel(iCount);

}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::CheckUsedMemory(){
    milliseconds tActivityCount  = std::chrono::steady_clock::now() - dtCheckMemoryUsage;
    if (tActivityCount > 1000ms){

        std::size_t iMemory{0};
        std::size_t iTmpSize{0};
        for(const auto &hl:Holders){
            if(!hl.second->GetUsedMemory(iTmpSize)){
                return;//holder iz buzy
            }
            iMemory += iTmpSize;
        }
        if (iMemory !=0){
            double d = iStoredUsedMemory > iMemory ? iStoredUsedMemory - iMemory : iMemory - iStoredUsedMemory;
            if (iStoredUsedMemory == 0 || d/iMemory > 0.01){
                iStoredUsedMemory = iMemory;
                emit UsedMemoryChanged(iStoredUsedMemory,iPhisicalMemory);
            }
        }
        dtCheckMemoryUsage = std::chrono::steady_clock::now();
    }
}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::CheckActivePipes()
{
    milliseconds tActivityCount  = std::chrono::steady_clock::now() - dtCheckPipesActivity;
    if (tActivityCount > 5000ms){
        dtCheckPipesActivity = std::chrono::steady_clock::now();
        //
        dataAmiPipeTask taskAmi(dataAmiPipeTask::eTask_type::RefreshPipeList);

        std::vector<int> vUnconnected;
        std::vector<int> vInformants;
        pipesHolder.CheckPipes(vTickersLst,taskAmi.pipesBindedActive,taskAmi.pipesBindedOff,taskAmi.pipesFree,vUnconnected,vInformants);
        ////////////////////////////
        for (auto &m:mStoredUnconnected) m.second = 0;
        for(const auto &TickerID:vUnconnected){
            m_TickerLstModel.setTickerState(TickerID,modelTickersList::eTickerState::NeededPipe);
            const auto It = mStoredUnconnected.find(TickerID);
            if (It != mStoredUnconnected.end()){
                It->second = 1;
            }
            else{
                mStoredUnconnected[TickerID] = 2;
            }
        }        
        auto It = mStoredUnconnected.begin();
        while ( It != mStoredUnconnected.end()){
            if (It->second == 0){
                BulbululatorSetState(It->first,Bulbululator::eTickerState::Connected);
                BulbululatorRemoveActive(It->first);
                auto ItNext = std::next(It);
                mStoredUnconnected.erase(It);
                It = ItNext;
            }
            else if (It->second == 2){
                BulbululatorAddActive(It->first);
                BulbululatorSetState(It->first,Bulbululator::eTickerState::NeededPipe);
                It++;
            }
            else{
                It++;
            }
        }
        ////////////////////////////
        for(const auto &TickerID:vInformants){
            m_TickerLstModel.setTickerState(TickerID,modelTickersList::eTickerState::Informant);
        }
        //
        queuePipeTasks.Push(taskAmi);
    }
}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::slotAskPipesNames(dataAmiPipeTask::pipes_type &pipesFree)
{
    dataAmiPipeTask task(dataAmiPipeTask::eTask_type::AskPipesNames);
    task.pipesFree = pipesFree;

    queuePipeTasks.Push(task);
}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::slotSendSignalToInvalidateGraph(int TickerID, std::time_t dtDegin, std::time_t dtEnd)
{
    GraphViewForm * wnd{nullptr};
    QList<QMdiSubWindow*> lst = ui->mdiArea->subWindowList();
    for(int i = 0; i < lst.size(); ++i){
        wnd = qobject_cast<GraphViewForm *>(lst[i]->widget());
        if(wnd && wnd->TickerID() == TickerID){
            wnd->slotInvalidateGraph(dtDegin,dtEnd);
        }
    }
}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::slotSendSignalToFastShow(int TickerID, std::time_t /*tBegin*/, std::time_t /*tEnd*/,std::shared_ptr<GraphHolder> ptrHolder)
{
    GraphViewForm * wnd{nullptr};
    QList<QMdiSubWindow*> lst = ui->mdiArea->subWindowList();
    for(int i = 0; i < lst.size(); ++i){
        wnd = qobject_cast<GraphViewForm *>(lst[i]->widget());
        if(wnd && wnd->TickerID() == TickerID){
            wnd->slotFastShowEvent(ptrHolder);
        }
    }
}
//--------------------------------------------------------------------------------------------------------------------------------
///
/// \brief plug for future
///
void MainWindow::slotNotImpl(){};



//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::LoadSettings()
{
    m_settings.beginGroup("Settings");
        m_Language   = m_settings.value("Language","English").toString();

        m_settings.beginGroup("Mainwindow");
            restoreGeometry(m_settings.value("geometry").toByteArray());
            restoreState(m_settings.value("windowState").toByteArray());
            m_sStyleName = m_settings.value("StyleName",style()->objectName()).toString();

            bToolBarOnLoadIsHidden      = m_settings.value("ToolBarIsHidden",false).toBool();
            bTickerBarOnLoadIsHidden    = m_settings.value("TickersBarIsHidden",false).toBool();
            bStatusBarOnLoadIsHidden    = m_settings.value("StatusBarIsHidden",false).toBool();
            bTickerBarButtonsHidden     = m_settings.value("TickerBarButtonsIsHidden",false).toBool();

            swtShowByName->setChecked(m_settings.value("docShowByName",false).toBool());
            swtShowAll->setChecked(m_settings.value("docShowAll",true).toBool());
            swtShowMarkets->setChecked(m_settings.value("docShowMarkets",false).toBool());

            qsStorageDirPath            = m_settings.value("StorageDirPath","").toString();
            bDefaultStoragePath         = m_settings.value("DefaultStoragePath",true).toBool();

            bFillNotAutoloadedTickers   = m_settings.value("FillNotAutoloadedTickers",true).toBool();
            bGrayColorFroNotAutoloadedTickers   = m_settings.value("GrayColorFroNotAutoloadedTickers",false).toBool();
            iDefaultMonthDepth          = m_settings.value("DefaultMonthDepth",1).toInt();

            iCurrentLogfile             = m_settings.value("CurrentLogfile",1).toInt();
            iCurrentErrorLogfile        = m_settings.value("CurrentErrorLogfile",1).toInt();
        m_settings.endGroup();

        m_settings.beginGroup("Configwindow");
            iDefaultTickerMarket    = m_settings.value("DefaultTickerMarket",0).toInt();
            bConfigTickerShowByName = m_settings.value("ConfigTickerShowByName",true).toBool();
            bConfigTickerSortByName = m_settings.value("ConfigTickerSortByName",true).toBool();

            bDefaultSaveLogToFile       = m_settings.value("DefaultSaveLogToFile",false).toBool();
            iDefaultLogSize             = m_settings.value("DefaultLogSize",10).toInt();
            iDefaultLogCount            = m_settings.value("DefaultLogCount",4).toInt();
            bDefaultSaveErrorLogToFile  = m_settings.value("DefaultSaveErrorLogToFile",true).toBool();
            iDefaultErrorLogSize        = m_settings.value("DefaultErrorLogSize",10).toInt();
            iDefaultErrorLogCount       = m_settings.value("DefaultErrorLogCount",4).toInt();
            bDefaultInvertMouseWheel    = m_settings.value("DefaultInvertMouseWheel",true).toBool();
            bDefaultShowHelpButtons     = m_settings.value("DefaultShowHelpButtons",true).toBool();
            bDefaultWhiteBackgtound     = m_settings.value("DefaultWhiteBackgtound",true).toBool();
            bDefaultShowIntroductoryTips= m_settings.value("DefaultShowIntroductoryTips",true).toBool();
        m_settings.endGroup();

        m_settings.beginGroup("ImportFinamForm");
            qsDefaultOpenDir    = m_settings.value("DefaultOpenDir","").toString();
            QString qs          = m_settings.value("ImportDelimiter",",").toString();
            std::string s = qs.toStdString();
            cImportDelimiter    = s.size()>0 ? s[0]: ',';
        m_settings.endGroup();

        m_settings.beginGroup("GraphViewForm");
            bGVLeftScOnLoadIsHidden   = !(m_settings.value("GVLeftSc"  ,true).toBool());
            bGVRightScOnLoadIsHidden  = !(m_settings.value("GVRightSc" ,true).toBool());
            bGVUpperScOnLoadIsHidden  = !(m_settings.value("GVUpperSc" ,true).toBool());
            bGVLowerScOnLoadIsHidden  = !(m_settings.value("GVLowerSc" ,true).toBool());
            bGVVolumeScOnLoadIsHidden = !(m_settings.value("GVVolumeSc",true).toBool());
        m_settings.endGroup();

        m_settings.beginGroup("AmiPipeForm");
            iStoredAmiPipeFormWidth     = m_settings.value("StoredAmiPipeFormWidth",700).toInt();
            iStoredTickerBarWidth       = m_settings.value("StoredTickerBarWidth",100).toInt();

            bAmiPipesFormShown          = m_settings.value("AmiPipesFormShown",false).toBool();
            bAmiPipesNewWndShown        = m_settings.value("bAmiPipesNewWndShown",true).toBool();
            bAmiPipesActiveWndShown     = m_settings.value("bAmiPipesActiveWndShown",true).toBool();

            bAmiPipeShowByNameUnallocated   = m_settings.value("AmiPipeShowByNameUnallocated"   ,false).toBool();
            bAmiPipeShowByNameActive        = m_settings.value("AmiPipeShowByNameActive"        ,false).toBool();
            bAmiPipeShowByNameOff           = m_settings.value("AmiPipeShowByNameOff"           ,false).toBool();
        m_settings.endGroup();

    m_settings.endGroup();
}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::SaveSettings()
{
    //
    m_settings.beginGroup("Settings");

        m_settings.setValue("Language",m_Language);

        m_settings.beginGroup("Mainwindow");
            m_settings.setValue("geometry",this->saveGeometry());
            m_settings.setValue("windowState",this->saveState());
            m_settings.setValue("ToolBarIsHidden",tbrToolBar->isHidden());
            m_settings.setValue("TickersBarIsHidden",ui->dkActiveTickers->isHidden());
            m_settings.setValue("StatusBarIsHidden",ui->statusbar->isHidden());
            m_settings.setValue("TickerBarButtonsIsHidden",ui->widgetTickerButtonBar->isHidden());

            m_settings.setValue("StyleName",m_sStyleName);
            m_settings.setValue("docShowByName",swtShowByName->isChecked());
            m_settings.setValue("docShowAll",swtShowAll->isChecked());
            m_settings.setValue("docShowMarkets",swtShowMarkets->isChecked());

            m_settings.setValue("StorageDirPath", qsStorageDirPath);
            m_settings.setValue("DefaultStoragePath",bDefaultStoragePath);

            m_settings.setValue("FillNotAutoloadedTickers",bFillNotAutoloadedTickers);
            m_settings.setValue("GrayColorFroNotAutoloadedTickers",bGrayColorFroNotAutoloadedTickers);
            m_settings.setValue("DefaultMonthDepth",iDefaultMonthDepth);

            m_settings.setValue("CurrentLogfile",iCurrentLogfile);
            m_settings.setValue("CurrentErrorLogfile",iCurrentErrorLogfile);
        m_settings.endGroup();

        m_settings.beginGroup("Configwindow");
            m_settings.setValue("DefaultTickerMarket",iDefaultTickerMarket);
            m_settings.setValue("ConfigTickerShowByName",bConfigTickerShowByName);
            m_settings.setValue("ConfigTickerSortByName",bConfigTickerSortByName);

            m_settings.setValue("DefaultSaveLogToFile",bDefaultSaveLogToFile);
            m_settings.setValue("DefaultLogSize",iDefaultLogSize);
            m_settings.setValue("DefaultLogCount",iDefaultLogCount);
            m_settings.setValue("DefaultSaveErrorLogToFile",bDefaultSaveErrorLogToFile);
            m_settings.setValue("DefaultErrorLogSize",iDefaultErrorLogSize);
            m_settings.setValue("DefaultErrorLogCount",iDefaultErrorLogCount);
            m_settings.setValue("DefaultInvertMouseWheel",bDefaultInvertMouseWheel);
            m_settings.setValue("DefaultShowHelpButtons",bDefaultShowHelpButtons);
            m_settings.setValue("DefaultWhiteBackgtound",bDefaultWhiteBackgtound);
            m_settings.setValue("DefaultShowIntroductoryTips",bDefaultShowIntroductoryTips);
        m_settings.endGroup();

        m_settings.beginGroup("ImportFinamForm");
            m_settings.setValue("DefaultOpenDir",qsDefaultOpenDir);
            std::string s {" "}; s[0] = cImportDelimiter;
            QString qs = QString::fromStdString(s);
            m_settings.setValue("ImportDelimiter",qs);
        m_settings.endGroup();

        m_settings.beginGroup("GraphViewForm");
            m_settings.setValue("GVLeftSc"  ,pacGVLeftSc->isChecked());
            m_settings.setValue("GVRightSc" ,pacGVRightSc->isChecked());
            m_settings.setValue("GVUpperSc" ,pacGVUpperSc->isChecked());
            m_settings.setValue("GVLowerSc" ,pacGVLowerSc->isChecked());
            m_settings.setValue("GVVolumeSc",pacGVVolumeSc->isChecked());
        m_settings.endGroup();

        m_settings.beginGroup("AmiPipeForm");
            m_settings.setValue("StoredAmiPipeFormWidth",iStoredAmiPipeFormWidth);
            m_settings.setValue("StoredTickerBarWidth",iStoredTickerBarWidth);
            m_settings.setValue("AmiPipesFormShown",bAmiPipesFormShown);
            m_settings.setValue("bAmiPipesNewWndShown",bAmiPipesNewWndShown);
            m_settings.setValue("bAmiPipesActiveWndShown",bAmiPipesActiveWndShown);

            m_settings.setValue("AmiPipeShowByNameUnallocated"  ,bAmiPipeShowByNameUnallocated);
            m_settings.setValue("AmiPipeShowByNameActive"  ,bAmiPipeShowByNameActive);
            m_settings.setValue("AmiPipeShowByNameOff"  ,bAmiPipeShowByNameOff);
        m_settings.endGroup();

    m_settings.endGroup();

    //m_settings.sync();

}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::LoadDataStorage()
{
    try{
        QString appDataFolder;
        if (bDefaultStoragePath){
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
            appDataFolder = qsStorageDirPath;
        }
        stStore.Initialize(appDataFolder.toStdString());

        stStore.LoadMarketConfig(vMarketsLst);
        stStore.LoadTickerConfig(vTickersLst);
    }
    //catch (std::runtime_error &e){
    catch (std::exception &e){
        //
        int n=QMessageBox::critical(0,tr("Error during initialising data dir!"),e.what());
        if (n==QMessageBox::Ok){;}
        //
    }
}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::slotSaveMarketDataStorage()
{
    try{
        stStore.SaveMarketConfig(vMarketsLst);
    }
    catch (std::exception &e){
        //
        int n=QMessageBox::critical(0,tr("Error during saving markets config!"),e.what());
        if (n==QMessageBox::Ok){;}
        //
    }
}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::slotTickerDataStorageUpdate(const QModelIndex & indxL,const QModelIndex & indxR)
{
    try{
        //qDebug()<<"store indexes: {"<<indxL.row()<<":"<<indxR.row()<<"}";
        for (int i = indxL.row(); i <= indxR.row(); ++i){
            slotSaveTickerConigRef(vTickersLst[i]);
        }
    }
    catch (std::exception &e){
        //
        int n=QMessageBox::critical(0,tr("Error during saving markets config!"),e.what());
        if (n==QMessageBox::Ok){;}
        //
    }
}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::slotTickerDataStorageRemove(const Ticker & tT)
{
    try{
        //std::cout<<"remove ticker: {"<<tT.TickerID()<<":"<<tT.TickerSign()<<"}";
        stStore.SaveTickerConfig(tT,Storage::op_type::remove);
    }
    catch (std::exception &e){
        //
        int n=QMessageBox::critical(0,tr("Error during saving markets config!"),e.what());
        if (n==QMessageBox::Ok){;}
        //
    }
}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::slotSaveTickerConig(const Ticker tT, const bool bFull)
{
    slotSaveTickerConigRef(tT, bFull);
}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::slotSaveTickerConigRef(const Ticker & tT, bool bFull)
{
    auto It (std::find_if(vTickersLstEtalon.begin(),vTickersLstEtalon.end(),[&](const auto &t){
                  return t.TickerID() == tT.TickerID();
                }));

    if (It == vTickersLstEtalon.end()
            || (bFull && !It->equalFull(tT))
            || (!bFull && !It->equal(tT))
            ){

        if (It == vTickersLstEtalon.end())  {vTickersLstEtalon.push_back(tT);}
        else                                {*It = tT;}

        stStore.SaveTickerConfig(tT);
    }
}

//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::SaveDataStorage()
{
}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::slotSendTestText()
{
    emit SendToLog("Test text");
    //qDebug()<<style()->objectName();
    //qDebug()<<style();
}
//--------------------------------------------------------------------------------------------------------------------------------
////////
/// \brief Initialisation of menues and panels
///
void MainWindow::InitAction()
{
    this->setWindowTitle(tr("FinLoader"));

    //------------------------------------------------
    QAction * pacNewDoc =new QAction("newdoc");
    pacNewDoc->setText(tr("&Quotes graph"));
    pacNewDoc->setShortcut(QKeySequence(tr("CTRL+N")));
    pacNewDoc->setToolTip(tr("Quotes graph"));
    pacNewDoc->setStatusTip(tr("Quotes graph"));
    pacNewDoc->setWhatsThis(tr("Quotes graph"));
    pacNewDoc->setIcon(QPixmap(":/store/images/sc_newdoc"));
    connect(pacNewDoc,SIGNAL(triggered()),SLOT(slotGraphViewWindow()));
    //------------------------------------------------
    QAction * pacOpen =new QAction("Open");
    pacOpen->setText(tr("&Load history data"));
    pacOpen->setShortcut(QKeySequence(tr("CTRL+O")));
    pacOpen->setToolTip(tr("Load history data"));
    pacOpen->setStatusTip(tr("Load history data"));
    pacOpen->setWhatsThis(tr("Load history data"));
    pacOpen->setIcon(QPixmap(":/store/images/sc_open"));
    connect(pacOpen,SIGNAL(triggered()),SLOT(slotImportFinQuotesWndow()));
    //------------------------------------------------
//    QAction * pacSave =new QAction("Save");
//    pacSave->setText(tr("&Save"));
//    pacSave->setShortcut(QKeySequence(tr("CTRL+S")));
//    pacSave->setToolTip(tr("Save Document"));
//    pacSave->setStatusTip(tr("Save file to disk"));
//    pacSave->setWhatsThis(tr("Save file to disk"));
//    pacSave->setIcon(QPixmap(":/store/images/sc_save"));
    //------------------------------------------------
    QAction * pacLogWnd =new QAction("LogWnd");
    pacLogWnd->setText(tr("Lo&g window"));
    pacLogWnd->setShortcut(QKeySequence(tr("CTRL+L")));
    pacLogWnd->setToolTip(tr("Log window"));
    pacLogWnd->setStatusTip(tr("Log window"));
    pacLogWnd->setWhatsThis(tr("Log window"));
    pacLogWnd->setIcon(QPixmap(":/store/images/sc_move"));
    connect(pacLogWnd,SIGNAL(triggered()),SLOT(slotNewLogWnd()));
    //------------------------------------------------
    QAction * pacErrLogWnd =new QAction("ErrLogWnd");
    pacErrLogWnd->setText(tr("&Error log window"));
    pacErrLogWnd->setShortcut(QKeySequence(tr("CTRL+E")));
    pacErrLogWnd->setToolTip(tr("Error log window"));
    pacErrLogWnd->setStatusTip(tr("Error log window"));
    pacErrLogWnd->setWhatsThis(tr("Error log window"));
    pacErrLogWnd->setIcon(QPixmap(":/store/images/sc_err_log"));
    connect(pacErrLogWnd,SIGNAL(triggered()),SLOT(slotNewErrLogWnd()));
    //------------------------------------------------
    QAction * pacConfig =new QAction("Config");
    pacConfig->setText(tr("Confi&g"));
    pacConfig->setShortcut(QKeySequence(tr("CTRL+G")));
    pacConfig->setToolTip(tr("Config"));
    pacConfig->setStatusTip(tr("Config"));
    pacConfig->setWhatsThis(tr("Config"));
    pacConfig->setIcon(QPixmap(":/store/images/sc_config"));
    connect(pacConfig,SIGNAL(triggered()),SLOT(slotConfigWndow()));
    //------------------------------------------------
    pacAmiPipe =new QAction("AmiPipe");
    pacAmiPipe->setText(tr("&Import from trade sistem"));
    pacAmiPipe->setCheckable(true);
    pacAmiPipe->setChecked(bAmiPipesFormShown);
    pacAmiPipe->setShortcut(QKeySequence(tr("CTRL+I")));
    pacAmiPipe->setToolTip(tr("Import from trade sistem"));
    pacAmiPipe->setStatusTip(tr("Import from trade sistem"));
    pacAmiPipe->setWhatsThis(tr("Import from trade sistem"));
    pacAmiPipe->setIcon(QPixmap(":/store/images/sc_cut"));
    connect(pacAmiPipe,SIGNAL(triggered()),SLOT(slotAmiPipeWndow()));
    //------------------------------------------------
    pacAmiPipeBarNew =new QAction("pacAmiPipeBarNew");
    pacAmiPipeBarNew->setText(tr("&New found pipes"));
    pacAmiPipeBarNew->setCheckable(true);
    pacAmiPipeBarNew->setChecked(bAmiPipesNewWndShown);
    pacAmiPipeBarNew->setToolTip(tr("New found pipes"));
    pacAmiPipeBarNew->setStatusTip(tr("New found pipes"));
    pacAmiPipeBarNew->setWhatsThis(tr("New found pipes"));
    connect(pacAmiPipeBarNew,SIGNAL(triggered()),SLOT(slotAmiPipeWndowNew()));
    //------------------------------------------------
    pacAmiPipeBarActive =new QAction("pacAmiPipeBarActive");
    pacAmiPipeBarActive->setText(tr("&Active pipes"));
    pacAmiPipeBarActive->setCheckable(true);
    pacAmiPipeBarActive->setChecked(bAmiPipesActiveWndShown);
    pacAmiPipeBarActive->setToolTip(tr("Active pipes"));
    pacAmiPipeBarActive->setStatusTip(tr("Active pipes"));
    pacAmiPipeBarActive->setWhatsThis(tr("Active pipes"));
    connect(pacAmiPipeBarActive,SIGNAL(triggered()),SLOT(slotAmiPipeWndowActive()));
    //------------------------------------------------
    pacTickersBar =new QAction(tr("Tickers bar"));
    pacTickersBar->setText(tr("Tickers bar"));
    pacTickersBar->setCheckable(true);
    pacTickersBar->setChecked(!bTickerBarOnLoadIsHidden);
    pacTickersBar->setIcon(QPixmap(":/store/images/sc_save"));
    connect(pacTickersBar,SIGNAL(triggered()),SLOT(slotTickersBarStateChanged()));
    //------------------------------------------------
    pacTickersBarButtonsHide =new QAction(tr("Show tickers bar panel"));
    pacTickersBarButtonsHide->setText(tr("Show tickers bar panel"));
    pacTickersBarButtonsHide->setCheckable(true);
    pacTickersBarButtonsHide->setChecked(!bTickerBarButtonsHidden);
    connect(pacTickersBarButtonsHide,SIGNAL(triggered()),SLOT(slotTickersBarButtonsStateChanged()));
    //------------------------------------------------
    pacStatusBar =new QAction(tr("Status bar"));
    pacStatusBar->setText(tr("Status bar"));
    pacStatusBar->setCheckable(true);
    pacStatusBar->setChecked(!bStatusBarOnLoadIsHidden);
    connect(pacStatusBar,SIGNAL(triggered()),SLOT(slotStatusBarStateChanged()));
    //------------------------------------------------
    pacToolBar =new QAction(tr("Toolbar"));
    pacToolBar->setText(tr("Toolbar"));
    pacToolBar->setCheckable(true);
    pacToolBar->setChecked(!bToolBarOnLoadIsHidden);
    connect(pacToolBar,SIGNAL(triggered()),SLOT(slotToolBarStateChanged()));
    //------------------------------------------------
    pacGVLeftSc =new QAction(tr("Left scale"));
    pacGVLeftSc->setText(tr("Left scale"));
    pacGVLeftSc->setCheckable(true);
    pacGVLeftSc->setChecked(!bGVLeftScOnLoadIsHidden);
    connect(pacGVLeftSc,SIGNAL(triggered()),SLOT(slotGVFramesVisibilityStateChanged()));
    //------------------------------------------------
    pacGVRightSc =new QAction(tr("Right scale"));
    pacGVRightSc->setText(tr("Right scale"));
    pacGVRightSc->setCheckable(true);
    pacGVRightSc->setChecked(!bGVRightScOnLoadIsHidden);
    connect(pacGVRightSc,SIGNAL(triggered()),SLOT(slotGVFramesVisibilityStateChanged()));
    //------------------------------------------------
    pacGVUpperSc =new QAction(tr("Upper horizontal scale"));
    pacGVUpperSc->setText(tr("Upper horizontal scale"));
    pacGVUpperSc->setCheckable(true);
    pacGVUpperSc->setChecked(!bGVUpperScOnLoadIsHidden);
    connect(pacGVUpperSc,SIGNAL(triggered()),SLOT(slotGVFramesVisibilityStateChanged()));
    //------------------------------------------------
    pacGVLowerSc =new QAction(tr("Lower horizontal scale"));
    pacGVLowerSc->setText(tr("Lower horizontal scale"));
    pacGVLowerSc->setCheckable(true);
    pacGVLowerSc->setChecked(!bGVLowerScOnLoadIsHidden);
    connect(pacGVLowerSc,SIGNAL(triggered()),SLOT(slotGVFramesVisibilityStateChanged()));
    //------------------------------------------------
    pacGVVolumeSc =new QAction(tr("Volume scale"));
    pacGVVolumeSc->setText(tr("Volume scale"));
    pacGVVolumeSc->setCheckable(true);
    pacGVVolumeSc->setChecked(!bGVVolumeScOnLoadIsHidden);
    connect(pacGVVolumeSc,SIGNAL(triggered()),SLOT(slotGVFramesVisibilityStateChanged()));
    //------------------------------------------------

    //
    pmnuFile = new QMenu(tr("&File","menu"));
    pmnuFile->addAction(pacNewDoc);
    //pmnuFile->addAction(pacAmiPipe);
    pmnuFile->addAction(pacOpen);
    //pmnuFile->addAction(pacSave);
    pmnuFile->addSeparator();
    pmnuFile->addAction(tr("&Quit"),
                        qApp,
                        SLOT(closeAllWindows()),
                        QKeySequence(tr("CTRL+Q"))
                );
    menuBar()->addMenu(pmnuFile);
    //
    m_mnuWindows = new QMenu(tr("&Windows"));
    menuBar()->addMenu(m_mnuWindows);
    connect(m_mnuWindows,SIGNAL(aboutToShow()),SLOT(slotWindows()));
    menuBar()->addSeparator();
    //
    pmnuTools = new QMenu(tr("&Tools"));
    pmnuTools->addAction(pacLogWnd);
    pmnuTools->addAction(pacErrLogWnd);
    menuBar()->addMenu(pmnuTools);
    //
    pmnuSettings = new QMenu(tr("&Settings"));

    pmnuSettings->addAction(pacToolBar);
    m_mnuAmiPipePanels = new QMenu(tr("&Import from trade sistem panel"));
    pmnuSettings->addMenu(m_mnuAmiPipePanels);

    pmnuSettings->addAction(pacTickersBar);
    pmnuSettings->addAction(pacStatusBar);
    pmnuSettings->addAction(pacTickersBarButtonsHide);
    pmnuSettings->addSeparator();
    m_mnuGraphViewConfig = new QMenu(tr("&Graphics view settings"));
    pmnuSettings->addMenu(m_mnuGraphViewConfig);
    pmnuSettings->addSeparator();
    pmnuSettings->addAction(pacConfig);
    pmnuSettings->addSeparator();

    m_mnuGraphViewConfig->addAction(pacGVLeftSc);
    m_mnuGraphViewConfig->addAction(pacGVRightSc);
    m_mnuGraphViewConfig->addAction(pacGVUpperSc);
    m_mnuGraphViewConfig->addAction(pacGVLowerSc);
    m_mnuGraphViewConfig->addAction(pacGVVolumeSc);

    m_mnuAmiPipePanels->addAction(pacAmiPipe);
    m_mnuAmiPipePanels->addAction(pacAmiPipeBarNew);
    m_mnuAmiPipePanels->addAction(pacAmiPipeBarActive);

    m_mnuStyles = new QMenu(tr("St&yles"));
    pmnuSettings->addMenu(m_mnuStyles);
    connect(m_mnuStyles,SIGNAL(aboutToShow()),SLOT(slotStyles()));
    m_mnuLangs = new QMenu(tr("&Language"));
    pmnuSettings->addMenu(m_mnuLangs);
    connect(m_mnuLangs,SIGNAL(aboutToShow()),SLOT(slotLanguages()));

    menuBar()->addMenu(pmnuSettings);
    //
    pmnuHelp = new QMenu(tr("&Help"));
    pmnuHelp->addAction(tr("&About"),this,SLOT(slotAbout()),Qt::Key_F1);
    menuBar()->addMenu(pmnuHelp);

    //------------------------------------------------
    m_psigmapper = new QSignalMapper(this);
    connect(m_psigmapper,SIGNAL(mapped(QWidget*)),this,SLOT(slotSetActiveSubWindow(QWidget*)));
    m_psigmapperStyle = new QSignalMapper(this);
    connect(m_psigmapperStyle,SIGNAL(mapped(QString)),this,SLOT(slotSetActiveStyle(QString)));
    m_psigmapperLang = new QSignalMapper(this);
    connect(m_psigmapperLang,SIGNAL(mapped(QString)),this,SLOT(slotSetActiveLang(QString)));
    //------------------------------------------------
    //------------------------------------------------
    tbrToolBar =new QToolBar("Toolbar");
    tbrToolBar->setObjectName("Toolbar");
    tbrToolBar->addAction(pacNewDoc);
    tbrToolBar->addAction(pacTickersBar);
    tbrToolBar->addAction(pacAmiPipe);
    tbrToolBar->addAction(pacOpen);
    //tbrToolBar->addAction(pacSave);
    tbrToolBar->addAction(pacConfig);
    tbrToolBar->addAction(pacLogWnd);
    tbrToolBar->addAction(pacErrLogWnd);

        this->addToolBar(tbrToolBar);
        if (bToolBarOnLoadIsHidden){
            tbrToolBar->hide();
        }
        //------------------------------------------------
        if (bStatusBarOnLoadIsHidden)
            ui->statusbar->hide();
        if (bTickerBarButtonsHidden)
            ui->widgetTickerButtonBar->hide();




        //------------------------------------------------
        connect(ui->lstView,SIGNAL(doubleClicked(const QModelIndex&)),this,SLOT(slotSetSelectedTicker(const  QModelIndex&)));    
}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::BulbululatorShowActivity   (int TickerID)
{
    bool bFound = false;
    size_t i = 0;
    while (i < vBulbululators.size()) {
        if (vBulbululators[i] == nullptr){
            vBulbululators.erase(std::next(vBulbululators.begin(),i));
            continue;
        }
        if(vBulbululators[i]->TickerID() == TickerID){
            bFound = true;
            break;
        }
        i++;
    }
    if (bFound){
        vBulbululators[i]->Bubble();
    }
    //----------------------
    ListViewShowActivity(TickerID);
}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::BulbululatorSetState(int TickerID,Bulbululator::eTickerState State)
{
    size_t i = 0;
    while (i < vBulbululators.size()) {
        if (vBulbululators[i] == nullptr){
            vBulbululators.erase(std::next(vBulbululators.begin(),i));
            continue;
        }
        if(vBulbululators[i]->TickerID() == TickerID){
            vBulbululators[i]->setState(State);
            break;
        }
        i++;
    }
}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::BulbululatorRemoveActive   (int TickerID)
{
    bool bFound = false;
    size_t i = 0;
    int iCurrInstanses{0};
    while (i < vBulbululators.size()) {
        if (vBulbululators[i] == nullptr){
            vBulbululators.erase(std::next(vBulbululators.begin(),i));
            continue;
        }
        if(vBulbululators[i]->TickerID() == TickerID){
            bFound = true;
            iCurrInstanses = vBulbululators[i]->RemoveInstance();
            break;
        }
        i++;
    }
    if (bFound && iCurrInstanses == 0){
        vBulbululators[i]->close();
        disconnect(vBulbululators[i],SIGNAL(DoubleClicked(const int)),this,SLOT(slotSetSelectedTicker(const  int)));
        ui->statusbar->removeWidget(vBulbululators[i]);
        //statusBarTickers->removeWidget(vBulbululators[i]);
        vBulbululators.erase(std::next(vBulbululators.begin(),i));
    }

}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::BulbululatorAddActive      (int TickerID)
{
    bool bFound{false};
    QString str;
    bool bBulbulator{false};

    for(const auto& t:vTickersLst){
        if(t.TickerID() == TickerID){
            str = QString::fromStdString(t.TickerSign());
            bFound = true;
            bBulbulator = t.Bulbululator();
            break;
        }
    }
    if (!bFound || !bBulbulator) return;

    bFound = false;
    size_t i = 0;
    while (i < vBulbululators.size()) {
        if (vBulbululators[i] == nullptr){
            vBulbululators.erase(std::next(vBulbululators.begin(),i));
            continue;
        }
        if(vBulbululators[i]->TickerID() == TickerID){
            bFound = true;
            vBulbululators[i]->AddInstance();
            break;
        }
        i++;
    }
    if (!bFound){
        Bulbululator * blbl = new Bulbululator();
        connect(blbl,SIGNAL(DoubleClicked(const int)),this,SLOT(slotSetSelectedTicker(const  int)));

        if(blbl){

            for(auto const & b:vBulbululators){
                ui->statusbar->removeWidget(b);
                //statusBarTickers->removeWidget(b);
            }

            blbl->SetText(str);
            blbl->SetTickerID(TickerID);
            vBulbululators.push_back(blbl);

            std::sort(vBulbululators.begin(),vBulbululators.end(),[]( Bulbululator * const l,Bulbululator * const r){
                return (QString::localeAwareCompare(l->Text(),r->Text()) < 0);
                });

            for(auto & b:vBulbululators){
                ui->statusbar->addWidget(b);
                //statusBarTickers->addWidget(b);
                b->show();
            }
            //ui->statusbar->addWidget(blbl);
            vBulbululators.back()->Bubble();
        }
    }
}
//--------------------------------------------------------------------------------------------------------------------------------
///
/// \brief create NewDoc Window (for test purpouse)
///
void MainWindow::slotNewDoc()
{
//    QWidget *pdoc=new QWidget;
//    pdoc->setAttribute(Qt::WA_DeleteOnClose);
//    pdoc->setWindowTitle(tr("Unnamed document"));
//    pdoc->setWindowIcon(QPixmap(":/store/images/sc_newdoc"));


//    QGridLayout *lt=new QGridLayout();
//    QLabel *lbl=new QLabel(tr("Document"));
//    QPushButton * btn1=new QPushButton(tr("Push it"));
//    QPushButton * btn2=new QPushButton(tr("Doun't"));
//    QComboBox * cbx=new QComboBox();

//    connect(btn1,SIGNAL(clicked()),this,SLOT(slotSendTestText()));

//    cbx->addItem(tr("First elem"));
//    cbx->addItem(tr("Second elem"));
//    lt->addWidget(lbl);
//    lt->addWidget(btn1);
//    lt->addWidget(btn2);
//    lt->addWidget(cbx);
//    pdoc->setLayout(lt);


//    QListView *lw8=new QListView();
//    lw8->setModel(&m_MarketLstModel);
//    lt->addWidget(lw8);

//    ui->mdiArea->addSubWindow(pdoc);
//    pdoc->show();
}

//--------------------------------------------------------------------------------------------------------------------------------
///
/// \brief Making menu for teg "Windows"
///
void MainWindow::slotWindows ()
{
    m_mnuWindows->clear();
    //
    QAction *pac;
    //
    pac = m_mnuWindows->addAction(tr("&Cascade"),ui->mdiArea,SLOT(cascadeSubWindows()));
    pac->setEnabled(!ui->mdiArea->subWindowList().isEmpty());
    //
    pac = m_mnuWindows->addAction(tr("&Tile"),ui->mdiArea,SLOT(tileSubWindows()));
    pac->setEnabled(!ui->mdiArea->subWindowList().isEmpty());
    //
    m_mnuWindows->addSeparator();
    //////////////
    QList<QMdiSubWindow*> lst = ui->mdiArea->subWindowList();

    for(int i = 0; i < lst.size(); ++i){
         pac = m_mnuWindows->addAction(lst.at(i)->windowTitle());
         pac->setCheckable(true);
         pac->setChecked(ui->mdiArea->activeSubWindow() == lst.at(i));
        //
         connect(pac,SIGNAL(triggered()),m_psigmapper,SLOT(map()));
         m_psigmapper->setMapping(pac,lst.at(i));
    }
};

//--------------------------------------------------------------------------------------------------------------------------------
////
/// \brief process changing active sub window (through menu)
/// \param pwg
///
void MainWindow::slotSetActiveSubWindow (QWidget* pwg)
{
    if(pwg){
        ui->mdiArea->setActiveSubWindow(qobject_cast<QMdiSubWindow*>(pwg));
    }
};

//--------------------------------------------------------------------------------------------------------------------------------
/// \brief About Window
///
void MainWindow::slotAbout   ()
{
    QMessageBox::about(0,tr("About"),"FinLoader v.0.0.1");

};
//--------------------------------------------------------------------------------------------------------------------------------
///
/// \brief fill menu with styles
///
void MainWindow::slotStyles     ()
{
    QAction * pac;
    m_mnuStyles->clear();
    lstStyles.clear();
    foreach(QString str,QStyleFactory::keys()){
        lstStyles.append(str);
    }
    lstStyles.append("BlackStyle");

    for(auto& s:lstStyles){
        pac = m_mnuStyles->addAction(s);
        connect(pac,SIGNAL(triggered()),m_psigmapperStyle,SLOT(map()));
        m_psigmapperStyle->setMapping(pac,s);
    }

};
//--------------------------------------------------------------------------------------------------------------------------------
////
/// \brief process style changing
/// \param styleName
///

void MainWindow::slotSetActiveStyle     (QString s)
{
    QString sOldStyle = m_sStyleName;
    m_sStyleName = s;
    if(s == "BlackStyle"){
        QFile fl(":/store/blackstyle.css");
        fl.open(QFile::ReadOnly);
        QString strCSS=QLatin1String(fl.readAll());
        qApp->setStyleSheet(strCSS);
    }
    else{
        /////////////////////////////////////////
        ///
        QPalette  pal  =  this->palette();
//        pal.setBrush(QPalette::Button, QBrush(Qt::red, Qt::Dense1Pattern));
//        pal.setBrush(QPalette::Base, QBrush(Qt::red, Qt::Dense1Pattern));
//!!!!!       pal.setBrush(QPalette::ColorRole::Window, QBrush(Qt::blue, Qt::BrushStyle::SolidPattern));
//        pal.setColor(QPalette: :ButtonText, Qt: :Ыuе);
//        pal.setColor(QPalette::Text,  Qt::magenta);
        //pal.setColor(QPalette::Active, QPalette::Base, Qt::white);
        //pal.setColor(QPalette::ColorRole::Window, Qt::white);
//        this->setPalette(pal);

        /////////////////////////////////////////
        QStyle * st=QStyleFactory::create(s);
        //qApp->setStyle(st);
        QApplication::setStyle(st);
        //
        // becouse stylesheet cannot be redone, reload
        if (sOldStyle == "BlackStyle"){
            QMessageBox *msg = new QMessageBox(QMessageBox::Question,
                                               tr("style change"),
                                               tr("For style change app needed to be reloaded. Do it?"),
                                               QMessageBox::Yes|QMessageBox::No
                        );
            int n = msg->exec();
            delete msg;
            if(n == QMessageBox::Yes){
                //qDebug()<<"reboot!!!";
                SaveSettings();
                qApp->quit();
                QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
            }
        }
    }
}
//--------------------------------------------------------------------------------------------------------------------------------
////
/// \brief create new log window
///
void MainWindow::slotNewLogWnd()
{
    QWidget *pdoc=new QWidget;
    pdoc->setAttribute(Qt::WA_DeleteOnClose);
    pdoc->setWindowTitle(tr("Log window"));
    pdoc->setWindowIcon(QPixmap(":/store/images/sc_move"));


    QGridLayout *lt=new QGridLayout();
    QTextEdit * ed=new QTextEdit();

    lt->addWidget(ed);
    lt->setMargin(1);
    pdoc->setLayout(lt);

    connect(this,SIGNAL(SendToLog(QString)),ed,SLOT(append(QString)));

    ui->mdiArea->addSubWindow(pdoc);
    pdoc->show();

}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::slotNewErrLogWnd()
{
    QWidget *pdoc=new QWidget;
    pdoc->setAttribute(Qt::WA_DeleteOnClose);
    pdoc->setWindowTitle(tr("Error log window"));
    pdoc->setWindowIcon(QPixmap(":/store/images/sc_err_log"));


    QGridLayout *lt=new QGridLayout();
    QTextEdit * ed=new QTextEdit();

    lt->addWidget(ed);
    lt->setMargin(1);
    pdoc->setLayout(lt);

    connect(this,SIGNAL(SendToErrorLog(QString)),ed,SLOT(append(QString)));

    ui->mdiArea->addSubWindow(pdoc);
    pdoc->show();
}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::slotLanguages  ()
{
    QAction * pac;
    QString sL;
    m_mnuLangs->clear();

    {
        sL = "English";
        pac = m_mnuLangs->addAction(sL);
        connect(pac,SIGNAL(triggered()),m_psigmapperLang,SLOT(map()));
        m_psigmapperLang->setMapping(pac,sL);
        //
        sL = "Русский";
        pac = m_mnuLangs->addAction(sL);
        connect(pac,SIGNAL(triggered()),m_psigmapperLang,SLOT(map()));
        m_psigmapperLang->setMapping(pac,sL);
    }

};
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::slotSetActiveLang      (QString sL)
{
    //////////////////////////////////////////////////////////////////////////////////////////
    /// \brief do it first, becouse need old translation (befour load new language to QTranslator)
    ///
    QMessageBox *msg = new QMessageBox(QMessageBox::Question,
                                   tr("Language change"),
                                   tr("For language change app needed to be reloaded. Do it?"),
                                   QMessageBox::Yes|QMessageBox::No
            );
    //////////////////////////////////////////////////////////////////////////////////////////
    /// \brief do lang exchange, anyway has it set befour or not, for usage purpouse.
    ///
    QString sOldLang = m_Language;
    if(sL == "Русский"){
        m_Language = sL;
        m_translator.load(":/store/FinLoader_ru_RU.qm");
        qApp->installTranslator(&m_translator);
    }
    else{
        m_Language = "English";
        m_translator.load(":/store/FinLoader_en_US.qm");
        qApp->installTranslator(&m_translator);
    }

    // do reload, if needed
    if (sOldLang.size()>0 && sOldLang != sL){
        int n = msg->exec();
        delete msg;

        if(n == QMessageBox::Yes){
            //qDebug()<<"reboot!!!";
            SaveSettings();
            qApp->quit();
            QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
        }
    }
    else{
        delete msg;
    }
}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::slotGraphViewWindow()
{
    auto qml(ui->lstView->selectionModel());
    auto lst (qml->selectedIndexes());
    //if(lst.count() > 0 && lst[0].isValid()){

    bool bWas{false};
    for (auto item:lst){
        const Ticker &t = proxyTickerModel.getTicker(item);
        slotSetSelectedTicker(t.TickerID());
        bWas = true;
    }
    if (!bWas){
        //TODO: do non blocking floating warning
        QMessageBox::warning(0,tr("Warning"),
                                   tr("Please, select a ticker in the box at the bottom left to continue!"),
                                   QMessageBox::Ok
                                   );
    }
    //ui->lstView->clearSelection();
    //qml->clearSelection();
}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::slotConfigWndow()
{
    ConfigWindow *pdoc=new ConfigWindow(&m_MarketLstModel,iDefaultTickerMarket,
                                        &m_TickerLstModel,bConfigTickerShowByName,bConfigTickerSortByName,
                                        bDefaultStoragePath,qsStorageDirPath,stStore,
                                        bFillNotAutoloadedTickers,
                                        bGrayColorFroNotAutoloadedTickers,
                                        iDefaultMonthDepth,
                                        bDefaultSaveLogToFile,
                                        iDefaultLogSize,
                                        iDefaultLogCount,
                                        bDefaultSaveErrorLogToFile,
                                        iDefaultErrorLogSize,
                                        iDefaultErrorLogCount,
                                        bDefaultInvertMouseWheel,
                                        bDefaultShowHelpButtons,
                                        bDefaultWhiteBackgtound,
                                        bDefaultShowIntroductoryTips
                                        );
    pdoc->setAttribute(Qt::WA_DeleteOnClose);
    pdoc->setWindowTitle(tr("Config"));
    pdoc->setWindowIcon(QPixmap(":/store/images/sc_config"));

   ui->mdiArea->addSubWindow(pdoc);

    connect(pdoc,SIGNAL(SendToMainLog(QString)),this,SIGNAL(SendToLog(QString)));
    connect(pdoc,SIGNAL(NeedSaveMarketsChanges()),this,SLOT(slotSaveMarketDataStorage()));

    connect(pdoc,SIGNAL(NeedSaveDefaultTickerMarket(int)),this,SLOT(slotStoreDefaultTickerMarket(int)));
    connect(pdoc,SIGNAL(NeedSaveShowByNames(bool)),this,SLOT(slotStoreConfigTickerShowByName(bool)));
    connect(pdoc,SIGNAL(NeedSaveSortByNames(bool)),this,SLOT(slotStoreConfigTickerSortByName(bool)));


    connect(this,SIGNAL(SaveUnsavedConfigs()),pdoc,SLOT(slotBtnSaveMarketClicked()));
    connect(this,SIGNAL(SaveUnsavedConfigs()),pdoc,SLOT(slotBtnSaveTickerClicked()));

    connect(pdoc,SIGNAL(NeedChangeDefaultPath(bool,QString)),this,SLOT(slotSaveNewDefaultPath(bool,QString)));
    connect(pdoc,SIGNAL(NeedSaveGeneralOptions(bool,bool,int, bool,int,int,bool,int,int,bool,bool,bool,bool)),
              this,SLOT(slotSaveGeneralOptions(bool,bool,int, bool,int,int,bool,int,int,bool,bool,bool,bool)));

    pdoc->show();
}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::slotImportFinQuotesWndow ()
{

    ImportFinQuotesForm *pdoc=new ImportFinQuotesForm (&m_MarketLstModel,iDefaultTickerMarket,&m_TickerLstModel/*,bConfigTickerShowByName,bConfigTickerSortByName*/);
    pdoc->setAttribute(Qt::WA_DeleteOnClose);
    pdoc->setWindowTitle(tr("Import"));
    pdoc->setWindowIcon(QPixmap(":/store/images/sc_open"));
    pdoc->SetDefaultOpenDir(qsDefaultOpenDir);
    pdoc->SetDelimiter(cImportDelimiter);


    ui->mdiArea->addSubWindow(pdoc);
    //
    connect(pdoc,SIGNAL(OpenImportFilePathChanged(QString &)),this,SLOT(slotDefaultOpenDirChanged(QString &)));
    connect(pdoc,SIGNAL(DelimiterHasChanged(char)),this,SLOT(slotImportDelimiterChanged(char)));
    connect(pdoc,SIGNAL(NeedSaveDefaultTickerMarket(int)),this,SLOT(slotStoreDefaultTickerMarket(int)));

    connect(pdoc,SIGNAL(NeedParseImportFinQuotesFile(dataFinLoadTask &)),this,SLOT(slotParseImportFinQuotesFile(dataFinLoadTask &)));
    connect(pdoc,SIGNAL(NeedToStopLoadings()),this,SLOT(slotStopFinQuotesLoadings()));

    connect(this,SIGNAL(ShowHelpButtonsChanged(bool)),pdoc,SLOT(slotShowHelpButtonsChanged(bool)));

    emit ShowHelpButtonsChanged(bDefaultShowHelpButtons);

    //
    pdoc->show();
}

//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::slotParseImportFinQuotesFile(dataFinLoadTask & dtTask)
{
    std::shared_lock lk(mutexMainHolder);
    if (Holders.find(dtTask.TickerID) == Holders.end()){
        lk.unlock();
        std::unique_lock lk2(mutexMainHolder);
        Holders[dtTask.TickerID] = std::make_shared<GraphHolder>(GraphHolder{dtTask.TickerID});
        lk2.unlock();
        lk.lock();
    }

    dtTask.SetStore(&stStore);
    dtTask.holder = Holders[dtTask.TickerID];
    lk.unlock();

    queueFinQuotesLoad.Push(dataFinLoadTask(dtTask));

    thrdPoolLoadFinQuotes.AddTask([&](){
        workerLoader::workerDataBaseWork(queueFinQuotesLoad,queueTrdAnswers,stStore);
        });
    ///
}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::slotStopFinQuotesLoadings()
{
    queueFinQuotesLoad.clear();
    thrdPoolLoadFinQuotes.Interrupt();
}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::slotDocbarShowAllChanged   (int /*iChange*/)
{
    if (swtShowAll)
        proxyTickerModel.setFilterByActive(!swtShowAll->isChecked());
}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::slotDocbarShowMarketChanged   (int)
{
    if (!swtShowMarkets || !swtShowByName){
        return;
    }

    if (!swtShowMarkets->isChecked()){
        if (swtShowByName->isChecked())
            ui->lstView->setModelColumn(0);
        else
            ui->lstView->setModelColumn(2);
    }
    else{
        if (swtShowByName->isChecked())
            ui->lstView->setModelColumn(3);
        else
            ui->lstView->setModelColumn(4);
    }
    proxyTickerModel.invalidate();

}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::slotDocbarShowNyNameChanged(int i)
{
    slotDocbarShowMarketChanged(i);
}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::InitDockBar()
{
    ////////////////////////////////////////////////////////////////////////
    /// prepare content for tickers bar doc

    if (swtShowAll){
        proxyTickerModel.setFilterByActive(!swtShowAll->isChecked());
    }

    proxyTickerModel.setSourceModel(&m_TickerLstModel);
    proxyTickerModel.sort(2);
    ui->lstView->setModel(&proxyTickerModel);

    slotDocbarShowMarketChanged(0);
    ////////////////////////////////////////////////////////////////////////
    /// prepare dockbar objects

    wtAmiDockbar = new QDockWidget();
    wtAmiDockbar->setObjectName("wtAmiDockbar");
    wtAmiDockbar ->setTitleBarWidget(new QWidget());
    addDockWidget(Qt::LeftDockWidgetArea, wtAmiDockbar);
    splitDockWidget(wtAmiDockbar,ui->dkActiveTickers,Qt::Horizontal);
    ////////////////////////////////////////////////////////////////////////
    /// prepare content for ami pipe docbar doc

    pAmiPipeWindow=new AmiPipesForm (&m_MarketLstModel,iDefaultTickerMarket,&m_TickerLstModel,pipesHolder,vTickersLst,
                                              bAmiPipeShowByNameUnallocated, bAmiPipeShowByNameActive,bAmiPipeShowByNameOff,
                                              bAmiPipesNewWndShown, bAmiPipesActiveWndShown);

    pAmiPipeWindow->setWindowTitle(tr("Import from trade sistems"));
    pAmiPipeWindow->setWindowIcon(QPixmap(":/store/images/sc_cut"));
    connect(pAmiPipeWindow,SIGNAL(NeedSaveDefaultTickerMarket(int)),this,SLOT(slotStoreDefaultTickerMarket(int)));

    connect(pAmiPipeWindow,SIGNAL(SendToMainLog(QString)),this,SIGNAL(SendToLog(QString)));
    connect(pAmiPipeWindow,SIGNAL(WasCloseEvent()),this,SLOT(slotAmiPipeFormWasClosed()));

    connect(pAmiPipeWindow,SIGNAL(NeedSaveShowByNamesUnallocated(bool)),this,SLOT(slotAmiPipeSaveShowByNamesUnallocated(bool)));
    connect(pAmiPipeWindow,SIGNAL(NeedSaveShowByNamesActive(bool)),this,SLOT(slotAmiPipeSaveShowByNamesActive(bool)));
    connect(pAmiPipeWindow,SIGNAL(NeedSaveShowByNamesOff(bool)),this,SLOT(slotAmiPipeSaveShowByNamesOff(bool)));

    connect(pAmiPipeWindow,SIGNAL(WidthWasChanged(int)),this,SLOT(slotAmiPipeWidthWasChanged(int)));

    connect(pAmiPipeWindow,SIGNAL(NewWndStateChanged(int)),this,SLOT(slotAmiPipeNewWndStateChanged(int)));
    connect(pAmiPipeWindow,SIGNAL(ActiveWndStateChanged(int)),this,SLOT(slotAmiPipeActiveWndStateChanged(int)));

    connect(pAmiPipeWindow,SIGNAL(buttonHideClicked()),this,SLOT(slotAmiPipeHideClicked()));

    connect(this,SIGNAL(AmiPipeInternalPanelsStateChanged(bool, bool)),pAmiPipeWindow,SLOT(slotInternalPanelsStateChanged(bool, bool)));

    connect(pAmiPipeWindow,SIGNAL(AskPipesNames(dataAmiPipeTask::pipes_type &)),this,SLOT(slotAskPipesNames(dataAmiPipeTask::pipes_type &)));

    connect(this,SIGNAL(PipeNameReceived(std::string,std::string)),pAmiPipeWindow,SLOT(slotPipeNameReceived(std::string,std::string)));

    ///////////////////////////////////////////////////////////////////////

    wtAmiDockbar->setWidget(pAmiPipeWindow);
    ////////////////////////////////////////////////////////////////////////
    /// resizing and hiding/showing

    ResizingLeftToolBars();

}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::ResizingLeftToolBars()
{
    if (bAmiPipesFormShown){
        if (wtAmiDockbar->isHidden()) wtAmiDockbar->show();

        pAmiPipeWindow->setFixedWidth(iStoredAmiPipeFormWidth);

        resizeDocks({wtAmiDockbar,ui->dkActiveTickers},{iStoredAmiPipeFormWidth,iStoredTickerBarWidth},Qt::Orientation::Horizontal);

    }
    else{
        if (!wtAmiDockbar->isHidden()) wtAmiDockbar->hide();
        resizeDocks({ui->dkActiveTickers},{iStoredTickerBarWidth},Qt::Orientation::Horizontal);
    }
}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::slotAmiPipeWidthWasChanged(int iW)
{
    iStoredAmiPipeFormWidth = iW;
    ResizingLeftToolBars();
}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::slotToolBarStateChanged()
{
    if(pacToolBar && pacToolBar->isChecked())
        tbrToolBar->show();
    else
        tbrToolBar->hide();
}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::slotStatusBarStateChanged()
{
    if(pacStatusBar && pacStatusBar->isChecked()){
        ui->statusbar->show();
    }
    else{
        ui->statusbar->hide();
    }
}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::slotTickersBarStateChanged()
{
    if(pacTickersBar && pacTickersBar->isChecked()){
        ui->dkActiveTickers->show();
    }
    else{
        ui->dkActiveTickers->hide();
    }
}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::slotTickersBarButtonsStateChanged()
{
    if(pacTickersBarButtonsHide && pacTickersBarButtonsHide->isChecked()){
        ui->widgetTickerButtonBar->show();
    }
    else{
        ui->widgetTickerButtonBar->hide();
    }
}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::ListViewShowActivity(int TickerID){
    std::chrono::time_point<std::chrono::steady_clock> tNow = std::chrono::steady_clock::now();

    auto It (mBlinkedState.find(TickerID));
    if (It  == mBlinkedState.end() ||
            (milliseconds(tNow - It->second.second)).count() > 200
            ){
        mBlinkedState[TickerID] = std::make_pair(true, tNow);
        m_TickerLstModel.blinkTicker(TickerID);
        qActivityQueue.push(std::make_pair(TickerID,tNow));
    }
}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::ListViewActivityTermination()
{
    if (!qActivityQueue.empty()){
        std::chrono::time_point dtNow(std::chrono::steady_clock::now());

        if (milliseconds (dtNow - qActivityQueue.front().second).count() >100 ){
            //
            mBlinkedState[qActivityQueue.front().first].first = false;
            m_TickerLstModel.blinkTicker(qActivityQueue.front().first);
            /////
            qActivityQueue.pop();
        }
    }
}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::slotSetSelectedTicker(const  QModelIndex& indx)
{
    const Ticker &t = proxyTickerModel.getTicker(indx);
    slotSetSelectedTicker(t.TickerID());
}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::slotSetSelectedTicker(const  int iTickerID)
{
    GraphViewForm * wnd;
    bool bFound{false};
    QList<QMdiSubWindow*> lst = ui->mdiArea->subWindowList();
    for(int i = 0; i < lst.size(); ++i){
        if(lst.at(i)->widget()->objectName() == "GraphViewForm")
        {
            wnd = qobject_cast<GraphViewForm *>(lst[i]->widget());
            if(wnd && wnd->TickerID() == iTickerID){
                bFound = true;
                break;
            }
        }
    }
    if (!bFound){
        std::tuple<bool,bool,bool,bool,bool> tp{
                    pacGVLeftSc->isChecked(),
                    pacGVRightSc->isChecked(),
                    pacGVUpperSc->isChecked(),
                    pacGVLowerSc->isChecked(),
                    pacGVVolumeSc->isChecked()};

        std::shared_lock lk(mutexMainHolder);
        if (Holders.find(iTickerID) == Holders.end()){
            lk.unlock();
            std::unique_lock lk2(mutexMainHolder);
            Holders[iTickerID] = std::make_shared<GraphHolder>(GraphHolder{iTickerID});
            lk2.unlock();
            lk.lock();
        }

        slotLoadGraph(iTickerID);

        GraphViewForm *pdoc=new GraphViewForm(iTickerID,vTickersLst,Holders[iTickerID]);
        lk.unlock();
        pdoc->setAttribute(Qt::WA_DeleteOnClose);
        pdoc->setWindowIcon(QPixmap(":/store/images/sc_newdoc"));

        ui->mdiArea->addSubWindow(pdoc);

        connect(pdoc,SIGNAL(NeedLoadGraph(const  int,const std::time_t,const std::time_t)),
                  this,SLOT(slotLoadGraph(const  int,const std::time_t,const std::time_t)));

        connect(pdoc,SIGNAL(SendToLog(QString)),this,SIGNAL(SendToLog(QString)));
        connect(this,SIGNAL(slotSendSignalToProcessRepaintQueue()),pdoc,SLOT(slotProcessRepaintQueue()));
        connect(pdoc,SIGNAL(NeedSaveTickerConig(const Ticker, const bool)),this,SLOT(slotSaveTickerConig(const Ticker, const bool)));

        connect(this,SIGNAL(SaveUnsavedConfigs()),pdoc,SLOT(slotSaveUnsavedConfigs()));

        connect(this,SIGNAL(UsedMemoryChanged(size_t,size_t)),pdoc,SLOT(slotUsedMemoryChanged(size_t,size_t)));

        connect(this,SIGNAL(InvertMouseWheelChanged(bool)),pdoc,SLOT(slotInvertMouseWheelChanged(bool)));
        connect(this,SIGNAL(ShowHelpButtonsChanged(bool)),pdoc,SLOT(slotShowHelpButtonsChanged(bool)));

        emit InvertMouseWheelChanged(bDefaultInvertMouseWheel);
        emit ShowHelpButtonsChanged(bDefaultShowHelpButtons);

        pdoc->setFramesVisibility(tp);
        pdoc->show();

        emit UsedMemoryChanged(iStoredUsedMemory,iPhisicalMemory);
    }
    else{
        wnd->setFocus();
    }
}
//--------------------------------------------------------------------------------------------------------------------------------

void MainWindow::slotLoadGraph(const  int iTickerID)
{
    std::shared_lock lk(mutexMainHolder);
    if (Holders.find(iTickerID) == Holders.end()){
        lk.unlock();
        std::unique_lock lk2(mutexMainHolder);
        Holders[iTickerID] = std::make_shared<GraphHolder>(GraphHolder{iTickerID});
        lk2.unlock();
        lk.lock();
    }
    if (Holders[iTickerID]->getViewGraphSize(Bar::eInterval::pTick) == 0){
        if (iDefaultMonthDepth < 0 ) iDefaultMonthDepth = 0;
        if (iDefaultMonthDepth > 99 ) iDefaultMonthDepth = 99;

        std::time_t tNow =std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::time_t tBegin = Storage::dateAddMonth(tNow, (-iDefaultMonthDepth));

        slotLoadGraph(iTickerID,tBegin,tNow);
    }
}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::slotLoadGraph(const  int iTickerID, const std::time_t tBegin, const std::time_t tEnd)
{
    std::shared_lock lk(mutexMainHolder);
    if (Holders.find(iTickerID) == Holders.end()){
        lk.unlock();
        std::unique_lock lk2(mutexMainHolder);
        Holders[iTickerID] = std::make_shared<GraphHolder>(GraphHolder{iTickerID});
        lk2.unlock();
        lk.lock();
    }

    dataFinLoadTask dataTask;
    dataTask.taskType       = dataFinLoadTask::TaskType::finQuotesLoadFromStorage;
    dataTask.TickerID       = iTickerID;
    dataTask.dtBegin        = tBegin;
    dataTask.dtEnd          = tEnd;
    dataTask.holder         = Holders[iTickerID];
    lk.unlock();


    auto ItT (std::find_if(vTickersLst.begin(),vTickersLst.end(),[&](const Ticker &t){
        return t.TickerID() == iTickerID;
        }));
    if (ItT != vTickersLst.end()){
        auto ItM (std::find_if(vMarketsLst.begin(),vMarketsLst.end(),[&](const Market &m){
            return m.MarketID() == ItT->MarketID();
            }));
        if (ItM != vMarketsLst.end()){
            dataTask.vSessionTable  = ItM->SessionTable();
            dataTask.vRepoTable     = ItM->RepoTable();
        }
    }

    queueFinQuotesLoad.Push(dataFinLoadTask(dataTask));

    thrdPoolLoadFinQuotes.AddTask([&](){
        workerLoader::workerDataBaseWork(queueFinQuotesLoad,queueTrdAnswers,stStore);
        });

}

//--------------------------------------------------------------------------------------------------------------------------------
//    void MainWindow::slotTestPvBars(std::shared_ptr<std::vector<std::vector<BarTick>>> pvBars)
//    {
////        {
////            ThreadFreeCout pcout;
////            pcout <<"vectors:"<<pvBars->size()<<"\n";
////            int iCount{0};
////            for (const auto &r:*pvBars.get()){
////                iCount +=  r.size();
////            }
////            pcout <<"total size = "<<iCount<<"\n";

////            pcout <<"stored:"<<testPvBars.size()<<"\n";
////            iCount = 0;
////            for (const auto &r:testPvBars){
////                iCount +=  r.size();
////            }
////            pcout <<"stored size = "<<iCount<<"\n";
////        }


//        bool bEqual{true};
//        if(testPvBars.size()>0){
//            size_t iCount{0};
//            auto ItL (testPvBars.begin());
//            auto ItTstL (pvBars->begin());
//            while(bEqual && ItL != testPvBars.end() && ItTstL != pvBars->end()){
//                if (ItTstL->size() == 0) {ItTstL++; continue;}
//                if (ItL->size() == 0) {ItL++; continue;}

//                auto It (ItL->begin());
//                auto ItTst (ItTstL->begin());
//                while(It != ItL->end() && ItTst != ItTstL->end()){
//                    if(It->Period() != ItTst->Period()){
//                        ThreadFreeCout pcout;
//                        pcout<<"testPvBars and pvBars not equal 1!!!\n";
//                        std::time_t pvP = ItTst->Period();
//                        std::time_t pvS = It->Period();
//                        pcout <<"test time:   "<< threadfree_gmtime_to_str(&pvP)<<"\n";
//                        pcout <<"stored time: "<< threadfree_gmtime_to_str(&pvS)<<"\n";
//                        bEqual = false;
//                        break;
//                    }
//                    iCount++;
//                    It++;
//                    ItTst++;
//                }
//                if((It != ItL->end() || ItTst != ItTstL->end())){
//                    ThreadFreeCout pcout;
//                    pcout<<"testPvBars and pvBars not equal 2!!!\n";
//                    bEqual = false;
//                    break;
//                }
//                ItL++;
//                ItTstL++;
//            }
//            if(ItL != testPvBars.end() || ItTstL != pvBars->end()){
//                ThreadFreeCout pcout;
//                pcout<<"testPvBars and pvBars not equal 3!!!\n";
//                bEqual = false;
//            }
//            if(bEqual){
//                ThreadFreeCout pcout;
//                pcout<<"testPvBars and pvBars are Equal. Size: {"<<iCount<<"}\n";
//            }
//        }
//        else{
//            ThreadFreeCout pcout;
//            pcout<<"testPvBars is empty\n";
//            if (pvBars == nullptr){
//                pcout<<"pvBars is empty too\n";
//            }
//            for (const auto &lst:*pvBars){
//                testPvBars.push_back({});
//                for(const auto & b:lst){
//                    testPvBars.back().push_back(b);
//                }
//            }
//        }
//    }
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::InitHolders()
{
    if (iDefaultMonthDepth < 0 ) iDefaultMonthDepth = 0;
    if (iDefaultMonthDepth > 99 ) iDefaultMonthDepth = 99;

    std::time_t tNow =std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::time_t tBegin = Storage::dateAddMonth(tNow, (-iDefaultMonthDepth));
    //
    for(const Ticker &t:vTickersLst){
        if (t.AutoLoad() || bFillNotAutoloadedTickers)
        {
            std::shared_lock lk(mutexMainHolder);
            if (Holders.find(t.TickerID()) == Holders.end()){
                lk.unlock();
                std::shared_lock lk2(mutexMainHolder);
                Holders[t.TickerID()] = std::make_shared<GraphHolder>(GraphHolder{t.TickerID()});
                lk2.unlock();
                lk.lock();
            }
            //
            dataFinLoadTask dataTask;
            dataTask.taskType       = dataFinLoadTask::TaskType::finQuotesLoadFromStorage;
            dataTask.TickerID       = t.TickerID();
            dataTask.dtBegin        = tBegin;
            dataTask.dtEnd          = tNow;
            dataTask.holder         = Holders[t.TickerID()];
            lk.unlock();

            auto ItM (std::find_if(vMarketsLst.begin(),vMarketsLst.end(),[&](const Market &m){
                return m.MarketID() == t.MarketID();
                }));
            if (ItM != vMarketsLst.end()){
                dataTask.vSessionTable  = ItM->SessionTable();
                dataTask.vRepoTable     = ItM->RepoTable();
            }

            queueFinQuotesLoad.Push(dataFinLoadTask(dataTask));

            thrdPoolLoadFinQuotes.AddTask([&](){
                workerLoader::workerDataBaseWork(queueFinQuotesLoad,queueTrdAnswers,stStore);
                });
        }
    }
}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::slotGVFramesVisibilityStateChanged()
{
    std::tuple<bool,bool,bool,bool,bool> tp{
                pacGVLeftSc->isChecked(),
                pacGVRightSc->isChecked(),
                pacGVUpperSc->isChecked(),
                pacGVLowerSc->isChecked(),
                pacGVVolumeSc->isChecked()};

    GraphViewForm * wnd{nullptr};
    QList<QMdiSubWindow*> lst = ui->mdiArea->subWindowList();
    for(int i = 0; i < lst.size(); ++i){
        wnd = qobject_cast<GraphViewForm *>(lst[i]->widget());
        if(wnd ){
            wnd->setFramesVisibility(tp);
        }
    }
}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::slotSaveNewDefaultPath(bool bDef,QString qsPath)
{
    qsStorageDirPath    = qsPath;
    bDefaultStoragePath = bDef;

    SaveSettings();
    qApp->quit();
    QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::slotAmiPipeWndow()
{

    bAmiPipesFormShown = !bAmiPipesFormShown;

    ResizingLeftToolBars();


//    if(pAmiPipeWindow == nullptr){

//        pAmiPipeWindow=new AmiPipesForm (&m_MarketLstModel,iDefaultTickerMarket,&m_TickerLstModel,pipesHolder,vTickersLst,
//                                          bAmiPipeShowByNameUnallocated, bAmiPipeShowByNameActive,bAmiPipeShowByNameOff);
//        pAmiPipeWindow->setAttribute(Qt::WA_DeleteOnClose);
//        pAmiPipeWindow->setWindowTitle(tr("Import from trade sistems"));
//        pAmiPipeWindow->setWindowIcon(QPixmap(":/store/images/sc_cut"));
//        connect(pAmiPipeWindow,SIGNAL(NeedSaveDefaultTickerMarket(int)),this,SLOT(slotStoreDefaultTickerMarket(int)));

//        //ui->mdiArea->addSubWindow(pdoc);

//        connect(pAmiPipeWindow,SIGNAL(SendToMainLog(QString)),this,SIGNAL(SendToLog(QString)));
//        connect(pAmiPipeWindow,SIGNAL(WasCloseEvent()),this,SLOT(slotAmiPipeFormWasClosed()));

//        connect(pAmiPipeWindow,SIGNAL(NeedSaveShowByNamesUnallocated(bool)),this,SLOT(slotAmiPipeSaveShowByNamesUnallocated(bool)));
//        connect(pAmiPipeWindow,SIGNAL(NeedSaveShowByNamesActive(bool)),this,SLOT(slotAmiPipeSaveShowByNamesActive(bool)));
//        connect(pAmiPipeWindow,SIGNAL(NeedSaveShowByNamesOff(bool)),this,SLOT(slotAmiPipeSaveShowByNamesOff(bool)));


//        pAmiPipeWindow->show();
//    }
//    else{
//        pAmiPipeWindow->show();
//        pAmiPipeWindow->setFocus();
//    }

}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::slotSaveGeneralOptions(bool FillNotAutoloaded,bool GrayColor,int MonthDepth,
                                        bool bSaveLogToFile,
                                        int iLogSize,
                                        int iLogCount,
                                        bool bSaveErrorLogToFile,
                                        int iErrorLogSize,
                                        int iErrorLogCount,
                                        bool bInvertMouseWheel,
                                        bool bShowHelpButtons,
                                        bool bWhiteBackgtound,
                                        bool bShowIntroductoryTips
                                        )
{
    bFillNotAutoloadedTickers           = FillNotAutoloaded;
    bGrayColorFroNotAutoloadedTickers   = GrayColor;
    iDefaultMonthDepth                  = MonthDepth;

    bDefaultSaveLogToFile           = bSaveLogToFile;
    iDefaultLogSize                 = iLogSize;
    iDefaultLogCount                = iLogCount;
    bDefaultSaveErrorLogToFile      = bSaveErrorLogToFile;
    iDefaultErrorLogSize            = iErrorLogSize;
    iDefaultErrorLogCount           = iErrorLogCount;
    bDefaultInvertMouseWheel        = bInvertMouseWheel;
    bDefaultShowHelpButtons         = bShowHelpButtons;
    bDefaultWhiteBackgtound         = bWhiteBackgtound;
    bDefaultShowIntroductoryTips    = bShowIntroductoryTips;

    SaveSettings();

    m_TickerLstModel.setGrayColorForInformants(bGrayColorFroNotAutoloadedTickers);

    emit InvertMouseWheelChanged(bDefaultInvertMouseWheel);
    emit ShowHelpButtonsChanged(bDefaultShowHelpButtons);

}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::slotAmiPipeFormWasClosed()
{
//    resizeDocks({ui->dkActiveTickers},{iStoredTickerBarWidth},Qt::Orientation::Horizontal);
//    if (pAmiPipeWindow){
//        ui->gridLayoutDockBarRight->removeWidget(pAmiPipeWindow);
//        delete pAmiPipeWindow;
//        pAmiPipeWindow = nullptr;
//    }
}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::slotAmiPipeSaveShowByNamesUnallocated(bool b)
{
    bAmiPipeShowByNameUnallocated = b;
    SaveSettings();
}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::slotAmiPipeSaveShowByNamesActive(bool b)
{
    bAmiPipeShowByNameActive = b;
    SaveSettings();
}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::slotAmiPipeSaveShowByNamesOff(bool b)
{
    bAmiPipeShowByNameOff = b;
    SaveSettings();
}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::BuildSessionsTableForFastTasks(FastTasksHolder & fastHolder)
{
    std::map<int,Market::SessionTable_type>  mappedRepoTable;

    for (const auto &t:vTickersLst){
        auto ItM (std::find_if(vMarketsLst.begin(),vMarketsLst.end(),[&](const Market &m){
            return m.MarketID() == t.MarketID();
            }));
        if (ItM != vMarketsLst.end()){
            mappedRepoTable[t.TickerID()] = ItM->RepoTable();
        }
    }
    ////
    fastHolder.setRepoTable(mappedRepoTable);    
}
//--------------------------------------------------------------------------------------------------------------------------------
std::size_t MainWindow::getPhisicalMemory()
{
    std::size_t iResult{0};

#ifdef _WIN32

    typedef BOOL (WINAPI *PGMSE)(LPMEMORYSTATUSEX);
    std::string strModule("kernel32.dll");
    std::wstring wModule(strModule.begin(), strModule.end());

    PGMSE pGMSE = (PGMSE) GetProcAddress( GetModuleHandle( wModule.data()), "GlobalMemoryStatusEx" );
    //PGMSE pGMSE = (PGMSE) GetProcAddress( GetModuleHandle( TEXT( "kernel32.dll" ) ), TEXT( "GlobalMemoryStatusEx") );
    if ( pGMSE != 0 )
    {
        MEMORYSTATUSEX mi;
        memset( &mi, 0, sizeof(MEMORYSTATUSEX) );
        mi.dwLength = sizeof(MEMORYSTATUSEX);
        if ( pGMSE( &mi ) == TRUE )
            iResult = mi.ullTotalPhys;
        else
            pGMSE = 0;
    }
    if ( pGMSE == 0 )
    {
        MEMORYSTATUS mi;
        memset( &mi, 0, sizeof(MEMORYSTATUS) );
        mi.dwLength = sizeof(MEMORYSTATUS);
        GlobalMemoryStatus( &mi );
        iResult = mi.dwTotalPhys;
    }

#elif __linux__

    //read /proc/meminfo
    QProcess p;
    p.start("awk", QStringList() << "/MemTotal/ { print $2 }" << "/proc/meminfo");
    p.waitForFinished();
    QString memory = p.readAllStandardOutput();

    QProcess p1;
    p1.start("awk", QStringList() << "/MemTotal/ { print $3 }" << "/proc/meminfo");
    p1.waitForFinished();
    QString sizing = p1.readAllStandardOutput().trimmed();

    int iScale{1};
    if(sizing.toUpper() == "KB")        {   iScale = 1024;      }
    else if(sizing.toUpper() == "MB")   {   iScale = 1048576;   }
    else if(sizing.toUpper() == "GB")   {   iScale = 1073741824;}


    iResult = memory.toLong() * iScale;
    p.close();

#elif defined(__APPLE__) && defined(__MACH__)

    // read sysctl
    QProcess p;
    p.start("sysctl", QStringList() << "kern.version" << "hw.physmem");
    p.waitForFinished();
    QString system_info = p.readAllStandardOutput();
    iResult = system_info.toLong();
    p.close();

#else

    iResult = 0;

#endif

    return iResult;
}
//--------------------------------------------------------------------------------------------------------------------------------
bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->wtTickerBar){
        eventTickerBar(watched, event);
    }
    return QObject::eventFilter(watched, event);
}
//--------------------------------------------------------------------------------------------------------------------------------
bool MainWindow::eventTickerBar(QObject */*watched*/, QEvent *event)
{

    const int iLineWidth{3};

    if (event->type() == QEvent::HoverMove){
        QMouseEvent *pe = (QMouseEvent *)event;
        QPoint pt = pe->pos();
        if (pt.x() + iLineWidth > ui->wtTickerBar->width()){
            if (!bLeftToolbarCursorOverriden){
                QApplication::setOverrideCursor(Qt::CursorShape::SizeHorCursor);
                bLeftToolbarCursorOverriden = true;
            }
        }
        else{
            if (bLeftToolbarCursorOverriden && !bInResizingLeftToolbar) {
                QApplication::restoreOverrideCursor();
                bLeftToolbarCursorOverriden = false;
            }
        }
        if (bInResizingLeftToolbar){

            int iStoped = pe->pos().x();
            int iNewWidth = iStoredLeftDocbarWidth + (iStoped - iStoredLeftDocbarRightPos);

            if (ui->dkActiveTickers->minimumWidth() < iNewWidth){
                iStoredTickerBarWidth = iNewWidth;
                //resizeDocks({ui->dkActiveTickers},{iNewWidth},Qt::Orientation::Horizontal);
                ResizingLeftToolBars();
            }
        }
    }
    else if (event->type() == QEvent::MouseButtonPress){
        QMouseEvent *pe = (QMouseEvent *)event;
        QPoint pt = pe->pos();
        if (pt.x() + iLineWidth > ui->wtTickerBar->width()){
            bInResizingLeftToolbar = true;
            if (!bLeftToolbarCursorOverriden){
                QApplication::setOverrideCursor(Qt::CursorShape::SizeHorCursor);
                bLeftToolbarCursorOverriden = true;
            }

            QMouseEvent *pe = (QMouseEvent *)event;
            iStoredLeftDocbarRightPos = pe->pos().x();
            iStoredLeftDocbarWidth = ui->dkActiveTickers->width();
        }
    }
    else if (event->type() == QEvent::MouseButtonRelease){

        if (bLeftToolbarCursorOverriden){
            QApplication::restoreOverrideCursor();
            bLeftToolbarCursorOverriden = false;
        }
        if (bInResizingLeftToolbar){
                bInResizingLeftToolbar = false;

                QMouseEvent *pe = (QMouseEvent *)event;
                int iStoped = pe->pos().x();
                int iNewWidth = iStoredLeftDocbarWidth + (iStoped - iStoredLeftDocbarRightPos);

                if (ui->dkActiveTickers->minimumWidth() < iNewWidth){

                    iStoredTickerBarWidth = iNewWidth;
                    //resizeDocks({ui->dkActiveTickers},{iNewWidth},Qt::Orientation::Horizontal);
                    ResizingLeftToolBars();

                }
        }
    }
    else if (event->type() == QEvent::HoverEnter){
        QMouseEvent *pe = (QMouseEvent *)event;
        QPoint pt = pe->pos();
        if (pt.x() + iLineWidth > ui->wtTickerBar->width()){
            if (!bLeftToolbarCursorOverriden){
                QApplication::setOverrideCursor(Qt::CursorShape::SizeHorCursor);
                bLeftToolbarCursorOverriden = true;
            }
        }
    }
    else if (event->type() == QEvent::HoverLeave){
        if (bLeftToolbarCursorOverriden) {
            QApplication::restoreOverrideCursor();
            bLeftToolbarCursorOverriden = false;
        }
    }
    return true;
}

//--------------------------------------------------------------------------------------------------------------------------------
void  MainWindow::slotAmiPipeNewWndStateChanged(int iS)
{
    if (iS != 0)    bAmiPipesNewWndShown = true;
    else            bAmiPipesNewWndShown = false;
    pacAmiPipeBarNew->setChecked(bAmiPipesNewWndShown);

}
//--------------------------------------------------------------------------------------------------------------------------------
void  MainWindow::slotAmiPipeActiveWndStateChanged(int iS)
{
    if (iS != 0)    bAmiPipesActiveWndShown = true;
    else            bAmiPipesActiveWndShown = false;
    pacAmiPipeBarActive->setChecked(bAmiPipesActiveWndShown);
}
//--------------------------------------------------------------------------------------------------------------------------------
void  MainWindow::slotAmiPipeHideClicked()
{
    bAmiPipesFormShown = false;
    ResizingLeftToolBars();
}
//--------------------------------------------------------------------------------------------------------------------------------
void  MainWindow::slotAmiPipeWndowNew()
{
    bAmiPipesNewWndShown = pacAmiPipeBarNew->isChecked();
    if (!bAmiPipesNewWndShown){
        bAmiPipesActiveWndShown = true;
        pacAmiPipeBarActive->setChecked(true);
    }
    AmiPipeInternalPanelsStateChanged(bAmiPipesNewWndShown,bAmiPipesActiveWndShown);
}
//--------------------------------------------------------------------------------------------------------------------------------
void  MainWindow::slotAmiPipeWndowActive()
{
    bAmiPipesActiveWndShown = pacAmiPipeBarActive->isChecked();
    if (!bAmiPipesActiveWndShown){
        bAmiPipesNewWndShown = true;
        pacAmiPipeBarNew->setChecked(true);
    }
    AmiPipeInternalPanelsStateChanged(bAmiPipesNewWndShown,bAmiPipesActiveWndShown);

    //slotInternalPanelsStateChanged(bool bLeft, bool bRight)
}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::slotSendToLog(QString str)
{
    if (bDefaultSaveLogToFile){
        std::stringstream ss;
        std::time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        ss << threadfree_gmtime_to_str(&t)<<": ";
        ss << str.toStdString();

        iCurrentLogfile = stStore.SaveToLogfile(ss.str(), "logfile", iCurrentLogfile,iDefaultLogSize, iDefaultLogCount);
        SaveSettings();
    }
}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::slotSendToErrorLog(QString str)
{
    if (bDefaultSaveErrorLogToFile){
        std::stringstream ss;
        std::time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        ss << threadfree_gmtime_to_str(&t)<<": ";
        ss << str.toStdString();

        iCurrentErrorLogfile = stStore.SaveToLogfile(ss.str(), "errorlog", iCurrentErrorLogfile,iDefaultErrorLogSize, iDefaultErrorLogCount);
        SaveSettings();
    }
}
//--------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------



