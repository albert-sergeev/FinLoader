#include "importfinamform.h"
#include "ui_importfinamform.h"

ImportFinamForm::ImportFinamForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ImportFinamForm)
{
    ui->setupUi(this);
}

ImportFinamForm::~ImportFinamForm()
{
    delete ui;
}
