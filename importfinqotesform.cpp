#include "importfinqotesform.h"
#include "ui_importfinqotesform.h"

#include<QFileDialog>
#include<QDebug>
#include<QStringList>
#include<QMessageBox>

#include<iostream>
#include<filesystem>
#include<iostream>
#include<fstream>
#include<ostream>


ImportFinQuotesForm::ImportFinQuotesForm(MarketsListModel *modelM, int DefaultTickerMarket,
                                 TickersListModel *modelT,
                                 QWidget *parent) :
    QWidget(parent),
    iDefaultTickerMarket{DefaultTickerMarket},
    modelMarket{modelM},
    modelTicker{modelT},
    parseDataReady{nullptr,nullptr},

    ui(new Ui::ImportFinamForm)
{
    ui->setupUi(this);
    //
    QColor colorDarkGreen(0, 100, 52,50);
    QColor colorDarkRed(31, 53, 200,40);
    //-------------------------------------------------------------

    QHBoxLayout *lt1 = new QHBoxLayout();
    lt1->setMargin(0);

    ui->wtShowByName->setLayout(lt1);
    swtShowByName = new StyledSwitcher(tr("Show by name "),tr(" Show by ticker"),true,10,this);
    lt1->addWidget(swtShowByName);
    swtShowByName->SetOnColor(QPalette::Window,colorDarkGreen);
    swtShowByName->SetOffColor(QPalette::Window,colorDarkRed);

    /////////////////////////////////

    connect(ui->btnOpen,SIGNAL(clicked()),this,SLOT(slotBtnOpenClicked()));
    connect(ui->btnCreate,SIGNAL(clicked()),this,SLOT(slotBtnCreateClicked()));
    connect(ui->btnImport,SIGNAL(clicked()),this,SLOT(slotBtnImportClicked()));
    connect(ui->btnTest,SIGNAL(clicked()),this,SLOT(slotBtnTestClicked()));

    connect(ui->btnOpen,SIGNAL(pressed()),this,SLOT(slotBtnOpenClicked()));

    connect(swtShowByName,SIGNAL(stateChanged(int)),this,SLOT(slotShowByNamesChecked(int)));

    connect(ui->edDelimiter,SIGNAL(textChanged(const QString &)),this,SLOT(slotEditDelimiterWgtChanged(const QString &)));

    connect(ui->dtStart,SIGNAL(dateTimeChanged(const QDateTime &)),this,SLOT(slotDateTimeStartChanged(const QDateTime &)));
    connect(ui->dtEnd,SIGNAL(dateTimeChanged(const QDateTime &)),this,SLOT(slotDateTimeEndChanged(const QDateTime &)));

    ui->progressBar->setValue(0);

    setMarketModel();
    setTickerModel();

    clearShowAreaOfFields();
    slotSetWidgetsInLoadState(bInLoading);

    QString str = tr("Welcome to the form for importing information on stock quotes.\n\r"
                     "If you want to import data, first download the file with trading info from https://www.finam.ru/profile/moex-akcii/sberbank/export/ or a similar site and save it on your computer.\n"
                     "Then select the file using the [...] button (see the top of this form). After that, in this text window, you will see the results of the file check.\n"
                     "If all is well, then to load the data into the database, click the import button.\n\r"
                     "Keep in mind that you can only import data with a tick period. Data with other periods can only be loaded into the database for verification.\n\r"
                     "You can open several forms for import at once and load several papers at the same time even if import from quik is running\n\r");

    ui->edText->append(str);

    //Добро пожаловать в форму для импорта данных с сайта finam.ru. Если   вы хотите импортировать данные, сначала загрузите файл с котировками с финам или аналогичной площадки и сохраните его у себя на компьютере. Затем выберите файл с помощью кнопки открыть (см. вверху этой формы). После этого в этом текстовом окне вы увидите результаты проверки файла. Если все хорошо, то для загрузки данных в базу, нажмите кнопку импорт. Имейте ввиду, что импортировать можно только данные с периодом тик. Данные с другими периодами можно только загружены в базу для проверки.
    //вы можете открыть сразу несколько форм для импорта и одновременно загружать несколько бумаг даже если  запущен импорт из quik
}
//--------------------------------------------------------------------------------------------------------
ImportFinQuotesForm::~ImportFinQuotesForm()
{
    delete ui;
}
//--------------------------------------------------------------------------------------------------------
void ImportFinQuotesForm::SetDefaultOpenDir(QString &s)
{
    pathDir = s.toStdString();
}
//--------------------------------------------------------------------------------------------------------
void ImportFinQuotesForm::slotBtnOpenClicked()
{
//    QFileDialog dlg (this,"Open file to import",
//                    QString::fromStdString(pathDir.string()),
//                    "*.txt");
//    if (dlg.exec()){
//        QStringList lst (dlg.selectedFiles());
//        if (lst.size() == 1){
//            QString str = lst.first();

    QString str=QFileDialog::getOpenFileName(0,"Open file to import",
                                                 QString::fromStdString(pathDir.string()),
                                                 "*.txt");
    if (str.size()>0){
        {
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
//    dlg.deleteLater();

}

//--------------------------------------------------------------------------------------------------------
void ImportFinQuotesForm::slotBtnCreateClicked(){

    pathFile ="/home/albert/Загрузки/SBER_210322_210330.txt";
    pathDir=pathFile.parent_path();
    QString qsDir=QString::fromStdString(pathDir.string());;
    OpenImportFilePathChanged(qsDir);
    //
    QString strFileToShow = QString::fromStdString(pathFile.filename().string());
    ui->edFileName->setText(strFileToShow);
    //
    slotPreparseImportFile();
};

//--------------------------------------------------------------------------------------------------------
void ImportFinQuotesForm::slotBtnTestClicked()
{
    slotPreparseImportFile();
};
//--------------------------------------------------------------------------------------------------------
void ImportFinQuotesForm::SetDelimiter(char c)
{
    cDelimiter = c;
    std::string s{" "};
    s[0] = c;
    trim(s);
    ui->edDelimiter->setText(QString::fromStdString(s));
}
//--------------------------------------------------------------------------------------------------------
void ImportFinQuotesForm::slotEditDelimiterWgtChanged(const QString &)
{
    std::string sD = ui->edDelimiter->text().toStdString();
    trim(sD);
    if(sD.size()<=0){
        cDelimiter = ' ';
    }
    else{
        cDelimiter = sD[0];
    }
    std::string s{" "};
    s[0] = cDelimiter;
    trim(s);
    ui->edDelimiter->setText(QString::fromStdString(s));
    ///////////////
    emit DelimiterHasChanged(cDelimiter);
}
//--------------------------------------------------------------------------------------------------------
// preliminary check import file. Lookup for Ticker sign, time periods etc.
void ImportFinQuotesForm::slotPreparseImportFile()
{

    ////////////////////////////////////////////////////////////////////////////////////
    // <TICKER> | <PER> | <DATE> | <TIME> | <LAST> | <VOL>
    // <TICKER> | <PER> | <DATE> | <TIME> | <OPEN> | <HIGH> | <LOW> | <CLOSE> | <VOL>

    clearShowAreaOfFields();
    ui->edText->append( "====================================================");
    ui->edText->append( "trying to parse the file:");
    ////////////////////////////////////////////////////////////////////////////////////


    bReadyToImport              = false;
    iTimePeriod                 = 0;
    iSelectedTickerId           = 0;
    iFoundTickerId              = 0;
    //sSelectedTickerSignFinam    = "";
    sFoundTickerSignFinam       = "";
    bFoundTicker                = false;
    bFoundTickerFinam           = false;




    std::string sBuff;
    std::istringstream iss;
    std::stringstream oss;
    std::istringstream issTmp;
    std::ostringstream ossErr;

    dataFinQuotesParse parseDt(&issTmp,&ossErr);

    parseDt.initDefaultFieldsValues(9);
    parseDt.setDelimiter(cDelimiter);

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
                    if      (bWasHeader && sWordBuff == "<TICKER>") {   parseDt.fields()[iN] = dataFinQuotesParse::fieldType::TICKER; }
                    else if (bWasHeader && sWordBuff == "<PER>")    {   parseDt.fields()[iN] = dataFinQuotesParse::fieldType::PER;    }
                    else if (bWasHeader && sWordBuff == "<DATE>")   {   parseDt.fields()[iN] = dataFinQuotesParse::fieldType::DATE;   iFieldMask |= 1;}
                    else if (bWasHeader && sWordBuff == "<TIME>")   {   parseDt.fields()[iN] = dataFinQuotesParse::fieldType::TIME;   iFieldMask |= 2;}
                    else if (bWasHeader && sWordBuff == "<OPEN>")   {   parseDt.fields()[iN] = dataFinQuotesParse::fieldType::OPEN;   iFieldMask |= 4;}
                    else if (bWasHeader && sWordBuff == "<HIGH>")   {   parseDt.fields()[iN] = dataFinQuotesParse::fieldType::HIGH;   iFieldMask |= 8;}
                    else if (bWasHeader && sWordBuff == "<LOW>")    {   parseDt.fields()[iN] = dataFinQuotesParse::fieldType::LOW;    iFieldMask |= 16;}
                    else if (bWasHeader && sWordBuff == "<CLOSE>")  {   parseDt.fields()[iN] = dataFinQuotesParse::fieldType::CLOSE;  iFieldMask |= 32;}
                    else if (bWasHeader && sWordBuff == "<VOL>")    {   parseDt.fields()[iN] = dataFinQuotesParse::fieldType::VOL;    iFieldMask |= 64;}
                    else if (bWasHeader && sWordBuff == "<LAST>")   {   parseDt.fields()[iN] = dataFinQuotesParse::fieldType::LAST;   iFieldMask |= 128;}
                    else {
                        if(iN == 0 || iN == 1){ // first column is sign or period
                            for(const unsigned char ch:sWordBuff){
                                if (!std::isalnum(ch)){
                                    ui->edText->append(QString::fromStdString(sBuff));
                                    ui->edText->append("Wrong file format");

                                    std::stringstream ss;
                                    ss << "you are using [";
                                    ss << cDelimiter;
                                    ss << "] delimiter. Maybe try another one? (See text box bolow)";
                                    ui->edText->append(QString::fromStdString(ss.str()));
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

                                std::stringstream ss;
                                ss << "you are using [";
                                ss << cDelimiter;
                                ss << "] delimiter. Maybe try another one? (See text box bolow)";
                                ui->edText->append(QString::fromStdString(ss.str()));

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
                    parseDt.SetHeaderPresence(true);
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
                if (!Storage::slotParseLine(parseDt, iss, bb)){
                    ui->edText->append(QString::fromStdString(iss.str()));
                    ui->edText->append(QString::fromStdString(ossErr.str()));
                    return;
                }

                showInterval(bb.Interval());
                iTimePeriod = bb.Interval();

                {
                    std::time_t tS (bb.Period());
                    std::tm* tmSt=localtime(&tS);
                    const QDate dtS(tmSt->tm_year+1900,tmSt->tm_mon+1,tmSt->tm_mday);
                    const QTime tmS(tmSt->tm_hour,tmSt->tm_min,tmSt->tm_sec);
                    QDateTime dt (dtS,tmS);
                    qdtMin = dt;

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

                if (!Storage::slotParseLine(parseDt, iss, bb)){
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
                    const QDate dtS(tmSt->tm_year+1900,tmSt->tm_mon+1,tmSt->tm_mday);
                    const QTime tmS(tmSt->tm_hour,tmSt->tm_min,tmSt->tm_sec);
                    QDateTime dt (dtS,tmS);
                    qdtMax = dt;

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
                sSelectedTickerSignFinam    = parseDt.Sign();
                sFoundTickerSignFinam       = parseDt.Sign();

                QModelIndex indxM;
                QModelIndex indxT;
                QModelIndex indxProxyT;

                if(     modelTicker->searchTickerByFinamSign(parseDt.Sign(), indxT)){
                    bFoundTickerFinam = true;
                }
                else if(modelTicker->searchTickerBySign(parseDt.Sign(), indxT)){
                    bFoundTicker = true;
                }
                //
                if((bFoundTicker || bFoundTickerFinam ) && indxT.isValid()){
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

                    iSelectedTickerId   = t.TickerID();
                    iFoundTickerId      = t.TickerID();

                    ui->edSign->setText(QString::fromStdString(t.TickerSign()+" {"+sFoundTickerSignFinam+"}"));
                    ui->viewTickers->setFocus();
                }
                else{
                    ui->edSign->setText(QString::fromStdString("{"+sFoundTickerSignFinam+"}"));
                }

                //////////////////////
                ui->edText->append(QString::fromStdString(oss.str()));


                parseDataReady              = parseDt;
                bReadyToImport              = true;

                if(bb.Interval() == Bar::eInterval::pTick){
                    bInChecking = false;
                    ui->btnImport->setText("Import");
                }
                else{
                    bInChecking = true;
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
void ImportFinQuotesForm::clearShowAreaOfFields()
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
void ImportFinQuotesForm::showInterval(int Interval)
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
void ImportFinQuotesForm::setMarketModel()//MarketsListModel *model, int DefaultTickerMarket
{


    //ui->viewTickets
    ui->cmbMarket->setModel(modelMarket);

    {

        connect(ui->cmbMarket,SIGNAL(activated(const int)),this,SLOT(slotSetSelectedTickersMarket(const  int)));

        bool bWas{false};
        for(int i = 0; i < modelMarket->rowCount(); i++){
            auto idx(modelMarket->index(i,0));
            if(idx.isValid()){
                if(modelMarket->getMarket(idx).MarketID() == iDefaultTickerMarket){
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

void ImportFinQuotesForm::slotSetSelectedTickersMarket(const  int i)
{
    if (modelMarket ==nullptr ||modelTicker ==nullptr ) return;
    //qDebug()<<"i: {"<<i<<"}";
    if( i < modelMarket->rowCount()){
        auto idx(modelMarket->index(i,0));
        if(idx.isValid()){
            if(modelMarket->getMarket(idx).MarketID() != iDefaultTickerMarket){
                iDefaultTickerMarket = modelMarket->getMarket(idx).MarketID();
                NeedSaveDefaultTickerMarket(iDefaultTickerMarket);
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
void ImportFinQuotesForm::setTickerModel()//TickersListModel *model,bool /*ShowByName*/,bool /*SortByName*/
{

    proxyTickerModel.setSourceModel(modelTicker);
    proxyTickerModel.setDefaultMarket(iDefaultTickerMarket);
    proxyTickerModel.sort(2);
    ui->viewTickers->setModel(&proxyTickerModel);

    slotShowByNamesChecked(swtShowByName->isChecked());

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
void ImportFinQuotesForm::slotSetSelectedTicker(const  QModelIndex& indx)
{


    //qDebug()<<"Enter slotSetSelectedMarket";
    iSelectedTickerId = 0;
    sSelectedTickerSignFinam = "";

    if (indx.isValid()){
        //const Ticker& t=modelTicker->getTicker(indx);
        const Ticker& t=proxyTickerModel.getTicker(indx);
        iSelectedTickerId = t.TickerID();
        sSelectedTickerSignFinam = t.TickerSignFinam();

        if(t.TickerSignFinam().size() > 0)
            ui->edSign->setText(QString::fromStdString(t.TickerSign())+" {"+QString::fromStdString(t.TickerSignFinam())+"}");
        else
            ui->edSign->setText(QString::fromStdString(t.TickerSign())+" {"+QString::fromStdString(sFoundTickerSignFinam)+"}");
    }

}
//--------------------------------------------------------------------------------------------------------
void ImportFinQuotesForm::slotShowByNamesChecked(int Checked)
{
    if (modelMarket ==nullptr ||modelTicker ==nullptr ) return;

    if (Checked){
        ui->viewTickers->setModelColumn(0);
    }
    else{
        ui->viewTickers->setModelColumn(2);
    }
    proxyTickerModel.invalidate();
}

//--------------------------------------------------------------------------------------------------------
void ImportFinQuotesForm::slotBtnImportClicked()
{
    slotSetWidgetsInLoadState(bInLoading);

    if(!bInLoading){
        if(bReadyToImport){
            QItemSelectionModel  *qml = ui->viewTickers->selectionModel();
            auto lst (qml->selectedIndexes());
            if(bFoundTickerFinam && iSelectedTickerId != iFoundTickerId){
                QString sQuestion;
                sQuestion = tr("Finam sign <") + QString::fromStdString(sFoundTickerSignFinam) + "> " +
                        tr("has already bound to another ticker. Select correct ticker or go to tickers config and reset settings if neсessary.");
                int n=QMessageBox::warning(0,tr("Warning"),sQuestion,QMessageBox::Ok);
                if (n==QMessageBox::Ok){;}
                return;
            }
            //
            if(!bFoundTicker && !bFoundTickerFinam){
                if(iSelectedTickerId == 0){
                    QString sQuestion;
                    sQuestion = tr("No ticker with <") + QString::fromStdString(sFoundTickerSignFinam) + "> " +
                            tr("sign was foung in base! Create new?");
                    int n=QMessageBox::warning(0,tr("Warning"),sQuestion,QMessageBox::Yes | QMessageBox::No);
                    if (n==QMessageBox::Yes){
                        Ticker t {  sFoundTickerSignFinam,sFoundTickerSignFinam,iDefaultTickerMarket};
                        t.SetTickerSignFinam(sFoundTickerSignFinam);
                        t.SetTickerSignQuik("");
                        t.SetAutoLoad(true);
                        t.SetUpToSys(false);
                        int i = proxyTickerModel.AddRow(t);

                        QItemSelectionModel  *qml =ui->viewTickers->selectionModel();

                        auto indx(proxyTickerModel.index(i,0));
                        if(indx.isValid() && qml){
                            qml->select(indx,QItemSelectionModel::SelectionFlag::ClearAndSelect | QItemSelectionModel::Rows) ;
                            slotSetSelectedTicker(indx);
                            }
                    }
                    else{return;}
                }
                else{
                    if (lst.size() > 0){

                        Ticker t = proxyTickerModel.getTicker(lst[0]);
                        QString sQuestion;

                        sQuestion = tr("To associate data for the finam sign <")+QString::fromStdString(sFoundTickerSignFinam) + "> " +
                                    tr("to the ticker") + " <" + QString::fromStdString(t.TickerSign()) +">?";
                        int n=QMessageBox::warning(0,tr("Warning"),sQuestion,QMessageBox::Yes | QMessageBox::No);
                        if (n==QMessageBox::Yes){
                            t.SetTickerSignFinam(sFoundTickerSignFinam);
                            proxyTickerModel.setData(lst[0],t,Qt::EditRole);
                        }
                        else{return;}
                    }
                    else{
                        int n=QMessageBox::warning(0,tr("Warning"),tr("Select correct file to import!"),QMessageBox::Ok);
                        if (n==QMessageBox::Ok){;}
                        return;
                    }
                }
            }
            else if(!bFoundTickerFinam){
                if(iSelectedTickerId != 0 && lst.size() > 0){

                        Ticker t = proxyTickerModel.getTicker(lst[0]);
                        QString sQuestion;

                        sQuestion = tr("To associate data for the finam sign <")+QString::fromStdString(sFoundTickerSignFinam) + "> " +
                                    tr("to the ticker") + " <" + QString::fromStdString(t.TickerSign()) +">?";
                        int n=QMessageBox::warning(0,tr("Warning"),sQuestion,QMessageBox::Yes | QMessageBox::No);
                        if (n==QMessageBox::Yes){
                            t.SetTickerSignFinam(sFoundTickerSignFinam);
                            proxyTickerModel.setData(lst[0],t,Qt::EditRole);
                        }
                        else{return;}
                }
                else{
                    int n=QMessageBox::warning(0,tr("Warning"),tr("Select correct file to import!"),QMessageBox::Ok);
                    if (n==QMessageBox::Ok){;}
                    return;
                }
            }
            //////////////////////////////////////////////////////////////////////////////////////////////////////

            dataFinLoadTask dataTask;

            dataTask.TickerID       = iSelectedTickerId;
            dataTask.iInterval      = iTimePeriod;
            dataTask.sSign          = sSelectedTickerSignFinam;
            dataTask.pathFileName   = pathFile;
            dataTask.SetParentWnd(this);

            QDateTime qdtSt         = ui->dtStart->dateTime();
            QDateTime qdtEnd        = ui->dtEnd->dateTime();

            std::tm   tmSt;
            tmSt.tm_year    = qdtSt.date().year()       - 1900;
            tmSt.tm_mon     = qdtSt.date().month()      - 1;
            tmSt.tm_mday    = qdtSt.date().day();
            tmSt.tm_hour    = qdtSt.time().hour();
            tmSt.tm_min     = qdtSt.time().minute();
            tmSt.tm_sec     = qdtSt.time().second();
            tmSt.tm_isdst   = 0;
            dataTask.dtBegin = std::mktime(&tmSt);

            std::tm   tmEnd;
            tmEnd.tm_year    = qdtEnd.date().year()     - 1900;
            tmEnd.tm_mon     = qdtEnd.date().month()    - 1;
            tmEnd.tm_mday    = qdtEnd.date().day();
            tmEnd.tm_hour    = qdtEnd.time().hour();
            tmEnd.tm_min     = qdtEnd.time().minute();
            tmEnd.tm_sec     = qdtEnd.time().second();
            tmEnd.tm_isdst   = 0;
            dataTask.dtEnd = std::mktime(&tmEnd);

            dataTask.parseData = parseDataReady;

            emit NeedParseImportFinQuotesFile(dataTask);

            bInLoading = true;
            slotSetWidgetsInLoadState(bInLoading);

        }
        else{
            int n=QMessageBox::warning(0,tr("Warning"),tr("Select correct file to import!"),QMessageBox::Ok);
            if (n==QMessageBox::Ok){;}
        }
    }
    else{
        QString sQuestion = tr("Do you want to stop loading process?\n Warning: will be stopped all loadings from files!");
        int n=QMessageBox::warning(0,tr("Warning"),sQuestion,QMessageBox::Yes | QMessageBox::No);
        if (n==QMessageBox::Yes){
            emit NeedToStopLoadings();
        }
    }
};
//--------------------------------------------------------------------------------------------------------
void ImportFinQuotesForm::SetProgressBarValue(int iVal)
{
    if(iVal >=0 && iVal <= 100){
        ui->progressBar->setValue(iVal);
    }
}
//--------------------------------------------------------------------------------------------------------
void ImportFinQuotesForm::slotLoadingHasBegun(){
    if(!bInChecking)
        ui->edText->append(tr("Import started..."));
    else
        ui->edText->append(tr("Data verification started..."));

}
//--------------------------------------------------------------------------------------------------------
void ImportFinQuotesForm::slotLoadingActivity(){;}
//--------------------------------------------------------------------------------------------------------
void ImportFinQuotesForm::slotLoadingHasFinished(bool bSuccess, QString qsErr){


    if (bSuccess){
        if(!bInChecking)
            ui->edText->append(tr("Import has been complited successfuly.\n"));
        else
            ui->edText->append(tr("Data verification has been complited successfuly.\n"));
    }
    else{
        if(!bInChecking)
            ui->edText->append(tr("Import was complited with errors:"));
        else
            ui->edText->append(tr("Data verification was complited with errors:"));

        ui->edText->append(qsErr);
        ui->edText->append("\n");
    }

    bInLoading = false;
    slotSetWidgetsInLoadState(bInLoading);
}
//--------------------------------------------------------------------------------------------------------
void ImportFinQuotesForm::slotTextInfo(QString qsStr){
    ui->edText->append(qsStr);
}
//--------------------------------------------------------------------------------------------------------
void ImportFinQuotesForm::slotSetWidgetsInLoadState(bool bInLoad)
{
    ui->btnCreate->setEnabled(!bInLoad);
    ui->btnOpen->setEnabled(!bInLoad);
    ui->btnTest->setEnabled(!bInLoad);
    ui->edDelimiter->setEnabled(!bInLoad);
    ui->dtStart->setEnabled(!bInLoad);
    ui->dtEnd->setEnabled(!bInLoad);
    ui->cmbMarket->setEnabled(!bInLoad);
    ui->viewTickers->setEnabled(!bInLoad);
    swtShowByName->setEnabled(!bInLoad);

    if(bInLoad){
        ui->btnImport->setText(tr("Stop"));
    }
    else{
        if(!bInChecking){
            ui->btnImport->setText("Import");
        }
        else{
            ui->btnImport->setText("Check");
        }
    }
}
//--------------------------------------------------------------------------------------------------------
void ImportFinQuotesForm::slotDateTimeStartChanged(const QDateTime &)
{
    if(bReadyToImport && ui->dtStart->dateTime() < qdtMin){
        ui->dtStart->setDateTime(qdtMin);
    }
}
//--------------------------------------------------------------------------------------------------------
void ImportFinQuotesForm::slotDateTimeEndChanged(const QDateTime &)
{
    if(bReadyToImport && ui->dtEnd->dateTime() > qdtMax){
        ui->dtEnd->setDateTime(qdtMax);
    }
}
//--------------------------------------------------------------------------------------------------------
