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

    connect(ui->btnOpen,SIGNAL(pressed()),this,SLOT(slotBtnOpenClicked()));

    connect(ui->chkShowByName,SIGNAL(stateChanged(int)),this,SLOT(slotShowByNamesChecked(int)));



    connect(ui->edDelimiter,SIGNAL(textChanged(const QString &)),this,SLOT(slotEditDelimiterWgtChanged(const QString &)));

    clearShowAreaOfFields();
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
    // <TICKER> | <PER> | <DATE> | <TIME> | <LAST> | <VOL>
    // <TICKER> | <PER> | <DATE> | <TIME> | <OPEN> | <HIGH> | <LOW> | <CLOSE> | <VOL>

    clearShowAreaOfFields();

    const int fieldMax{9};
    std::vector<int> fields =
                        {   fieldType::TICKER,
                            fieldType::PER,
                            fieldType::DATE,
                            fieldType::TIME,
                            fieldType::OPEN,
                            fieldType::HIGH,
                            fieldType::LOW,
                            fieldType::CLOSE,
                            fieldType::VOL
                        };


    std::string sFinamSign;
    std::istringstream signstream;
    signstream.str(pathFile.filename().string());
    std::getline(signstream,sFinamSign,'_');

    ui->edText->append( "====================================================\n");

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

                std::vector<std::string> vS;//{std::istream_iterator<StringDelimiter<cDelim>>{iss},{}};
                std::string sWordBuff;
                int iN{0};
                //int IntervalDefault(Bar::eInterval::pTick);
                int IntervalDefault(-1);


                bool bWasHeader{true};
                int iFieldMask{0};

                /////////////////////////////////////////////////////////////
                /////////////////////////////////////////////////////////////
                // parse header section
                /////////////////////////////////////////////////////////////
                /////////////////////////////////////////////////////////////

                while (std::getline(iss,sWordBuff,cDelimiter)){
                    trim(sWordBuff);
                    if      (bWasHeader && sWordBuff == "<TICKER>") {   fields[iN] = fieldType::TICKER; }
                    else if (bWasHeader && sWordBuff == "<PER>")    {   fields[iN] = fieldType::PER;    IntervalDefault = -1;}
                    else if (bWasHeader && sWordBuff == "<DATE>")   {   fields[iN] = fieldType::DATE;   iFieldMask ^= 1;}
                    else if (bWasHeader && sWordBuff == "<TIME>")   {   fields[iN] = fieldType::TIME;   iFieldMask ^= 2;}
                    else if (bWasHeader && sWordBuff == "<OPEN>")   {   fields[iN] = fieldType::OPEN;   iFieldMask ^= 4;}
                    else if (bWasHeader && sWordBuff == "<HIGH>")   {   fields[iN] = fieldType::HIGH;   iFieldMask ^= 8;}
                    else if (bWasHeader && sWordBuff == "<LOW>")    {   fields[iN] = fieldType::LOW;    iFieldMask ^= 16;}
                    else if (bWasHeader && sWordBuff == "<CLOSE>")  {   fields[iN] = fieldType::CLOSE;  iFieldMask ^= 32;}
                    else if (bWasHeader && sWordBuff == "<VOL>")    {   fields[iN] = fieldType::VOL;    iFieldMask ^= 64;}
                    else if (bWasHeader && sWordBuff == "<LAST>")   {   fields[iN] = fieldType::LAST;   iFieldMask ^= 128;}
                    else {
                        if(iN == 0 || iN == 1){ // first column is sign or period
                            for(const unsigned char ch:sWordBuff){
                                if (!std::isalnum(ch)){
                                    ui->edText->append(QString::fromStdString(sBuff));
                                    ui->edText->append("Wrong file format");
                                    return;
                                }
                            }
                        }
                        else{
                            try{
                                std::stod(sWordBuff);
                            }
                            catch (std::exception &e){
                                ui->edText->append(QString::fromStdString(sBuff));
                                ui->edText->append("Wrong file format");
                                return;
                                }
                        }
                        //
                        bWasHeader = false;
                        //break;
                    }
                    vS.push_back(sWordBuff);
                    iN++;
                    if (iN > fieldMax){
                        ui->edText->append(QString::fromStdString(sBuff));
                        ui->edText->append("Wrong file format - too many fields");
                        return;
                    }
                }
                /////////////////////////////////////////////////////////////
                /////////////////////////////////////////////////////////////
                // analize header section
                /////////////////////////////////////////////////////////////
                /////////////////////////////////////////////////////////////

                //oss << "====================================================\n";

                if (bWasHeader){
                    if (iFieldMask != 127 && iFieldMask != 195) {
                        ui->edText->append(QString::fromStdString(sBuff));
                        ui->edText->append("Wrong file format: missing fields!");
                        return;
                    }

                    for(const auto &v:vS){
                        oss<<v<<cDelimiter;
                    }
                    oss <<"\n";
                }
                else{
                    if (iN == 9){
                        ;
                    }
                    else if (iN == 6){
                        fields[4] = fieldType::LAST;
                        fields[5] = fieldType::VOL;
                        IntervalDefault = Bar::eInterval::pTick;
                    }
                    else{
                        ui->edText->append(QString::fromStdString(sBuff));
                        ui->edText->append("Wrong file format: wrong number of fields");
                        return;
                    }

                }

                /////////////////////////////////////////////////////////////
                /////////////////////////////////////////////////////////////
                // analize first rows section
                /////////////////////////////////////////////////////////////
                /////////////////////////////////////////////////////////////

                //===================
                if (bWasHeader){
                    if (std::getline(file,sBuff)) {
                        // link stringstream
                        iss.clear();
                        iss.str(sBuff);
                    }
                    else{
                        ui->edText->append("Wrong file format: too fiew rows");
                        return;
                    }
                }
                else{
                    iss.clear();
                    iss.str(sBuff);
                }
                //===================
                Bar bb(0,0,0,0,0,0);
                {
                    std::istringstream issTmp;
                    std::ostringstream ossErr;
                    if (!slotParseLine(fields, iss, issTmp,ossErr, bb, iN, IntervalDefault)){
                        ui->edText->append(QString::fromStdString(iss.str()));
                        ui->edText->append(QString::fromStdString(ossErr.str()));
                        return;
                    }
                }
                ui->edOpen->setText(QString::number(bb.Open()));
                ui->edHigh->setText(QString::number(bb.High()));
                ui->edLow->setText(QString::number(bb.Low()));
                ui->edClose->setText(QString::number(bb.Close()));
                ui->edVolume->setText(QString::number(bb.Volume()));
                showInterval(bb.Interval());

                {
                    std::time_t tS (bb.Period());
                    std::tm* tmSt=localtime(&tS);
                    const QDate dtS(tmSt->tm_year,tmSt->tm_mon,tmSt->tm_mday);
                    const QTime tmS(tmSt->tm_hour,tmSt->tm_min,tmSt->tm_sec);
                    QDateTime dt (dtS,tmS);

                    ui->dtStart->setDateTime(dt);
                }
                if (IntervalDefault <0 ) IntervalDefault = bb.Interval();
                //
                oss<<sBuff;
                oss<<"...\n";

                /////////////////////////////////////////////////////////////
                /////////////////////////////////////////////////////////////
                // analize last rows section
                /////////////////////////////////////////////////////////////
                /////////////////////////////////////////////////////////////


                file.seekg(0,std::ios::end);
                size_t filesize = file.tellg();
                filesize = filesize > 500 ? filesize - 500: 0 ;
                file.seekg(filesize, std::ios::beg);
                //
                std::string sOldLine{""};

                while (std::getline(file,sBuff)) {
                    sOldLine = sBuff;
                }
                // link stringstream
                iss.clear();
                iss.str(sOldLine);

                {
                    std::istringstream issTmp;
                    std::ostringstream ossErr;
                    if (!slotParseLine(fields, iss, issTmp,ossErr, bb, iN, IntervalDefault)){
                        ui->edText->append(QString::fromStdString(iss.str()));
                        ui->edText->append(QString::fromStdString(ossErr.str()));
                        return;
                    }
                }

                {
                    std::time_t tS (bb.Period());
                    std::tm* tmSt=localtime(&tS);
                    const QDate dtS(tmSt->tm_year,tmSt->tm_mon,tmSt->tm_mday);
                    const QTime tmS(tmSt->tm_hour,tmSt->tm_min,tmSt->tm_sec);
                    QDateTime dt (dtS,tmS);

                    ui->dtEnd->setDateTime(dt);
                }
                oss<<sOldLine;
                oss<<"\n\r";
                QString qsT{tr("Preliminary check was successfull\n")};
                oss<<qsT.toStdString();
                oss<<"filename: "<<pathFile.filename().string();

                //////////////////////
                ui->edText->append(QString::fromStdString(oss.str()));
                ui->edSign->setText("<"+QString::fromStdString(sFinamSign)+">");

                if(bb.Interval() == Bar::eInterval::pTick){
                    ui->btnImport->setText("Import");
                }
                else{
                    ui->btnImport->setText("Check");
                }

            }
            else
                ui->edText->append("Wrong file format: no lines");
        }
        else
            ui->edText->append("Wrong file format: bad file");
    }
    else
        ui->edText->append("Wrong file format: bad path to file");

}
//--------------------------------------------------------------------------------------------------------
void ImportFinamForm::clearShowAreaOfFields()
{
    ui->edOpen->setText("");
    ui->edHigh->setText("");
    ui->edLow->setText("");
    ui->edClose->setText("");
    ui->edVolume->setText("");

    const QDate dtS(1990,1,1);
    const QTime tmS(0,0,0);
    QDateTime dt (dtS,tmS);

    ui->dtStart->setDateTime(dt);
    ui->dtEnd->setDateTime(dt);

    showInterval(-1);
}
//--------------------------------------------------------------------------------------------------------
void ImportFinamForm::showInterval(int Interval)
{
    switch (Interval) {
    case Bar::eInterval::pTick:
        ui->rbtnTick->setChecked(true);
        break;
    case Bar::eInterval::p1:
        ui->rbtn1->setChecked(true);
        break;
    case Bar::eInterval::p5:
        ui->rbtn5->setChecked(true);
        break;
    case Bar::eInterval::p10:
        ui->rbtn10->setChecked(true);
        break;
    case Bar::eInterval::p15:
        ui->rbtn15->setChecked(true);
        break;
    case Bar::eInterval::p30:
        ui->rbtn30->setChecked(true);
        break;
    case Bar::eInterval::p60:
        ui->rbtn60->setChecked(true);
        break;
    case Bar::eInterval::p120:
        ui->rbtn120->setChecked(true);
        break;
    case Bar::eInterval::p180:
        ui->rbtn180->setChecked(true);
        break;
    case Bar::eInterval::pDay:
        ui->rbtnDay->setChecked(true);
        break;
    case Bar::eInterval::pWeek:
        ui->rbtnWeek->setChecked(true);
        break;
    case Bar::eInterval::pMonth:
        ui->rbtnMonth->setChecked(true);
        break;
    default:
        ;
    }
}

