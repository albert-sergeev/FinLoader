#ifndef IMPORTFINAMFORM_H
#define IMPORTFINAMFORM_H

#include <QWidget>
#include<filesystem>

#include "marketslistmodel.h"
#include "tickerslistmodel.h"
#include "storage.h"
#include "bar.h"
#include "datafinamloadtask.h"




namespace Ui {
class ImportFinamForm;
}

class finamParseData{

    //std::istringstream * const iss;
    //std::ostringstream * const oss;
    std::istringstream * const issT;
    std::ostringstream * const ossE;
    int iColMax;
    int iDefaultInterval;
    std::string sSign;

    std::vector<int>  vFieldsType;

public:

    enum fieldType:int {TICKER,PER,DATE,TIME,OPEN,HIGH,LOW,CLOSE,VOL,LAST};

    finamParseData(std::istringstream * issTmp,std::ostringstream * ossErr):
        issT{issTmp},ossE{ossErr},iColMax{9},iDefaultInterval{-1},sSign{""}
    {
        vFieldsType.resize(9);
        t_tp.tm_isdst = 0;
    };
public:

    std::string t_sWordBuff;
    std::string t_sSign;
    int         t_iInterval;
    double      t_dTmp;
    std::string t_sYear{"1990"};
    std::string t_sMonth{"01"};
    std::string t_sDay{"01"};
    std::string t_sHour{"00"};
    std::string t_sMin{"00"};
    std::string t_sSec{"00"};
    std::tm     t_tp;
    int         t_iCurrN{0};

public:

    std::ostringstream & ossErr()       {return *ossE;};
    std::istringstream & issTmp()       {return *issT;};
    int ColMax()           const        {return  iColMax;};
    int DefaultInterval()  const        {return  iDefaultInterval;};
    std::string Sign()     const        {return  sSign;};
    std::vector<int> & fields()    { return vFieldsType;};


    void setDefaultInterval(const int DefaultInterval)  {iDefaultInterval = DefaultInterval;};
    void setDefaultSign(const std::string Sign)         {sSign = Sign;};

    //----------------------
    bool initDefaultFieldsValues(int iCol){
        vFieldsType[0] = fieldType::TICKER;
        vFieldsType[1] = fieldType::PER;
        vFieldsType[2] = fieldType::DATE;
        vFieldsType[3] = fieldType::TIME;
        vFieldsType[4] = fieldType::OPEN;
        vFieldsType[5] = fieldType::HIGH;
        vFieldsType[6] = fieldType::LOW;
        vFieldsType[7] = fieldType::CLOSE;
        vFieldsType[8] = fieldType::VOL;

        if(iCol == 9 ){
            iColMax = iCol;
        }
        else if(iCol == 6 ){
            iColMax = iCol;
            vFieldsType[4] = fieldType::LAST;
            vFieldsType[5] = fieldType::VOL;
            iDefaultInterval = Bar::eInterval::pTick;
        }
        else{
            return false;
        }
        return  true;
    }
    //----------------------



};

class ImportFinamForm : public QWidget
{
    Q_OBJECT

private:
    //std::filesystem::directory_entry drCurr;
    std::filesystem::path pathFile;
    std::filesystem::path pathDir;
    char cDelimiter{','};

    int iDefaultTickerMarket;

    MarketsListModel * const modelMarket;
    TickersListModel * const modelTicker;
    TickerProxyListModel proxyTickerModel;

    bool bReadyToImport{false};
    int iTimePeriod{0};
    int iSelectedTickerId{0};
    int iFoundTickerId{0};
    std::string sFoundTickerSignFinam{""};
    std::string sSelectedTickerSignFinam{""};
    bool bFoundTicker{false};
    bool bFoundTickerFinam{false};


public:
    explicit ImportFinamForm(MarketsListModel *modelM, int DefaultTickerMarket,
                             TickersListModel *modelT,/*bool ShowByName,bool SortByName,*/
                             QWidget *parent = nullptr);
    ~ImportFinamForm();

public:
    void SetDefaultOpenDir(QString &s);
    void SetDelimiter(char c);


signals:
    void OpenImportFilePathChanged(QString &);
    void DelimiterHasChanged(char c);
    void NeedParseImportFinamFile(dataFinamLoadTask &);
    void NeedSaveDefaultTickerMarket(int);

private slots:

    void slotBtnOpenClicked();
    void slotBtnCreateClicked();
    void slotBtnTestClicked();
    void slotBtnImportClicked();
    void slotEditDelimiterWgtChanged(const QString &);
    void slotShowByNamesChecked(int Checked);

    void slotSetSelectedTickersMarket(const  int i);
    void slotSetSelectedTicker(const  QModelIndex& indx);
    void slotSetSelectedTicker(const  QModelIndex& indx,const  QModelIndex& ) {slotSetSelectedTicker(indx);};

    void slotPreparseImportFile();
    //bool slotParseLine(std::vector<int> & fieldsType, std::istringstream & issLine, std::istringstream & issTmp, std::ostringstream & ossErr, Bar &b, int ColMax, int DefaultInterval = (-1));
    bool slotParseLine(finamParseData & parseDt, std::istringstream & issLine, Bar &b);



    //bool  searchTickerBySign(std::string sSign, QModelIndex& indx);

    void showInterval(int Interval);
    void clearShowAreaOfFields();

private:

//    void setMarketModel(MarketsListModel *model, int DefaultTickerMarket);
//    void setTickerModel(TickersListModel *model,bool ShowByName,bool SortByName);
    void setMarketModel();
    void setTickerModel();

    Ui::ImportFinamForm *ui;
};





#endif // IMPORTFINAMFORM_H
