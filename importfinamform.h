#ifndef IMPORTFINAMFORM_H
#define IMPORTFINAMFORM_H

#include <QWidget>

namespace Ui {
class ImportFinamForm;
}

class ImportFinamForm : public QWidget
{
    Q_OBJECT

public:
    explicit ImportFinamForm(QWidget *parent = nullptr);
    ~ImportFinamForm();

private:
    Ui::ImportFinamForm *ui;
};

#endif // IMPORTFINAMFORM_H