//--------------------------------------------------------------------------------------------------------
bool ImportFinamForm::slotParseLine(std::vector<int> & fieldsType, std::istringstream & issLine, std::istringstream & iss, std::ostringstream & ossErr, Bar &b, int ColMax, int DefaultInterval)
{

    std::string sWordBuff;
    std::string sSign;
    int iInterval;
    //std::istringstream iss; // stringstream needed becouse locale mismatch in std::stod (Qt and STL fight)
    double dTmp;

    std::string sYear{"1990"};
    std::string sMonth{"01"};
    std::string sDay{"01"};

    std::string sHour{"00"};
    std::string sMin{"00"};
    std::string sSec{"00"};

    std::tm tp;
    tp.tm_isdst = 0;

    int iCurrN{0};
    try{
        while (std::getline(issLine,sWordBuff,cDelimiter)){
            trim(sWordBuff);
            iss.clear();
            iss.str(sWordBuff);

            switch(fieldsType[iCurrN]){
            case fieldType::TICKER:
                sSign = sWordBuff;
                break;
            case fieldType::PER:
                if(sWordBuff == "day"){
                    iInterval = Bar::eInterval::pDay;
                }
                else{
                    iInterval = std::stoi(sWordBuff);
                    if(             iInterval != Bar::eInterval::pTick
                                &&  iInterval != Bar::eInterval::p1
                                &&  iInterval != Bar::eInterval::p5
                                &&  iInterval != Bar::eInterval::p10
                                &&  iInterval != Bar::eInterval::p15
                                &&  iInterval != Bar::eInterval::p30
                                &&  iInterval != Bar::eInterval::p60
                                &&  iInterval != Bar::eInterval::p120
                                &&  iInterval != Bar::eInterval::p180
                            ){
                        ossErr << "Wrong file format: wrong period field value";
                        return false;
                    }
                }
                if (DefaultInterval >= 0){
                    if(DefaultInterval != iInterval){
                        ossErr << "Interval miscast";
                        return false;
                    }
                }
                b.initInterval(iInterval);
                break;
            case fieldType::DATE:
                //20210322,
                if(sWordBuff.size()>=8){
                    copy(sWordBuff.begin()    ,sWordBuff.begin() + 4,sYear.begin());
                    copy(sWordBuff.begin() + 4,sWordBuff.begin() + 6,sMonth.begin());
                    copy(sWordBuff.begin() + 6,sWordBuff.begin() + 8,sDay.begin());
                    tp.tm_year = std::stoi(sYear);
                    tp.tm_mon = std::stoi(sMonth);
                    tp.tm_mday = std::stoi(sDay);
                }
                else{
                    ossErr << "Wrong file format: wrong date field value";
                    return false;
                }
                break;
            case fieldType::TIME:
                //095936
                if(sWordBuff.size()>=6){
                    copy(sWordBuff.begin()    ,sWordBuff.begin() + 2,sHour.begin());
                    copy(sWordBuff.begin() + 2,sWordBuff.begin() + 4,sMin.begin());
                    copy(sWordBuff.begin() + 4,sWordBuff.begin() + 6,sSec.begin());
                    tp.tm_hour = std::stoi(sHour);
                    tp.tm_min = std::stoi(sMin);
                    tp.tm_sec = std::stoi(sSec);
                }
                else{
                    ossErr << "Wrong file format: wrong time field value";
                    return false;
                }

                break;
            case fieldType::OPEN:
                iss >> dTmp; b.setOpen (dTmp);
                break;
            case fieldType::HIGH:
                iss >> dTmp; b.setHigh (dTmp);
                break;
            case fieldType::LOW:
                iss >> dTmp; b.setLow (dTmp);
                break;
            case fieldType::CLOSE:
                iss >> dTmp; b.setClose (dTmp);
                break;
            case fieldType::LAST:
                iss >> dTmp;
                b.setOpen   (dTmp);
                b.setHigh   (b.Open());
                b.setLow    (b.Open());
                b.setClose  (b.Open());
                break;
            case fieldType::VOL:
                b.setVolume (std::stoi(sWordBuff));
                break;
            default:
                ossErr << "Wrong file format: column parsing";
                return false;
                break;
            }
            iCurrN++;
            if (iCurrN > ColMax){
                ossErr << "Wrong file format: not equal column count";
                return false;
            }
        }
        b.setPeriod(std::mktime(&tp));
    }
    catch (std::exception &e){
        ossErr << "Wrong file format";
        return false;
        }

    return  true;
}


