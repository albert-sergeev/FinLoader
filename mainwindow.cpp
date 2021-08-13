#include "mainwindow.h"
#include "ui_mainwindow.h"

#include<QListView>
#include<QHBoxLayout>





//--------------------------------------------------------------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , vMarketsLst{}
    , m_MarketLstModel{vMarketsLst,this}
    , vTickersLst{}
    , m_TickerLstModel{vTickersLst,vMarketsLst,mBlinkedState,this}
    , thrdPoolLoadFinQuotes{(int)std::thread::hardware_concurrency()/2}
    , ui(new Ui::MainWindow)
{

    ui->setupUi(this);

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
    swtShowByName = new StyledSwitcher(tr("Show by name "),tr(" Show by ticker"),true,10,this);
    lt3->addWidget(swtShowByName);
    swtShowByName->SetOnColor(QPalette::Window,colorDarkGreen);
    swtShowByName->SetOffColor(QPalette::Window,colorDarkRed);
    //-------------------------------------------------------------


    LoadSettings();
    slotSetActiveLang (m_Language);
    slotSetActiveStyle(m_sStyleName);

    InitAction();

    LoadDataStorage();

    InitDockBar();

    connect(&m_TickerLstModel,SIGNAL(dataChanged(const QModelIndex &,const QModelIndex &)), this,SLOT(slotTickerDataStorageUpdate(const QModelIndex &,const QModelIndex &)));
    connect(&m_TickerLstModel,SIGNAL(dataRemoved(const Ticker &)), this,SLOT(slotTickerDataStorageRemove(const Ticker &)));



    connect(swtShowByName,SIGNAL(stateChanged(int)),this,SLOT(slotDocbarShowNyNameChanged(int)));
    connect(swtShowAll,SIGNAL(stateChanged(int)),this,SLOT(slotDocbarShowAllChanged(int)));
    connect(swtShowMarkets,SIGNAL(stateChanged(int)),this,SLOT(slotDocbarShowMarketChanged(int)));


    ///
    startTimer(100); // timer to process GUID events
}
//--------------------------------------------------------------------------------------------------------------------------------
MainWindow::~MainWindow()
{
    SaveSettings();
    delete ui;
}
//--------------------------------------------------------------------------------------------------------------------------------
bool MainWindow::event(QEvent *event)
{
    if(event->type() == QEvent::Close){
        SaveUnsavedConfigs();

        for(auto w: vBulbululators){
            if(w!=nullptr){
                w->close();
            }
        }

    }
    return QWidget::event(event);
}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::timerEvent(QTimerEvent * event)
{
    bool bSuccess{false};
    auto pdata (queueTrdAnswers.Pop(bSuccess));
    ImportFinQuotesForm * wnd{nullptr};
    while(bSuccess){
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
                SendToLog(QString::fromStdString(ss.str()));
            }
            break;
        case dataBuckgroundThreadAnswer::eAnswerType::storagLoadFromStorageGraphEnd:
            {
                std::stringstream ss;
                ss << "Load from storage ends [" << data.TickerID()<<"] success: ["<<data.Successfull()<<"]";
                if(!data.Successfull()){
                    ss <<data.GetErrString();
                }
                SendToLog(QString::fromStdString(ss.str()));
            }
            break;
        case dataBuckgroundThreadAnswer::eAnswerType::storagLoadToGraphBegin:
            {
                std::stringstream ss;
                ss << "Load to Graph begins [" << data.TickerID()<<"]";
                SendToLog(QString::fromStdString(ss.str()));
            }
            break;
        case dataBuckgroundThreadAnswer::eAnswerType::storagLoadToGraphEnd:
            {
                std::stringstream ss;
                ss << "Load to Graph ends [" << data.TickerID()<<"]";
                if(!data.Successfull()){
                    ss <<"\n"<<data.GetErrString();
                }
                SendToLog(QString::fromStdString(ss.str()));
            }
            break;
        case dataBuckgroundThreadAnswer::eAnswerType::logText:
            SendToLog(QString::fromStdString(trim(data.GetTextInfo())));
            break;
        case dataBuckgroundThreadAnswer::eAnswerType::logCriticalError:
            SendToLog(QString::fromStdString(trim(data.GetErrString())));
            {
                int n=QMessageBox::critical(0,tr("Critical error"),
                                   QString::fromStdString(data.GetErrString()),
                                   QMessageBox::Ok
                                   );
                if (n==QMessageBox::Ok){
                    ;
                }
            }
            break;
            //,,
        default:
            break;
        }
        //////////////////////////////////////////////
        pdata = queueTrdAnswers.Pop(bSuccess);
    }
    //////////////////
    ListViewActivityTermination();
    //
    QWidget::timerEvent(event);
}
//--------------------------------------------------------------------------------------------------------------------------------
///
/// \brief plug for future
///
void MainWindow::slotNotImpl(){};



