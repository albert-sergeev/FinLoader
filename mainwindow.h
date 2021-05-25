#ifndef MAINWINDOW_H
#define MAINWINDOW_H

//#include <QtWidgets>
#include <QMainWindow>
#include <QSignalMapper>
#include <QTranslator>
#include <QSettings>
#include <QStyle>
#include <QDebug>
#include <QToolBar>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QMdiSubWindow>
#include <QMessageBox>
#include <QStyleFactory>
#include <QFile>
#include <QTextEdit>
//#include <>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    QMenu * m_mnuWindows;
    QMenu * m_mnuStyles;
    QMenu * m_mnuLangs;
    QSignalMapper * m_psigmapper;
    QSignalMapper * m_psigmapperStyle;
    QSignalMapper * m_psigmapperLang;
    QList<QString> lstStyles;
    QString m_sStyleName;
    QString m_Language;
    QTranslator m_translator;

    QSettings m_settings;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void addToLog(QString);

protected:
    void InitAction();
    void SaveSettings();
    void LoadSettings();

public slots:
    void slotNotImpl    ();
    void slotNewDoc     ();
    void slotNewLogWnd  ();
    void slotWindows    ();
    void slotStyles     ();
    void slotLanguages  ();
    void slotAbout      ();
    void slotSendTestText();
    void slotSetActiveSubWindow (QWidget*);
    void slotSetActiveStyle     (QString);
    void slotSetActiveLang      (QString);



private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
