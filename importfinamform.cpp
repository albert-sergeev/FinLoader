#include "importfinamform.h"
#include "ui_importfinamform.h"

#include<QFileDialog>
#include<QDebug>

#include<iostream>
#include<filesystem>
#include<iostream>
#include<fstream>
#include<ostream>

#include "storage.h"

ImportFinamForm::ImportFinamForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ImportFinamForm)
{
    ui->setupUi(this);
    //

    connect(ui->btnOpen,SIGNAL(clicked()),this,SLOT(slotBtnOpenClicked()));
    connect(ui->btnCreate,SIGNAL(clicked()),this,SLOT(slotBtnCreateClicked()));
    connect(ui->btnImport,SIGNAL(clicked()),this,SLOT(slotBtnImportClicked()));
    connect(ui->btnTest,SIGNAL(clicked()),this,SLOT(slotBtnTestClicked()));


    connect(ui->edDelimiter,SIGNAL(textChanged(const QString &)),this,SLOT(slotEditDelimiterWgtChanged(const QString &)));

}
//--------------------------------------------------------------------------------------------------------
ImportFinamForm::~ImportFinamForm()
{
    delete ui;
}
//--------------------------------------------------------------------------------------------------------
void ImportFinamForm::SetDefaultOpenDir(QString &s)
{
    pathDir = s.toStdString();
}
//--------------------------------------------------------------------------------------------------------
void ImportFinamForm::slotBtnOpenClicked()
{

    QString str=QFileDialog::getOpenFileName(0,"Open file to import",
                                             QString::fromStdString(pathDir.string()),
                                             "*.txt");
    if (str.size()>0){
        pathFile = str.toStdString();
        if(std::filesystem::exists(pathFile)){
            //
            pathDir=pathFile.parent_path();
            QString qsDir=QString::fromStdString(pathDir.string());;
            OpenImportFilePathChanged(qsDir);
            //
            QString strFileToShow = QString::fromStdString(pathFile.filename().string());
            ui->edFileName->setText(strFileToShow);
            //
            slotPreparseImportFile();
        }
    }
}

//--------------------------------------------------------------------------------------------------------
void ImportFinamForm::slotBtnCreateClicked(){};
void ImportFinamForm::slotBtnImportClicked(){};
//--------------------------------------------------------------------------------------------------------
void ImportFinamForm::slotBtnTestClicked()
{
    slotPreparseImportFile();
};
//--------------------------------------------------------------------------------------------------------
void ImportFinamForm::SetDelimiter(char c)
{
    cDelimiter = c;
    std::string s{" "};
    s[0] = c;
    ui->edDelimiter->setText(QString::fromStdString(s));
}
//--------------------------------------------------------------------------------------------------------
void ImportFinamForm::slotEditDelimiterWgtChanged(const QString &)
{
    std::string sD = ui->edDelimiter->text().toStdString();
    if(sD.size()<=0){
        cDelimiter = ' ';
    }
    else{
        cDelimiter = sD[0];
    }
    ///////////////
    emit DelimiterHasChanged(cDelimiter);
}
//--------------------------------------------------------------------------------------------------------
// preliminary check import file. Lookup for Ticker sign, time periods etc.
void ImportFinamForm::slotPreparseImportFile()
{
    ///////////////

    if(std::filesystem::exists(pathFile)    &&
       std::filesystem::is_regular_file(pathFile)
            ){
        std::ifstream file(pathFile);
        if(file.good()){

            std::string sBuff;
            std::istringstream iss;

            std::stringstream oss;

            if (std::getline(file,sBuff)) {
                // link stringstream
                iss.clear();
                iss.str(sBuff);
                //
                //std::vector<std::string> vS{std::istream_iterator<StringDelimiter<cDelim>>{iss},{}};
                std::vector<std::string> vS;//{std::istream_iterator<StringDelimiter<cDelim>>{iss},{}};
                std::string sWordBuff;
                while (std::getline(iss,sWordBuff,cDelimiter)){
                    vS.push_back(sWordBuff);
                }

                for(const auto &v:vS){
                    oss<<v<<" | ";
                }
                ui->edText->append(QString::fromStdString(oss.str()));

            }

        }
    }
}
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
