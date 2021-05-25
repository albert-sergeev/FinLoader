#include "mainwindow.h"
#include "ui_mainwindow.h"

//--------------------------------------------------------------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    InitAction();
    LoadSettings();
}
//--------------------------------------------------------------------------------------------------------------------------------
MainWindow::~MainWindow()
{
    SaveSettings();
    delete ui;
}

//--------------------------------------------------------------------------------------------------------------------------------
///
/// \brief plug for future
///
void MainWindow::slotNotImpl(){};
void MainWindow::slotLanguages  (){};
void MainWindow::slotSetActiveLang      (QString){};

//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::LoadSettings()
{

    QString sMark;

    m_settings.beginGroup("Settings");
        sMark = m_settings.value("markKey","nothing").toString();
        m_settings.beginGroup("Mainwindow");
            int nWidth = m_settings.value("Width", width()).toInt();
            int nHeight = m_settings.value("Height",height()).toInt();
            m_sStyleName = m_settings.value("StyleName",style()->objectName()).toString();
        m_settings.endGroup();
    m_settings.endGroup();
    //
    resize(nWidth,nHeight);
    slotSetActiveStyle(m_sStyleName);
    //addToLog(sMark);

}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::SaveSettings()
{
    //
    m_settings.beginGroup("Settings");
        m_settings.setValue("markKey","Hulala");
        m_settings.beginGroup("Mainwindow");
            m_settings.setValue("Width", this->width());
            m_settings.setValue("Height",this->height());
            m_settings.setValue("StyleName",m_sStyleName);
        m_settings.endGroup();
    m_settings.endGroup();

}
//--------------------------------------------------------------------------------------------------------------------------------
void MainWindow::slotSendTestText()
{
    addToLog("Test text");
    //addToLog("Test text");
    qDebug()<<style()->objectName();
    qDebug()<<style();
}
//--------------------------------------------------------------------------------------------------------------------------------
////////
/// \brief Initialisation of menues and panels
///
void MainWindow::InitAction()
{
    //------------------------------------------------
    QAction * pacNewDoc =new QAction("newdoc");
    pacNewDoc->setText("&New");
    pacNewDoc->setShortcut(QKeySequence("CTRL+N"));
    pacNewDoc->setToolTip("New board");
    pacNewDoc->setStatusTip("New board");
    pacNewDoc->setWhatsThis("New board");
    pacNewDoc->setIcon(QPixmap(":/store/images/sc_newdoc"));
    connect(pacNewDoc,SIGNAL(triggered()),SLOT(slotNewDoc()));
    //------------------------------------------------
    QAction * pacOpen =new QAction("Open");
    pacOpen->setText("&Open");
    pacOpen->setShortcut(QKeySequence("CTRL+L"));
    pacOpen->setToolTip("Load history data");
    pacOpen->setStatusTip("Load history data");
    pacOpen->setWhatsThis("Load history data");
    pacOpen->setIcon(QPixmap(":/store/images/sc_open"));
    //------------------------------------------------
    QAction * pacSave =new QAction("Save");
    pacSave->setText("&Save");
    pacSave->setShortcut(QKeySequence("CTRL+S"));
    pacSave->setToolTip("Save Document");
    pacSave->setStatusTip("Save file to disk");
    pacSave->setWhatsThis("Save file to disk");
    pacSave->setIcon(QPixmap(":/store/images/sc_save"));
    //------------------------------------------------
    QAction * pacLogWnd =new QAction("LogWnd");
    pacLogWnd->setText("Lo&g window");
    pacLogWnd->setShortcut(QKeySequence("ALT+L"));
    pacLogWnd->setToolTip("Log window");
    pacLogWnd->setStatusTip("Log window");
    pacLogWnd->setWhatsThis("Log window");
    pacLogWnd->setIcon(QPixmap(":/store/images/sc_move"));
    connect(pacLogWnd,SIGNAL(triggered()),SLOT(slotNewLogWnd()));
    //------------------------------------------------
    QAction * pacConfig =new QAction("Config");
    pacConfig->setText("Confi&g");
    pacConfig->setShortcut(QKeySequence("CTRL+G"));
    pacConfig->setToolTip("Config");
    pacConfig->setStatusTip("Config");
    pacConfig->setWhatsThis("Config");
    pacConfig->setIcon(QPixmap(":/store/images/sc_config"));
    //------------------------------------------------
    //
    QMenu * pmnuFile = new QMenu("&File");
    pmnuFile->addAction(pacNewDoc);
    pmnuFile->addAction(pacOpen);
    pmnuFile->addAction(pacSave);
    pmnuFile->addSeparator();
    pmnuFile->addAction("&Quit",
                        qApp,
                        SLOT(closeAllWindows()),
                        QKeySequence("CTRL+Q")
                );
    menuBar()->addMenu(pmnuFile);
    //
    m_mnuWindows = new QMenu("&Windows");
    menuBar()->addMenu(m_mnuWindows);
    connect(m_mnuWindows,SIGNAL(aboutToShow()),SLOT(slotWindows()));
    menuBar()->addSeparator();
    //
    QMenu * pmnuTools = new QMenu("&Tools");
    pmnuTools->addAction(pacLogWnd);
    menuBar()->addMenu(pmnuTools);
    //
    QMenu * pmnuSettings = new QMenu("&Settings");
    pmnuSettings->addAction(pacConfig);
    pmnuSettings->addSeparator();

    m_mnuStyles = new QMenu("St&yles");
    pmnuSettings->addMenu(m_mnuStyles);
    connect(m_mnuStyles,SIGNAL(aboutToShow()),SLOT(slotStyles()));
    m_mnuLangs = new QMenu("&Language");
    pmnuSettings->addMenu(m_mnuLangs);
    connect(m_mnuLangs,SIGNAL(aboutToShow()),SLOT(slotLanguages()));

    menuBar()->addMenu(pmnuSettings);
    //
    QMenu * pmnuHelp = new QMenu("&Help");
    pmnuHelp->addAction("&About",this,SLOT(slotAbout()),Qt::Key_F1);
    menuBar()->addMenu(pmnuHelp);

    //------------------------------------------------
    m_psigmapper = new QSignalMapper(this);
    connect(m_psigmapper,SIGNAL(mapped(QWidget*)),this,SLOT(slotSetActiveSubWindow(QWidget*)));
    m_psigmapperStyle = new QSignalMapper(this);
    connect(m_psigmapperStyle,SIGNAL(mapped(QString)),this,SLOT(slotSetActiveStyle(QString)));
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
    pdoc->setWindowTitle("Unnamed document");
    pdoc->setWindowIcon(QPixmap(":/store/images/sc_newdoc"));


    QGridLayout *lt=new QGridLayout();
    QLabel *lbl=new QLabel("Document");
    QPushButton * btn1=new QPushButton("Push it");
    QPushButton * btn2=new QPushButton("Doun't");
    QComboBox * cbx=new QComboBox();

    connect(btn1,SIGNAL(clicked()),this,SLOT(slotSendTestText()));

    cbx->addItem("First elem");
    cbx->addItem("Second elem");
    lt->addWidget(lbl);
    lt->addWidget(btn1);
    lt->addWidget(btn2);
    lt->addWidget(cbx);
    pdoc->setLayout(lt);

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
    pac = m_mnuWindows->addAction("&Cascade",ui->mdiArea,SLOT(cascadeSubWindows()));
    pac->setEnabled(!ui->mdiArea->subWindowList().isEmpty());
    //
    pac = m_mnuWindows->addAction("&Tile",ui->mdiArea,SLOT(tileSubWindows()));
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
    QMessageBox::about(0,"About","FinLoader v.0.0.1");

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
        if (sOldStyle == "BlackStyle"){
            QMessageBox *msg = new QMessageBox(QMessageBox::Question,
                                               "Изменение стиля",
                                               "Для применения стиля нужна перезагрузка. Перегрузить приложение?",
                                               QMessageBox::Yes|QMessageBox::No
                        );
            int n = msg->exec();
            delete msg;
            if(n == QMessageBox::Yes){
                //qDebug()<<"reboot!!!";
                SaveSettings();
                qApp->quit();
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
    pdoc->setWindowTitle("Log window");
    pdoc->setWindowIcon(QPixmap(":/store/images/sc_move"));


    QGridLayout *lt=new QGridLayout();
    QTextEdit * ed=new QTextEdit();

    lt->addWidget(ed);
    lt->setMargin(1);
    pdoc->setLayout(lt);

    connect(this,SIGNAL(addToLog(QString)),ed,SLOT(append(QString)));

    ui->mdiArea->addSubWindow(pdoc);
    pdoc->show();

}
//--------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------

