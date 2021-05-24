#include "mainwindow.h"
#include "ui_mainwindow.h"

//--------------------------------------------------------------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    InitAction();
}
//--------------------------------------------------------------------------------------------------------------------------------
MainWindow::~MainWindow()
{
    delete ui;
}

//--------------------------------------------------------------------------------------------------------------------------------
///
/// \brief plug for future
///
void MainWindow::slotNotImpl(){};

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
    QMenu * pmnuHelp = new QMenu("&Help");
    pmnuHelp->addAction("&About",this,SLOT(slotAbout()),Qt::Key_F1);
    menuBar()->addMenu(pmnuHelp);

    //------------------------------------------------
    m_psigmapper = new QSignalMapper(this);
    connect(m_psigmapper,SIGNAL(mapped(QWidget*)),this,SLOT(slotSetActiveSubWindow(QWidget*)));
    //------------------------------------------------
    //------------------------------------------------
    QToolBar * tbr =new QToolBar("Top tool");
    tbr->addAction(pacNewDoc);
    tbr->addAction(pacOpen);
    tbr->addAction(pacSave);
    tbr->addAction(pacConfig);

    this->addToolBar(tbr);

}

//--------------------------------------------------------------------------------------------------------------------------------
///
/// \brief MainWindow::slotNewDoc
///
void MainWindow::slotNewDoc()
{
    QWidget *pdoc=new QWidget;
    pdoc->setAttribute(Qt::WA_DeleteOnClose);
    pdoc->setWindowTitle("Unnamed document");
    pdoc->setWindowIcon(QPixmap(":/store/images/sc_newdoc"));
    ui->mdiArea->addSubWindow(pdoc);
    pdoc->show();
}

//--------------------------------------------------------------------------------------------------------------------------------
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
