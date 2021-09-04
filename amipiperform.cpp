#include "amipiperform.h"
#include "ui_amipiperform.h"

//--------------------------------------------------------------------------------------------------------------------
AmiPiperForm::AmiPiperForm(modelMarketsList *modelM, int DefaultTickerMarket,
                           modelTickersList *modelT,
                           AmiPipeHolder & p,
                           QWidget *parent) :
    QWidget(parent),
    iDefaultTickerMarket{DefaultTickerMarket},
    modelMarket{modelM},
    modelTicker{modelT},
    pipes{p},
    ui(new Ui::AmiPiperForm)
{
    ui->setupUi(this);
    ///////////////////////////////////////////////////////////////////////
    /// \brief connect

    connect(ui->btnCheck,SIGNAL(clicked()),this,SLOT(btnCheckClicked()));

}
//--------------------------------------------------------------------------------------------------------------------
AmiPiperForm::~AmiPiperForm()
{
    delete ui;
}
//--------------------------------------------------------------------------------------------------------------------
void AmiPiperForm::btnCheckClicked()
{
    auto lstPipes = pipes.CheckActivePipes();

    for (const auto & e:lstPipes){
        ui->textEdit->append(QString::fromStdString(e));
    }


    emit SendToMainLog("ami clicked");
}
