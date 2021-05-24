#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    QMenu* m_mnuWindows;
    QSignalMapper * m_psigmapper;
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void InitAction();

public slots:
    void slotNotImpl    ();
    void slotNewDoc     ();
    void slotWindows    ();
    void slotAbout      ();
    void slotSetActiveSubWindow (QWidget*);



private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
