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

    ui->progressBar->setValue(0);

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
void ImportFinamForm::slotBtnImportClicked()
{
    //                    Ticker t {proxyTickerModel.getTicker(indxProxyT)};
    //                    iSelectedTickerId = t.TickerID();
    //                    if (t.TickerSignFinam().size() == 0){

    //                        t.SetTickerSignFinam(parseDt.Sign());
    //                        proxyTickerModel.setData(indxProxyT,t,Qt::EditRole);

    //                    }
};
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
    ////////////////////////////////////////////////////////////////////////////////////
    // <TICKER> | <PER> | <DATE> | <TIME> | <LAST> | <VOL>
    // <TICKER> | <PER> | <DATE> | <TIME> | <OPEN> | <HIGH> | <LOW> | <CLOSE> | <VOL>

    clearShowAreaOfFields();
    ui->edText->append( "====================================================\n");
    ////////////////////////////////////////////////////////////////////////////////////

    iSelectedTickerId = 0;
    sSelectedTickerSignFinam = "";


    std::string sBuff;
    std::istringstream iss;
    std::stringstream oss;
    std::istringstream issTmp;
    std::ostringstream ossErr;

    finamParseData parseDt(&issTmp,&ossErr);

    parseDt.initDefaultFieldsValues(9);

    {
        std::string sSign;
        std::istringstream signstream;
        signstream.str(pathFile.filename().string());
        std::getline(signstream,sSign,'_');
        parseDt.setDefaultSign(sSign);
    }



    if(std::filesystem::exists(pathFile)    &&
       std::filesystem::is_regular_file(pathFile)
            ){
        std::ifstream file(pathFile);

        if(file.good()){



            if (std::getline(file,sBuff)) {
                // link stringstream
                iss.clear();
                iss.str(sBuff);
                //

                std::vector<std::string> vS;//{std::istream_iterator<StringDelimiter<cDelim>>{iss},{}};
                std::string sWordBuff;
                int iN{0};




                bool bWasHeader{true};
                int iFieldMask{0};

                /////////////////////////////////////////////////////////////
                /////////////////////////////////////////////////////////////
                // parse header section
                /////////////////////////////////////////////////////////////
                /////////////////////////////////////////////////////////////

                while (std::getline(iss,sWordBuff,cDelimiter)){
                    trim(sWordBuff);
                    if      (bWasHeader && sWordBuff == "<TICKER>") {   parseDt.fields()[iN] = finamParseData::fieldType::TICKER; }
                    else if (bWasHeader && sWordBuff == "<PER>")    {   parseDt.fields()[iN] = finamParseData::fieldType::PER;    }
                    else if (bWasHeader && sWordBuff == "<DATE>")   {   parseDt.fields()[iN] = finamParseData::fieldType::DATE;   iFieldMask ^= 1;}
                    else if (bWasHeader && sWordBuff == "<TIME>")   {   parseDt.fields()[iN] = finamParseData::fieldType::TIME;   iFieldMask ^= 2;}
                    else if (bWasHeader && sWordBuff == "<OPEN>")   {   parseDt.fields()[iN] = finamParseData::fieldType::OPEN;   iFieldMask ^= 4;}
                    else if (bWasHeader && sWordBuff == "<HIGH>")   {   parseDt.fields()[iN] = finamParseData::fieldType::HIGH;   iFieldMask ^= 8;}
                    else if (bWasHeader && sWordBuff == "<LOW>")    {   parseDt.fields()[iN] = finamParseData::fieldType::LOW;    iFieldMask ^= 16;}
                    else if (bWasHeader && sWordBuff == "<CLOSE>")  {   parseDt.fields()[iN] = finamParseData::fieldType::CLOSE;  iFieldMask ^= 32;}
                    else if (bWasHeader && sWordBuff == "<VOL>")    {   parseDt.fields()[iN] = finamParseData::fieldType::VOL;    iFieldMask ^= 64;}
                    else if (bWasHeader && sWordBuff == "<LAST>")   {   parseDt.fields()[iN] = finamParseData::fieldType::LAST;   iFieldMask ^= 128;}
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
                    if (iN > parseDt.ColMax()){
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

                    oss<<vS[0];
                    std::accumulate(next(vS.begin()),vS.end(),0,[&](auto &ss, const auto c){ oss<<" | "<<c; return  ss;});
                    oss <<"\n";
                }
                else{
                    if (!parseDt.initDefaultFieldsValues(iN)){
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
                if (!slotParseLine(parseDt, iss, bb)){
                    ui->edText->append(QString::fromStdString(iss.str()));
                    ui->edText->append(QString::fromStdString(ossErr.str()));
                    return;
                }

                showInterval(bb.Interval());

                {
                    std::time_t tS (bb.Period());
                    std::tm* tmSt=localtime(&tS);
                    const QDate dtS(tmSt->tm_year,tmSt->tm_mon,tmSt->tm_mday);
                    const QTime tmS(tmSt->tm_hour,tmSt->tm_min,tmSt->tm_sec);
                    QDateTime dt (dtS,tmS);

                    ui->dtStart->setDateTime(dt);
                }

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

                if (!slotParseLine(parseDt, iss, bb)){
                    ui->edText->append(QString::fromStdString(iss.str()));
                    ui->edText->append(QString::fromStdString(ossErr.str()));
                    return;
                }

                ui->edOpen->setText(QString::number(bb.Open()));
                ui->edHigh->setText(QString::number(bb.High()));
                ui->edLow->setText(QString::number(bb.Low()));
                ui->edClose->setText(QString::number(bb.Close()));
                ui->edVolume->setText(QString::number(bb.Volume()));

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
                oss<<"filename: "<<pathFile.filename().string()<<"\n";

                /////////////////////////////////////////////////////////////
                /////////////////////////////////////////////////////////////
                // assign to ticker section
                /////////////////////////////////////////////////////////////
                /////////////////////////////////////////////////////////////
                sSelectedTickerSignFinam = parseDt.Sign();

                QModelIndex indxM;
                QModelIndex indxT;
                QModelIndex indxProxyT;
                bool bFound{false};
                bool bFoundFinam{false};
                if(     modelTicker->searchTickerByFinamSign(parseDt.Sign(), indxT)){
                    bFoundFinam = true;
                }
                else if(modelTicker->searchTickerBySign(parseDt.Sign(), indxT)){
                    bFound = true;
                }
                //
                if((bFound || bFoundFinam ) && indxT.isValid()){
                    const Ticker &t {modelTicker->getTicker(indxT)};

                    if(!modelMarket->searchMarketByMarketID(t.MarketID(), indxM)){
                        ui->edText->append(QString::fromStdString(sBuff));
                        ui->edText->append("Data integrity was broken! Ticker without market!");
                        return;
                    }

                    ui->cmbMarket->setCurrentIndex(indxM.row());
                    slotSetSelectedTickersMarket(indxM.row());

                    indxProxyT = proxyTickerModel.mapFromSource(indxT);


                    QItemSelectionModel  *qml = ui->viewTickers->selectionModel();

                    ui->viewTickers->setCurrentIndex(indxProxyT);
                    qml->select(indxProxyT,QItemSelectionModel::SelectionFlag::Select | QItemSelectionModel::Rows| QItemSelectionModel::Current) ;
                    //slotSetSelectedTicker(indxProxyT);

                    iSelectedTickerId = t.TickerID();

                    ui->edSign->setText(QString::fromStdString(t.TickerSign()+" {"+sSelectedTickerSignFinam+"}"));
                    ui->viewTickers->setFocus();
                }
                else{
                    ui->edSign->setText(QString::fromStdString("{"+sSelectedTickerSignFinam+"}"));
                }

                //////////////////////
                ui->edText->append(QString::fromStdString(oss.str()));



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
    ui->edInterval->setText("");

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
        ui->edInterval->setText("Tick");
        break;
    case Bar::eInterval::p1:
        ui->edInterval->setText("1 min");
        break;
    case Bar::eInterval::p5:
        ui->edInterval->setText("5 min");
        break;
    case Bar::eInterval::p10:
        ui->edInterval->setText("10 min");
        break;
    case Bar::eInterval::p15:
        ui->edInterval->setText("15 min");
        break;
    case Bar::eInterval::p30:
        ui->edInterval->setText("30 min");
        break;
    case Bar::eInterval::p60:
        ui->edInterval->setText("60 min");
        break;
    case Bar::eInterval::p120:
        ui->edInterval->setText("120 min");
        break;
    case Bar::eInterval::p180:
        ui->edInterval->setText("180 min");
        break;
    case Bar::eInterval::pDay:
        ui->edInterval->setText("Day");
        break;
    case Bar::eInterval::pWeek:
        ui->edInterval->setText("Week");
        break;
    case Bar::eInterval::pMonth:
        ui->edInterval->setText("Month");
        break;
    default:
        ;
    }
}

//--------------------------------------------------------------------------------------------------------
bool ImportFinamForm::slotParseLine(finamParseData & parseDt, std::istringstream & issLine, Bar &b)
{

    parseDt.t_iCurrN = 0;

    try{
        while (std::getline(issLine,parseDt.t_sWordBuff,cDelimiter)){
            trim(parseDt.t_sWordBuff);
            parseDt.issTmp().clear();
            parseDt.issTmp().str(parseDt.t_sWordBuff);

            switch(parseDt.fields()[parseDt.t_iCurrN]){
            case finamParseData::fieldType::TICKER:
                parseDt.t_sSign = parseDt.t_sWordBuff;
                if (parseDt.Sign().size() ==0){
                    parseDt.setDefaultSign(parseDt.t_sSign);
                }
                else if(parseDt.Sign() != parseDt.t_sSign){
                    parseDt.ossErr() << "Ticker sign mismatch";
                    return false;
                }
                break;
            case finamParseData::fieldType::PER:
                if(parseDt.t_sWordBuff == "day"){
                    parseDt.t_iInterval = Bar::eInterval::pDay;
                }
                else if(parseDt.t_sWordBuff == "week"){
                    parseDt.t_iInterval = Bar::eInterval::pWeek;
                }
                else if(parseDt.t_sWordBuff == "month"){
                    parseDt.t_iInterval = Bar::eInterval::pMonth;
                }
                else{
                    parseDt.t_iInterval = std::stoi(parseDt.t_sWordBuff);
                    if(             parseDt.t_iInterval != Bar::eInterval::pTick
                                &&  parseDt.t_iInterval != Bar::eInterval::p1
                                &&  parseDt.t_iInterval != Bar::eInterval::p5
                                &&  parseDt.t_iInterval != Bar::eInterval::p10
                                &&  parseDt.t_iInterval != Bar::eInterval::p15
                                &&  parseDt.t_iInterval != Bar::eInterval::p30
                                &&  parseDt.t_iInterval != Bar::eInterval::p60
                                &&  parseDt.t_iInterval != Bar::eInterval::p120
                                &&  parseDt.t_iInterval != Bar::eInterval::p180
                            ){
                        parseDt.ossErr() << "Wrong file format: wrong period field value";
                        return false;
                    }
                }
                if (parseDt.DefaultInterval() >= 0){
                    if(parseDt.DefaultInterval() != parseDt.t_iInterval){
                        parseDt.ossErr() << "Interval mismatch";
                        return false;
                    }
                }
                else{
                    parseDt.setDefaultInterval(parseDt.t_iInterval);
                }
                b.initInterval(parseDt.t_iInterval);
                break;
            case finamParseData::fieldType::DATE:
                //20210322,
                if(parseDt.t_sWordBuff.size()>=8){
                    copy(parseDt.t_sWordBuff.begin()    ,parseDt.t_sWordBuff.begin() + 4,parseDt.t_sYear.begin());
                    copy(parseDt.t_sWordBuff.begin() + 4,parseDt.t_sWordBuff.begin() + 6,parseDt.t_sMonth.begin());
                    copy(parseDt.t_sWordBuff.begin() + 6,parseDt.t_sWordBuff.begin() + 8,parseDt.t_sDay.begin());
                    parseDt.t_tp.tm_year = std::stoi(parseDt.t_sYear);
                    parseDt.t_tp.tm_mon = std::stoi(parseDt.t_sMonth);
                    parseDt.t_tp.tm_mday = std::stoi(parseDt.t_sDay);
                }
                else{
                    parseDt.ossErr() << "Wrong file format: wrong date field value";
                    return false;
                }
                break;
            case finamParseData::fieldType::TIME:
                //095936
                if(parseDt.t_sWordBuff.size()>=6){
                    copy(parseDt.t_sWordBuff.begin()    ,parseDt.t_sWordBuff.begin() + 2,parseDt.t_sHour.begin());
                    copy(parseDt.t_sWordBuff.begin() + 2,parseDt.t_sWordBuff.begin() + 4,parseDt.t_sMin.begin());
                    copy(parseDt.t_sWordBuff.begin() + 4,parseDt.t_sWordBuff.begin() + 6,parseDt.t_sSec.begin());
                    parseDt.t_tp.tm_hour = std::stoi(parseDt.t_sHour);
                    parseDt.t_tp.tm_min = std::stoi(parseDt.t_sMin);
                    parseDt.t_tp.tm_sec = std::stoi(parseDt.t_sSec);
                }
                else{
                    parseDt.ossErr() << "Wrong file format: wrong time field value";
                    return false;
                }

                break;
            case finamParseData::fieldType::OPEN:
                parseDt.issTmp() >> parseDt.t_dTmp; b.setOpen (parseDt.t_dTmp);
                break;
            case finamParseData::fieldType::HIGH:
                parseDt.issTmp() >> parseDt.t_dTmp; b.setHigh (parseDt.t_dTmp);
                break;
            case finamParseData::fieldType::LOW:
                parseDt.issTmp() >> parseDt.t_dTmp; b.setLow (parseDt.t_dTmp);
                break;
            case finamParseData::fieldType::CLOSE:
                parseDt.issTmp() >> parseDt.t_dTmp; b.setClose (parseDt.t_dTmp);
                break;
            case finamParseData::fieldType::LAST:
                parseDt.issTmp() >> parseDt.t_dTmp;
                b.setOpen   (parseDt.t_dTmp);
                b.setHigh   (b.Open());
                b.setLow    (b.Open());
                b.setClose  (b.Open());
                break;
            case finamParseData::fieldType::VOL:
                b.setVolume (std::stoi(parseDt.t_sWordBuff));
                break;
            default:
                parseDt.ossErr() << "Wrong file format: column parsing";
                return false;
                break;
            }
            parseDt.t_iCurrN++;
            if (parseDt.t_iCurrN > parseDt.ColMax()){
                parseDt.ossErr() << "Wrong file format: not equal column count";
                return false;
            }
        }
        if (parseDt.DefaultInterval() < 0 ) parseDt.setDefaultInterval ( Bar::eInterval::pTick);

        b.setPeriod(std::mktime(&parseDt.t_tp));
    }
    catch (std::exception &e){
        parseDt.ossErr() << "Wrong file format";
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

                // sel first item

                QItemSelectionModel  *qml = ui->viewTickers->selectionModel();
                auto first_i(proxyTickerModel.index(0,0));
                if(first_i.isValid() && qml){
                    qml->select(first_i,QItemSelectionModel::SelectionFlag::ClearAndSelect | QItemSelectionModel::Rows) ;
                    slotSetSelectedTicker(first_i);
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
    if(first_i.isValid() && qml){
        qml->select(first_i,QItemSelectionModel::SelectionFlag::ClearAndSelect | QItemSelectionModel::Rows) ;
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
        iSelectedTickerId = t.TickerID();

        if(t.TickerSignFinam().size() > 0)
            ui->edSign->setText(QString::fromStdString(t.TickerSign())+" {"+QString::fromStdString(t.TickerSignFinam())+"}");
        else
            ui->edSign->setText(QString::fromStdString(t.TickerSign())+" {"+QString::fromStdString(sSelectedTickerSignFinam)+"}");
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
}

//--------------------------------------------------------------------------------------------------------
//bool  ImportFinamForm::searchTickerBySign(std::string sSign, QModelIndex& indx)
//{
//    return modelTicker->searchTickerByFinamSign(sSign,  indx);
//}
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------