//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::LoadSettings()
{

    QString sMark;

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

        m_settings.endGroup();

        m_settings.beginGroup("Configwindow");
            iDefaultTickerMarket    = m_settings.value("DefaultTickerMarket",0).toInt();
            bConfigTickerShowByName = m_settings.value("ConfigTickerShowByName",true).toBool();
            bConfigTickerSortByName = m_settings.value("ConfigTickerSortByName",true).toBool();
        m_settings.endGroup();

        m_settings.beginGroup("ImportFinamForm");
            qsDefaultOpenDir    = m_settings.value("DefaultOpenDir","").toString();
            QString qs          = m_settings.value("ImportDelimiter",",").toString();
            std::string s = qs.toStdString();
            cImportDelimiter    = s.size()>0 ? s[0]: ',';
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
        m_settings.endGroup();

        m_settings.beginGroup("Configwindow");
            m_settings.setValue("DefaultTickerMarket",iDefaultTickerMarket);
            m_settings.setValue("ConfigTickerShowByName",bConfigTickerShowByName);
            m_settings.setValue("ConfigTickerSortByName",bConfigTickerSortByName);
        m_settings.endGroup();

        m_settings.beginGroup("ImportFinamForm");
            m_settings.setValue("DefaultOpenDir",qsDefaultOpenDir);
            std::string s {" "}; s[0] = cImportDelimiter;
            QString qs = QString::fromStdString(s);
            m_settings.setValue("ImportDelimiter",qs);
        m_settings.endGroup();

    m_settings.endGroup();

}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::LoadDataStorage()
{
    try{
        stStore.Initialize();
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
            stStore.SaveTickerConfig(vTickersLst[i]);
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
void MainWindow::SaveDataStorage()
{
}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::slotSendTestText()
{
    SendToLog("Test text");
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
    pacNewDoc->setText(tr("&New"));
    pacNewDoc->setShortcut(QKeySequence(tr("CTRL+N")));
    pacNewDoc->setToolTip(tr("New graph"));
    pacNewDoc->setStatusTip(tr("New graph"));
    pacNewDoc->setWhatsThis(tr("New graph"));
    pacNewDoc->setIcon(QPixmap(":/store/images/sc_newdoc"));
    connect(pacNewDoc,SIGNAL(triggered()),SLOT(slotGraphViewWindow()));

    //------------------------------------------------
    QAction * pacOpen =new QAction("Open");
    pacOpen->setText(tr("&Open"));
    pacOpen->setShortcut(QKeySequence(tr("CTRL+O")));
    pacOpen->setToolTip(tr("Load history data"));
    pacOpen->setStatusTip(tr("Load history data"));
    pacOpen->setWhatsThis(tr("Load history data"));
    pacOpen->setIcon(QPixmap(":/store/images/sc_open"));
    connect(pacOpen,SIGNAL(triggered()),SLOT(slotImportFinQuotesWndow()));
    //------------------------------------------------
    QAction * pacSave =new QAction("Save");
    pacSave->setText(tr("&Save"));
    pacSave->setShortcut(QKeySequence(tr("CTRL+S")));
    pacSave->setToolTip(tr("Save Document"));
    pacSave->setStatusTip(tr("Save file to disk"));
    pacSave->setWhatsThis(tr("Save file to disk"));
    pacSave->setIcon(QPixmap(":/store/images/sc_save"));
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
    QAction * pacConfig =new QAction("Config");
    pacConfig->setText(tr("Confi&g"));
    pacConfig->setShortcut(QKeySequence(tr("CTRL+G")));
    pacConfig->setToolTip(tr("Config"));
    pacConfig->setStatusTip(tr("Config"));
    pacConfig->setWhatsThis(tr("Config"));
    pacConfig->setIcon(QPixmap(":/store/images/sc_config"));
    connect(pacConfig,SIGNAL(triggered()),SLOT(slotConfigWndow()));
    //------------------------------------------------
    pacTickersBar =new QAction(tr("Tickers bar"));
    pacTickersBar->setText(tr("Tickers bar"));
    pacTickersBar->setCheckable(true);
    pacTickersBar->setChecked(!bTickerBarOnLoadIsHidden);
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
    //
    QMenu * pmnuFile = new QMenu(tr("&File","menu"));
    pmnuFile->addAction(pacNewDoc);
    pmnuFile->addAction(pacOpen);
    pmnuFile->addAction(pacSave);
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
    QMenu * pmnuTools = new QMenu(tr("&Tools"));
    pmnuTools->addAction(pacLogWnd);
    menuBar()->addMenu(pmnuTools);
    //
    QMenu * pmnuSettings = new QMenu(tr("&Settings"));

    pmnuSettings->addAction(pacToolBar);
    pmnuSettings->addAction(pacTickersBar);
    pmnuSettings->addAction(pacStatusBar);
    pmnuSettings->addAction(pacTickersBarButtonsHide);
    pmnuSettings->addSeparator();
    pmnuSettings->addAction(pacConfig);
    pmnuSettings->addSeparator();

    m_mnuStyles = new QMenu(tr("St&yles"));
    pmnuSettings->addMenu(m_mnuStyles);
    connect(m_mnuStyles,SIGNAL(aboutToShow()),SLOT(slotStyles()));
    m_mnuLangs = new QMenu(tr("&Language"));
    pmnuSettings->addMenu(m_mnuLangs);
    connect(m_mnuLangs,SIGNAL(aboutToShow()),SLOT(slotLanguages()));

    menuBar()->addMenu(pmnuSettings);
    //
    QMenu * pmnuHelp = new QMenu(tr("&Help"));
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
    tbrToolBar->addAction(pacOpen);
    tbrToolBar->addAction(pacSave);
    tbrToolBar->addAction(pacConfig);
    tbrToolBar->addAction(pacLogWnd);

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
void MainWindow::BulbululatorRemoveActive   (int TickerID)
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
        vBulbululators[i]->close();
        disconnect(vBulbululators[i],SIGNAL(DoubleClicked(const int)),this,SLOT(slotSetSelectedTicker(const  int)));
        ui->statusbar->removeWidget(vBulbululators[i]);
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
            break;
        }
        i++;
    }
    if (!bFound){
        Bulbululator * blbl = new Bulbululator();
        connect(blbl,SIGNAL(DoubleClicked(const int)),this,SLOT(slotSetSelectedTicker(const  int)));

        if(blbl){
            blbl->SetText(str);
            blbl->SetTickerID(TickerID);
            vBulbululators.push_back(blbl);
            ui->statusbar->addWidget(blbl);
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

    for(auto s:lstStyles){
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
                //TODO: auto start app
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
            //TODO: auto start app
        }
    }
}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::slotGraphViewWindow()
{
    auto qml(ui->lstView->selectionModel());
    auto lst (qml->selectedIndexes());
    //if(lst.count() > 0 && lst[0].isValid()){

    for (auto item:lst){
        const Ticker &t = proxyTickerModel.getTicker(item);
        slotSetSelectedTicker(t.TickerID());
    }
}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::slotConfigWndow()
{
    ConfigWindow *pdoc=new ConfigWindow(&m_MarketLstModel,iDefaultTickerMarket,
                                        &m_TickerLstModel,bConfigTickerShowByName,bConfigTickerSortByName);
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

    //
    pdoc->show();
}

//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::slotParseImportFinQuotesFile(dataFinLoadTask & dtTask)
{
    dtTask.SetStore(&stStore);
    queueFinQuotesLoad.Push(dataFinLoadTask(dtTask));

    thrdPoolLoadFinQuotes.AddTask([&](){
        workerLoader::workerDataBaseWork(queueFinQuotesLoad,queueTrdAnswers,stStore);
        });
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
    if (swtShowAll){
        proxyTickerModel.setFilterByActive(!swtShowAll->isChecked());
    }

    proxyTickerModel.setSourceModel(&m_TickerLstModel);
    proxyTickerModel.sort(2);
    ui->lstView->setModel(&proxyTickerModel);

    slotDocbarShowMarketChanged(0);

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
        GraphViewForm *pdoc=new GraphViewForm(iTickerID,vTickersLst);
        pdoc->setAttribute(Qt::WA_DeleteOnClose);
        pdoc->setWindowIcon(QPixmap(":/store/images/sc_newdoc"));

        ui->mdiArea->addSubWindow(pdoc);

        connect(pdoc,SIGNAL(NeedLoadGraph(const  int,const std::time_t,const std::time_t)),
                  this,SLOT(slotLoadGraph(const  int,const std::time_t,const std::time_t)));

        pdoc->show();
    }
    else{
        wnd->setFocus();
    }
}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::slotLoadGraph(const  int iTickerID, const std::time_t tBegin, const std::time_t tEnd)
{
    if (Holders.find(iTickerID) == Holders.end()){
        Holders[iTickerID] = std::make_shared<GraphHolder>(GraphHolder{iTickerID});
    }

    dataFinLoadTask dataTask;
    dataTask.taskType       = dataFinLoadTask::TaskType::finQuotesLoadFromStorage;
    dataTask.TickerID       = iTickerID;
    dataTask.dtBegin        = tBegin;
    dataTask.dtEnd          = tEnd;
    dataTask.holder         = Holders[iTickerID];


    queueFinQuotesLoad.Push(dataFinLoadTask(dataTask));

    thrdPoolLoadFinQuotes.AddTask([&](){
        workerLoader::workerDataBaseWork(queueFinQuotesLoad,queueTrdAnswers,stStore);
        });
}

