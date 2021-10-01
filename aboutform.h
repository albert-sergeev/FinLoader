#ifndef ABOUTFORM_H
#define ABOUTFORM_H

#include <QWidget>

namespace Ui {
class AboutForm;
}

class AboutForm : public QWidget
{
    Q_OBJECT

public:
    explicit AboutForm(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    ~AboutForm();

protected slots:

    void slotQuit();
    void slotLicense();

private:
    Ui::AboutForm *ui;
};

#endif // ABOUTFORM_H