//--------------------------------------------------------------------------------------------------------
void ImportFinamForm::setMarketModel(MarketsListModel *model, int DefaultTickerMarket)
{
    modelMarket = model;

    //ui->viewTickets
    ui->cmbMarket->setModel(modelMarket);

    {

        connect(ui->cmbMarket,SIGNAL(activated(const int)),this,SLOT(slotSetSelectedTickersMarket(const  int)));

        bool bWas{false};
        for(int i = 0; i < modelMarket->rowCount(); i++){
            auto idx(modelMarket->index(i,0));
            if(idx.isValid()){
                if(modelMarket->getMarket(idx).MarketID() == DefaultTickerMarket){
                    //iDefaultTickerMarket = DefaultTickerMarket;
                    ui->cmbMarket->setCurrentIndex(i);
                    slotSetSelectedTickersMarket(i);
                    bWas = true;
                    break;
                }
            }
        }
        if (!bWas){
            auto idx(modelMarket->index(0,0));
            if(idx.isValid()){
                iDefaultTickerMarket = modelMarket->getMarket(idx).MarketID();
                slotSetSelectedTickersMarket(0);
            }
        }
    }
    ////////////////////////////////////////////////////

}
//--------------------------------------------------------------------------------------------------------

void ImportFinamForm::slotSetSelectedTickersMarket(const  int i)
{
    //qDebug()<<"i: {"<<i<<"}";
    if(i < modelMarket->rowCount()){
        auto idx(modelMarket->index(i,0));
        if(idx.isValid()){
            if(modelMarket->getMarket(idx).MarketID() != iDefaultTickerMarket){
                iDefaultTickerMarket = modelMarket->getMarket(idx).MarketID();
                //NeedSaveDefaultTickerMarket(iDefaultTickerMarket);
                proxyTickerModel.setDefaultMarket(iDefaultTickerMarket);
                // clear
                //ClearTickerWidgetsValues();
                //setEnableTickerWidgets(true);
                //slotTickerDataChanged(false);
                // sel first item
                QItemSelectionModel  *qml =new QItemSelectionModel(&proxyTickerModel);
                auto first_i(proxyTickerModel.index(0,0));
                if(first_i.isValid()){
                    qml->select(first_i,QItemSelectionModel::SelectionFlag::Select) ;
                    //slotSetSelectedTicker(first_i);
                    ui->viewTickers->setFocus();
                }

            }
        }
    }
}
//--------------------------------------------------------------------------------------------------------
void ImportFinamForm::setTickerModel(TickersListModel *model,bool /*ShowByName*/,bool /*SortByName*/)
{

    modelTicker = model;
    proxyTickerModel.setDefaultMarket(iDefaultTickerMarket);
    proxyTickerModel.setSourceModel(model);
    ui->viewTickers->setModel(&proxyTickerModel);
    proxyTickerModel.sort(2);

    slotShowByNamesChecked(ui->chkShowByName->isChecked());

    connect(ui->viewTickers,SIGNAL(clicked(const QModelIndex&)),this,SLOT(slotSetSelectedTicker(const  QModelIndex&)));

    /////////

    QItemSelectionModel  *qml =new QItemSelectionModel(&proxyTickerModel);
    //QItemSelectionModel  *qml =new QItemSelectionModel(model);
    ui->viewTickers->setSelectionModel(qml);


    connect(qml,SIGNAL(currentRowChanged(const QModelIndex&,const QModelIndex&)),this,SLOT(slotSetSelectedTicker(const  QModelIndex&,const QModelIndex&)));


    auto first_i(proxyTickerModel.index(0,0));
    //auto first_i(model->index(0,0));
    if(first_i.isValid()){
        qml->select(first_i,QItemSelectionModel::SelectionFlag::Select) ;
        slotSetSelectedTicker(first_i);
    }
    else{
//        slotTickerDataChanged(false);
        //
//        setEnableTickerWidgets(false);
    }
    ////
}
//--------------------------------------------------------------------------------------------------------
void ImportFinamForm::slotSetSelectedTicker(const  QModelIndex& indx)
{


    //qDebug()<<"Enter slotSetSelectedMarket";

    if (indx.isValid()){
        //const Ticker& t=modelTicker->getTicker(indx);
        const Ticker& t=proxyTickerModel.getTicker(indx);

        ui->edSign->setText(QString::fromStdString(t.TickerSign())+" {"+QString::fromStdString(t.TickerSignFinam())+"}");

    }

}
//--------------------------------------------------------------------------------------------------------
void ImportFinamForm::slotShowByNamesChecked(int Checked)
{
    if (Checked){
        ui->viewTickers->setModelColumn(0);
    }
    else{
        ui->viewTickers->setModelColumn(2);
    }
    proxyTickerModel.invalidate();
    //NeedSaveShowByNames(Checked);
}

//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
