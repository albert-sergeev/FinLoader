#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "configwindow.h"

#include<QListView>


//--------------------------------------------------------------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , vMarketsLst{}
    , m_MarketLstModel{vMarketsLst}
    , ui(new Ui::MainWindow)
{


    ui->setupUi(this);

    LoadSettings();
    slotSetActiveLang (m_Language);
    slotSetActiveStyle(m_sStyleName);

    InitAction();

    LoadDataStorage();
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
        ;
    }
    return QWidget::event(event);
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
            int nWidth   = m_settings.value("Width", width()).toInt();
            int nHeight  = m_settings.value("Height",height()).toInt();
            m_sStyleName = m_settings.value("StyleName",style()->objectName()).toString();
        m_settings.endGroup();
    m_settings.endGroup();
    //
    resize(nWidth,nHeight);

}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::SaveSettings()
{
    //
    m_settings.beginGroup("Settings");
        m_settings.setValue("Language",m_Language);
        m_settings.beginGroup("Mainwindow");
            m_settings.setValue("Width", this->width());
            m_settings.setValue("Height",this->height());
            m_settings.setValue("StyleName",m_sStyleName);
        m_settings.endGroup();
    m_settings.endGroup();

}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::LoadDataStorage()
{
    try{
        stStore.Initialize();
        stStore.LoadMarketConfig(vMarketsLst);

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
void MainWindow::slotSaveTicketDataStorage()
{
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
    //------------------------------------------------
    QAction * pacNewDoc =new QAction("newdoc");
    pacNewDoc->setText(tr("&New"));
    pacNewDoc->setShortcut(QKeySequence(tr("CTRL+N")));
    pacNewDoc->setToolTip(tr("New board"));
    pacNewDoc->setStatusTip(tr("New board"));
    pacNewDoc->setWhatsThis(tr("New board"));
    pacNewDoc->setIcon(QPixmap(":/store/images/sc_newdoc"));
    connect(pacNewDoc,SIGNAL(triggered()),SLOT(slotNewDoc()));
    //------------------------------------------------
    QAction * pacOpen =new QAction("Open");
    pacOpen->setText(tr("&Open"));
    pacOpen->setShortcut(QKeySequence(tr("CTRL+O")));
    pacOpen->setToolTip(tr("Load history data"));
    pacOpen->setStatusTip(tr("Load history data"));
    pacOpen->setWhatsThis(tr("Load history data"));
    pacOpen->setIcon(QPixmap(":/store/images/sc_open"));
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
    QToolBar * tbr =new QToolBar("Top tool");
    tbr->addAction(pacNewDoc);
    tbr->addAction(pacOpen);
    tbr->addAction(pacSave);
    tbr->addAction(pacConfig);
    tbr->addAction(pacLogWnd);

    this->addToolBar(tbr);

}

//--------------------------------------------------------------------------------------------------------------------------------
///
/// \brief create NewDoc Window (for test purpouse)
///
void MainWindow::slotNewDoc()
{
    QWidget *pdoc=new QWidget;
    pdoc->setAttribute(Qt::WA_DeleteOnClose);
    pdoc->setWindowTitle(tr("Unnamed document"));
    pdoc->setWindowIcon(QPixmap(":/store/images/sc_newdoc"));


    QGridLayout *lt=new QGridLayout();
    QLabel *lbl=new QLabel(tr("Document"));
    QPushButton * btn1=new QPushButton(tr("Push it"));
    QPushButton * btn2=new QPushButton(tr("Doun't"));
    QComboBox * cbx=new QComboBox();

    connect(btn1,SIGNAL(clicked()),this,SLOT(slotSendTestText()));

    cbx->addItem(tr("First elem"));
    cbx->addItem(tr("Second elem"));
    lt->addWidget(lbl);
    lt->addWidget(btn1);
    lt->addWidget(btn2);
    lt->addWidget(cbx);
    pdoc->setLayout(lt);


    QListView *lw8=new QListView();
    lw8->setModel(&m_MarketLstModel);
    lt->addWidget(lw8);

    ui->mdiArea->addSubWindow(pdoc);
    pdoc->show();
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
void MainWindow::slotConfigWndow()
{
    ConfigWindow *pdoc=new ConfigWindow;
    pdoc->setAttribute(Qt::WA_DeleteOnClose);
    pdoc->setWindowTitle(tr("Config"));
    pdoc->setWindowIcon(QPixmap(":/store/images/sc_config"));

   // vMarketsLst.push_back({"MOEX","MOEX"});
    pdoc->setMarketModel(&m_MarketLstModel);
    ui->mdiArea->addSubWindow(pdoc);

    connect(pdoc,SIGNAL(SendToMainLog(QString)),this,SIGNAL(SendToLog(QString)));
    connect(pdoc,SIGNAL(NeedSaveMarketsChanges()),this,SLOT(slotSaveMarketDataStorage()));
    pdoc->show();
}
//--------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------

