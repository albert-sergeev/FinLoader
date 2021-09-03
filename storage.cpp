#include "storage.h"

#include<filesystem>
#include<iostream>
#include<regex>
#include<iomanip>
#include<fstream>
#include<ostream>
#include<chrono>

#include "threadpool.h"
#include "threadfreelocaltime.h"

//using namespace std::filesystem;

////////////////////////////////////////////////////////////////////
// file switch stages:
//      1   2   3   4   5   6   (7)
//  1.  W   L2  L2  U   W   W   (W)
//  2.  U   W   W   W   L2  L2  (U)
//  3.  U   U   S   L1  L1  L1  (U)
//  4.  L1  L1  L1  U   U   S   (L1)
//
// Description:
//  W   - write (append)
//  U   - undefined (unused)
//  L1  - read only (read order: first)
//  L2  - read only (read order: second)
//  S   - shrinked (compilation of L1 + L2)


//--------------------------------------------------------------------------------------------------------
Storage::Storage():bInitialized{false}
    ,vStorageW  {1,2,2,2,1,1}
    ,vStorageL1 {4,4,4,3,3,3}
    ,vStorageL2 {0,1,1,0,2,2}
    ,vStorageS  {0,0,3,0,0,4}

    ,vStorage1  {true ,true ,true ,false,true ,true }
    ,vStorage2  {false,true ,true ,true ,true ,true }
    ,vStorage3  {false,false,true ,true ,true ,true }
    ,vStorage4  {true ,true ,true ,false,false,true }
{


}
//--------------------------------------------------------------------------------------------------------
void Storage::Initialize(std::string sPath)
{
    // comment 2
    if (bInitialized) return;

    /// for speed optimisation
    std::ios::sync_with_stdio(false);


    std::unique_lock lk1(mutexMarketConfigFile);
    std::unique_lock lk2(mutexTickerConfigFile);

    //std::filesystem::path pathCurr;
    //std::filesystem::path pathDataDir;
    //std::filesystem::path pathStorageDir;

    sStoragePath = sPath;
    pathCurr = std::filesystem::absolute(sStoragePath);
    std::string sTmp = pathCurr.string();

    if(!std::filesystem::exists(pathCurr)){
        if(!std::filesystem::create_directory(pathCurr)){
            throw std::runtime_error("Unable to create directory " + sTmp);
        }
    }
    pathCurr = std::filesystem::absolute(sStoragePath);
    if(!std::filesystem::is_directory(pathCurr)){
        throw std::runtime_error(sStoragePath + " - is not directory");
    }
    ///========
    if(!std::filesystem::exists(pathCurr/"data")){
        if(!std::filesystem::create_directory(pathCurr/"data")){
            throw std::runtime_error("Unable to create ./data/ directory");
        }
    }
    pathDataDir = std::filesystem::absolute(pathCurr/"data");
    if(!std::filesystem::is_directory(pathDataDir)){
        throw std::runtime_error(" ./data - is not directory");
    }
    ///========
    if(!std::filesystem::exists(pathDataDir/"storage")){
        if(!std::filesystem::create_directory(pathDataDir/"storage")){
            throw std::runtime_error("Unable to create ./data/dtorage/ directory");
        }
    }
    pathStorageDir = std::filesystem::absolute(pathDataDir/"storage");
    if(!std::filesystem::is_directory(pathStorageDir)){
        throw std::runtime_error(" ./data/storage - is not directory");
    }
    ///========
    pathMarkersFile = std::filesystem::absolute(pathDataDir/"markets.dat");
    //
    if(!std::filesystem::exists(pathMarkersFile)){ // if no file - create new with defaults
        std::ofstream fileMarket(pathMarkersFile);
        std::vector<Market> m{{trim("MOEX"),trim("MICEX")}};
        SaveMarketConfigLocal(m);
        if(!std::filesystem::exists(pathMarkersFile)){
            throw std::runtime_error("Error create file: ./data/markets.dat");
        }
    }
    if(   !std::filesystem::is_regular_file(pathMarkersFile)){
        throw std::runtime_error("./data/markets.dat - has wrong type");
    }
    ///========
    pathTickersSwitcherFile = std::filesystem::absolute(pathDataDir/"tickers.swr");
    //
    if(!std::filesystem::exists(pathTickersSwitcherFile)){ // if no file - create new with defaults
        SwitchTickersConfigFile();
        if(!std::filesystem::exists(pathTickersSwitcherFile)){
            throw std::runtime_error("Error create file: ./data/tickers.swr");
        }
    }
    if(!std::filesystem::is_regular_file(pathTickersSwitcherFile)){
        throw std::runtime_error("./data/tickers.swr - has wrong type");
    }
    ///========
    pathTickersFile = ReadTickersConfigFileName(true);
    //
    if(!std::filesystem::exists(pathTickersFile)){ // if no file - create new with defaults
        FormatTickerConfigV_1();
        if(!std::filesystem::exists(pathTickersFile)){
            throw std::runtime_error("Error create file: ./data/tickers.dat");
        }
    }
    if(   !std::filesystem::is_regular_file(pathTickersFile)){
        throw std::runtime_error("./data/tickerss.dat - has wrong type");
    }
    ///========
    bInitialized = true;
}
//--------------------------------------------------------------------------------------------------------
void Storage::LoadMarketConfig(std::vector<Market> & vMarketsLst)
{
    if (!bInitialized) Initialize(sStoragePath);
    //
    std::unique_lock lk(mutexMarketConfigFile);
    //
    std::string sBuff;
    std::istringstream iss{sBuff};
    std::ifstream fileMarket(pathMarkersFile.string());
    if(std::getline(fileMarket,sBuff)){
        if(sBuff == "v1"){
            ParsMarketConfigV_1(vMarketsLst,fileMarket);
        }
        else{
            throw std::runtime_error("wrong file format ./data/markets.dat!");
        }
    }
    else{
        std::stringstream ss;
        ss <<"error reading file: "<<pathMarkersFile;
        throw std::runtime_error(ss.str());
    }
}
//--------------------------------------------------------------------------------------------------------
void Storage::ParsMarketConfigV_1(std::vector<Market> & vMarketsLst, std::ifstream &file)
{
    vMarketsLst.clear();
    //
    std::string sBuff;
    std::istringstream iss;
    while (std::getline(file,sBuff)) {
        // link stringstream
        iss.clear();
        iss.str(sBuff);
        //
        std::vector<std::string> vS{std::istream_iterator<std::string>{iss},{}};

        if(vS.size()<7){
            std::stringstream ss;
            ss <<"error parsing file. wrong format: "<<pathMarkersFile;
            throw std::runtime_error(ss.str());
        }
        //
        Market m{vS[1],vS[2],std::stoi(vS[0])};

        m.SetAutoLoad   (std::stoi(vS[3]));
        m.SetUpToSys    (std::stoi(vS[4]));
        m.SetStartTime  (std::stoi(vS[5]))  ;
        m.SetEndTime    (std::stoi(vS[6]))  ;

        vMarketsLst.push_back(m);
    }
}
//--------------------------------------------------------------------------------------------------------
// plug for different versions
void Storage::SaveMarketConfig(std::vector<Market> & vMarketsLst)
{
    std::unique_lock lk(mutexMarketConfigFile);
    //
    SaveMarketConfigLocal(vMarketsLst);
}
void Storage::SaveMarketConfig(std::vector<Market> && vMarketsLst)
{
    SaveMarketConfig(vMarketsLst);
}
//--------------------------------------------------------------------------------------------------------
// to avoid mutex
void Storage::SaveMarketConfigLocal(std::vector<Market> & vMarketsLst)
{
    SaveMarketConfigV_1(vMarketsLst);
}
//--------------------------------------------------------------------------------------------------------
void Storage::SaveMarketConfigV_1(std::vector<Market> & vMarketsLst)
{
    std::ofstream fileMarket(pathMarkersFile);
    fileMarket <<"v1\n";
    //
    for(const auto & m:vMarketsLst){
        fileMarket<<m.MarketID()<<" ";
        fileMarket<<m.MarketName()<<" ";
        fileMarket<<m.MarketSign()<<" ";
        fileMarket<<m.AutoLoad()<<" ";
        fileMarket<<m.UpToSys()<<" ";
        fileMarket<<m.StartTime()<<" ";
        fileMarket<<m.EndTime()<<" ";
        fileMarket<<"\n";
    }
}
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
void Storage::SwitchTickersConfigFile()
{
    int iSW{0};
    int iSWNew{0};
    if(!std::filesystem::exists(pathTickersSwitcherFile)){
        std::ofstream fileSW(pathTickersSwitcherFile);
        if (fileSW){
            fileSW <<"1\n";
            iSWNew = 1;
        }
        else{
            throw std::runtime_error("Error create file: ./data/tickers.swr");
        }
    }
    else{
        std::string sBuff;
        std::ifstream fileSW(pathTickersSwitcherFile.string());
        if(std::getline(fileSW,sBuff) && sBuff.size() >0){
            if(sBuff == "1"){
                iSW = 1;
                iSWNew = 2;
            }
            else{
                iSW = 2;
                iSWNew = 1;
            }
        }
        else{
            throw std::runtime_error("Error reading file: ./data/tickers.swr");
        }
    }
    //-----------------------------------------------------------
    std::filesystem::path oldfile   = std::filesystem::absolute(pathDataDir/"tickers.dat");
    std::filesystem::path buckup1   = std::filesystem::absolute(pathDataDir/"tickers.backup");
    std::filesystem::path buckup2   = std::filesystem::absolute(pathDataDir/"tickers.backup2");
    std::filesystem::path datafile1 = std::filesystem::absolute(pathDataDir/"tickers.dt1");
    std::filesystem::path datafile2 = std::filesystem::absolute(pathDataDir/"tickers.dt2");

    if (std::filesystem::exists(buckup2)){
        std::filesystem::remove(buckup2);
    }
    if (std::filesystem::exists(buckup1)){
        std::filesystem::rename(buckup1,buckup2);
    }
    if (iSW == 0 && std::filesystem::exists(oldfile)){
        if (std::filesystem::exists(datafile1)){
            std::filesystem::rename(datafile1,buckup1);
        }
        std::filesystem::rename(oldfile,datafile1);

        if (std::filesystem::exists(datafile2)){
            std::filesystem::remove(datafile2);
        }
    }
    else{
        if (iSW == 1){
            if (std::filesystem::exists(datafile1)){
                std::filesystem::rename(datafile1,buckup1);
            }
        }
        else{
            if (std::filesystem::exists(datafile2)){
                std::filesystem::rename(datafile2,buckup1);
            }
        }
        std::ofstream fileSW(pathTickersSwitcherFile);
        if (fileSW){
            fileSW << iSWNew<<"\n";
        }
        else{
            throw std::runtime_error("Error create file: ./data/tickers.swr");
        }
    }
}
//--------------------------------------------------------------------------------------------------------
std::filesystem::path Storage::ReadTickersConfigFileName(bool bOld)
{
    std::filesystem::path datafile1 = std::filesystem::absolute(pathDataDir/"tickers.dt1");
    std::filesystem::path datafile2 = std::filesystem::absolute(pathDataDir/"tickers.dt2");

    std::string sBuff;
    std::ifstream fileSW(pathTickersSwitcherFile.string());
    if(std::getline(fileSW,sBuff) && sBuff.size() >0){
        if(sBuff == "1"){
            if (bOld) return datafile1;
            else      return datafile2;
        }
        else{
            if (bOld) return datafile2;
            else      return datafile1;
        }
    }
    else{
        throw std::runtime_error("Error reading file: ./data/tickers.swr");
    }
}
//--------------------------------------------------------------------------------------------------------
void Storage::SaveTickerConfig(const Ticker & tT, op_type tp)
{
    std::unique_lock lk(mutexTickerConfigFile);
    SaveTickerConfigLocal(tT,tp);
};
//--------------------------------------------------------------------------------------------------------
void Storage::SaveTickerConfig(Ticker && tT, op_type tp)
{
    SaveTickerConfig(tT,tp);
};
//--------------------------------------------------------------------------------------------------------
void Storage::FormatTickerConfigV_1()
{
    std::ofstream fileTicker(pathTickersFile);
    fileTicker<<"v1\n";
}
//--------------------------------------------------------------------------------------------------------
void Storage::SaveTickerConfigLocal(const Ticker & tT, op_type tp)
{
    SaveTickerConfigV_2(pathTickersFile,tT,tp);
}
//--------------------------------------------------------------------------------------------------------
void Storage::SaveTickerConfigV_1(std::filesystem::path  pathFile,const Ticker & tT, op_type tp, int iForceMark)
{

        std::ofstream fileTicker(pathFile,std::ios_base::app);

        if (iForceMark > 0){
            fileTicker<<(iForceMark)<<",";
        }
        else{
            fileTicker<<(iTickerMark++)<<",";
        }


        fileTicker<<tp<<",";

        fileTicker<<tT.MarketID()<<",";
        fileTicker<<tT.TickerID()<<",";

        fileTicker<<tT.TickerName()<<",";
        fileTicker<<tT.TickerSign()<<",";
        fileTicker<<tT.TickerSignFinam()<<",";
        fileTicker<<tT.TickerSignQuik()<<",";
        fileTicker<<tT.AutoLoad()<<",";
        fileTicker<<tT.UpToSys()<<",";
        fileTicker<<tT.Bulbululator()<<",";
        fileTicker<<tT.Buble()<<",";

        fileTicker<<"\n";

};

//--------------------------------------------------------------------------------------------------------
void Storage::LoadTickerConfig(std::vector<Ticker> & vTickersLst)
{

    if (!bInitialized) Initialize(sStoragePath);
    //
    std::unique_lock lk(mutexTickerConfigFile);
    //
    std::string sBuff;
    std::istringstream iss{sBuff};
    std::ifstream fileTicker(pathTickersFile.string());


    if(std::getline(fileTicker,sBuff)){
        int iCollisions{0};
        if(sBuff == "v1"){
            iCollisions = ParsTickerConfigV_1(vTickersLst,fileTicker);
            iCollisions = 1000;// to force format switch
        }
        else if(sBuff == "v2"){
            iCollisions = ParsTickerConfigV_2(vTickersLst,fileTicker);
        }
        else{
            std::stringstream ss;
            ss <<"wrong file format: ./data/"<<pathTickersFile.filename();
            throw std::runtime_error(ss.str());
        }
        if (iCollisions > 10){
            fileTicker.close();
            CompressTickerConfigFile(vTickersLst);
            pathTickersFile = ReadTickersConfigFileName(true);
        }
    }
    else{
        std::stringstream ss;
        ss <<"error reading file: "<<pathTickersFile;
        throw std::runtime_error(ss.str());
    }

};
//--------------------------------------------------------------------------------------------------------
void Storage::CompressTickerConfigFile(std::vector<Ticker> & vTickersLst)
{
    std::filesystem::path  pathNewTickersFile = ReadTickersConfigFileName(false);

    std::ofstream fileTicker(pathNewTickersFile);
    if (!fileTicker){
        std::stringstream ss;
        ss <<"error during creation of file: ./data/"<<pathNewTickersFile.filename();
        throw std::runtime_error(ss.str());
    }
    fileTicker<<"v2\n";
    fileTicker.close();

    int iForceMark{1};
    for (const auto & t:vTickersLst){
        SaveTickerConfigV_2(pathNewTickersFile,t,op_type::update,iForceMark);
        iForceMark++;
    }
    SwitchTickersConfigFile();
    pathTickersFile = ReadTickersConfigFileName(true);
    iTickerMark = iForceMark;
}
//--------------------------------------------------------------------------------------------------------
int Storage::ParsTickerConfigV_1(std::vector<Ticker> & vTickersLst, std::ifstream & file)
{
    int iMark{0};
    int iCollisionCount{0};
    op_type tp;


    int iMarketID{0};
    int iTickerID{0};


    std::map<size_t,size_t> mM;

    //
    vTickersLst.clear();
    //
    std::string sBuff;
    std::istringstream iss;
    while (std::getline(file,sBuff)) {
        // link stringstream
        iss.clear();
        iss.str(sBuff);
        //
        std::vector<std::string> vS{std::istream_iterator<StringDelimiter<','>>{iss},{}};

        if(vS.size()<12){
            std::stringstream ss;
            ss <<"error parsing file. wrong format: "<<pathTickersFile;
            throw std::runtime_error(ss.str());
        }
        //copy(vS.begin(),vS.end(),std::ostream_iterator<std::string>(std::cout,","));std::cout<<"\n";

        //
        iMark                   =std::stoi(vS[0]);
        if (iTickerMark <= iMark) iTickerMark = iMark + 1;

        tp                      = std::stoi(vS[1]) == 2 ? op_type::remove : op_type::update;

        iMarketID               =std::stoi(vS[2]);
        iTickerID               =std::stoi(vS[3]);

        Ticker t{iTickerID,vS[4],vS[5],iMarketID};

        t.SetTickerSignFinam    (vS[6]);
        t.SetTickerSignQuik     (vS[7]);
        t.SetAutoLoad           (std::stoi(vS[8]));
        t.SetUpToSys            (std::stoi(vS[9]));
        t.SetBulbululator       (std::stoi(vS[10]));
        t.SetBuble            (std::stoi(vS[11]));


        auto ItM (mM.find(t.TickerID()));

        if(ItM != mM.end()){
            iCollisionCount++;
            if (tp == op_type::update){
                vTickersLst[ItM->second] = t;
            }
            else{// do remove
                mM[vTickersLst[vTickersLst.size()-1].TickerID()] = ItM->second;
                vTickersLst[ItM->second] = vTickersLst[vTickersLst.size()-1];
                vTickersLst.erase(next(begin(vTickersLst),vTickersLst.size()-1));
                mM.erase(ItM);
            }
        }
        else{
            if (tp == op_type::update){
                mM[t.TickerID()] = vTickersLst.size();
                vTickersLst.push_back(t);
            }
        }
    }
    return iCollisionCount;
};
//--------------------------------------------------------------------------------------------------------
void Storage::FormatTickerConfigV_2()
{
    std::ofstream fileTicker(pathTickersFile);
    fileTicker<<"v2\n";
}
//--------------------------------------------------------------------------------------------------------
void Storage::SaveTickerConfigV_2(std::filesystem::path  pathFile,const Ticker & tT, op_type tp , int iForceMark )
{
    std::ofstream fileTicker(pathFile,std::ios_base::app);

    //--------------------------------------------
    // v1
    if (iForceMark > 0){
        fileTicker<<(iForceMark)<<",";
    }
    else{
        fileTicker<<(iTickerMark++)<<",";
    }

    fileTicker<<tp<<",";

    fileTicker<<tT.MarketID()<<",";
    fileTicker<<tT.TickerID()<<",";

    fileTicker<<trim(tT.TickerName())<<",";
    //fileTicker<<tT.TickerName()<<",";

    fileTicker<<trim(tT.TickerSign())<<",";
    fileTicker<<trim(tT.TickerSignFinam())<<",";
    fileTicker<<trim(tT.TickerSignQuik())<<",";
    fileTicker<<tT.AutoLoad()<<",";
    fileTicker<<tT.UpToSys()<<",";
    fileTicker<<tT.Bulbululator()<<",";
    fileTicker<<tT.Buble()<<",";
    //--------------------------------------------
    // v2

    fileTicker<<tT.StoredSelectedInterval()<<",";
    fileTicker<<tT.OHLC()<<",";
    fileTicker<<tT.StoredTimePosition()<<",";
    fileTicker<<tT.ViewBeginDate()<<",";
    fileTicker<<tT.ViewEndDate()<<",";

    fileTicker<<tT.HScale()<<"|";
    fileTicker<<tT.VVolumeScale()<<"|";
    fileTicker<< tT.VScalse(Bar::eInterval::pTick)<<"|";
    fileTicker<< tT.VScalse(Bar::eInterval::p1)<<"|";
    fileTicker<< tT.VScalse(Bar::eInterval::p5)<<"|";
    fileTicker<< tT.VScalse(Bar::eInterval::p10)<<"|";
    fileTicker<< tT.VScalse(Bar::eInterval::p15)<<"|";
    fileTicker<< tT.VScalse(Bar::eInterval::p30)<<"|";
    fileTicker<< tT.VScalse(Bar::eInterval::p60)<<"|";
    fileTicker<< tT.VScalse(Bar::eInterval::p120)<<"|";
    fileTicker<< tT.VScalse(Bar::eInterval::p180)<<"|";
    fileTicker<< tT.VScalse(Bar::eInterval::pDay)<<"|";
    fileTicker<< tT.VScalse(Bar::eInterval::pWeek)<<"|";
    fileTicker<< tT.VScalse(Bar::eInterval::pMonth)<<"";


    fileTicker<<"\n";
}
//--------------------------------------------------------------------------------------------------------
int Storage::ParsTickerConfigV_2(std::vector<Ticker> & vTickersLst, std::ifstream & file)
{
    int iMark{0};
    int iCollisionCount{0};
    op_type tp;


    int iMarketID{0};
    int iTickerID{0};


    std::map<size_t,size_t> mM;

    //
    vTickersLst.clear();
    //
    std::string sBuff;
    std::istringstream iss;
    while (std::getline(file,sBuff)) {
        // link stringstream
        iss.clear();
        iss.str(sBuff);
        //
        std::vector<std::string> vS{std::istream_iterator<StringDelimiter<','>>{iss},{}};

        if(vS.size()<18){
            std::stringstream ss;
            ss <<"error parsing file. wrong format: "<<pathTickersFile;
            throw std::runtime_error(ss.str());
        }
        //copy(vS.begin(),vS.end(),std::ostream_iterator<std::string>(std::cout,","));std::cout<<"\n";

        //
        iMark                   =std::stoi(vS[0]);
        if (iTickerMark <= iMark) iTickerMark = iMark + 1;

        tp                      = std::stoi(vS[1]) == 2 ? op_type::remove : op_type::update;

        iMarketID               =std::stoi(vS[2]);
        iTickerID               =std::stoi(vS[3]);

        Ticker t{iTickerID,vS[4],vS[5],iMarketID};

        t.SetTickerSignFinam    (vS[6]);
        t.SetTickerSignQuik     (vS[7]);
        t.SetAutoLoad           (std::stoi(vS[8]));
        t.SetUpToSys            (std::stoi(vS[9]));
        t.SetBulbululator       (std::stoi(vS[10]));
        t.SetBuble              (std::stoi(vS[11]));

        //--------------------------------------------
        // v2
        t.SetStoredSelectedInterval(Bar::castInterval(std::stoi(vS[12])));
        t.SetOHLC(std::stoi(vS[13]));
        t.SetStoredTimePosition(std::stoi(vS[14]));
        t.SetViewBegin(std::stol(vS[15]));
        t.SetViewEnd(std::stol(vS[16]));



        std::stringstream ssTail;
        auto It (std::next(vS.begin(),17));
        while (It != vS.end()) {
            ssTail << (*It)<<",";
            It++;
        }

        iss.clear();
        iss.str(ssTail.str());
        std::vector<std::string> vS2{std::istream_iterator<StringDelimiter<'|'>>{iss},{}};

        if(vS2.size()<14){
            std::stringstream ss;
            ss <<"error parsing file. wrong format: "<<pathTickersFile;
            throw std::runtime_error(ss.str());
        }

        std::stringstream ss;
        double d;
        std::map<int,double> m;

        ss.clear(); ss.str(vS2[0]);  ss >> d; t.SetHScale(d);
        ss.clear(); ss.str(vS2[1]);  ss >> d; t.SetVVolumeScale(d);

        ss.clear(); ss.str(vS2[2]);  ss >> d; m[Bar::eInterval::pTick]        = d;
        ss.clear(); ss.str(vS2[3]);  ss >> d; m[Bar::eInterval::p1]           = d;
        ss.clear(); ss.str(vS2[4]);  ss >> d; m[Bar::eInterval::p5]           = d;
        ss.clear(); ss.str(vS2[5]);  ss >> d; m[Bar::eInterval::p10]          = d;
        ss.clear(); ss.str(vS2[6]);  ss >> d; m[Bar::eInterval::p15]          = d;
        ss.clear(); ss.str(vS2[7]);  ss >> d; m[Bar::eInterval::p30]          = d;
        ss.clear(); ss.str(vS2[8]);  ss >> d; m[Bar::eInterval::p60]          = d;
        ss.clear(); ss.str(vS2[9]);  ss >> d; m[Bar::eInterval::p120]         = d;
        ss.clear(); ss.str(vS2[10]); ss >> d; m[Bar::eInterval::p180]         = d;
        ss.clear(); ss.str(vS2[11]); ss >> d; m[Bar::eInterval::pDay]         = d;
        ss.clear(); ss.str(vS2[12]); ss >> d; m[Bar::eInterval::pWeek]        = d;
        ss.clear(); ss.str(vS2[13]); ss >> d; m[Bar::eInterval::pMonth]       = d;


        t.SetVScalse(m);

        auto ItM (mM.find(t.TickerID()));

        if(ItM != mM.end()){
            iCollisionCount++;
            if (tp == op_type::update){
                vTickersLst[ItM->second] = t;
            }
            else{// do remove
                mM[vTickersLst[vTickersLst.size()-1].TickerID()] = ItM->second;
                vTickersLst[ItM->second] = vTickersLst[vTickersLst.size()-1];
                vTickersLst.erase(next(begin(vTickersLst),vTickersLst.size()-1));
                mM.erase(ItM);
            }
        }
        else{
            if (tp == op_type::update){
                mM[t.TickerID()] = vTickersLst.size();
                vTickersLst.push_back(t);
            }
        }
    }
    return iCollisionCount;
}
//--------------------------------------------------------------------------------------------------------

std::time_t Storage::dateCastToMonth(std::time_t t)
{
    std::tm* tmT = threadfree_gmtime(&t);
    std::tm tmNew;

    tmNew.tm_year   = tmT->tm_year;
    tmNew.tm_mon    = tmT->tm_mon;
    tmNew.tm_mday   = 1;
    tmNew.tm_hour   = 0;
    tmNew.tm_min    = 0;
    tmNew.tm_sec    = 0;
    tmNew.tm_isdst   = tmT->tm_isdst;

    return mktime_gm(&tmNew);
}
//--------------------------------------------------------------------------------------------------------
////
/// \brief Work only with dateCastToMonth() type date. It cuts everything else.
/// \param t
/// \return
///
std::time_t Storage::dateAddMonth(std::time_t t)
{

    std::tm* tmT = threadfree_gmtime(&t);
    std::tm tmNew;

    tmNew.tm_year   = tmT->tm_year;
    tmNew.tm_mon    = tmT->tm_mon;
    tmNew.tm_mday   = 1;
    tmNew.tm_hour   = 0;
    tmNew.tm_min    = 0;
    tmNew.tm_sec    = 0;
    tmNew.tm_isdst   = tmT->tm_isdst;

    tmNew.tm_mon++;
    if (tmNew.tm_mon > 11){
        tmNew.tm_mon = 0;
        tmNew.tm_year++;
    }
    return mktime_gm(&tmNew);
}
//--------------------------------------------------------------------------------------------------------
std::time_t Storage::dateAddMonth(std::time_t t, int iMonth)
{
    std::tm* tmT = threadfree_gmtime(&t);
    std::tm tmNew;

    tmNew.tm_year   = tmT->tm_year;
    tmNew.tm_mon    = tmT->tm_mon;
    tmNew.tm_mday   = 1;
    tmNew.tm_hour   = 0;
    tmNew.tm_min    = 0;
    tmNew.tm_sec    = 0;
    tmNew.tm_isdst   = tmT->tm_isdst;
    ////////////////////////////////////
    tmNew.tm_mon   += iMonth;
    while(tmNew.tm_mon >11){
        tmNew.tm_year++;
        tmNew.tm_mon -= 12;
    }
    while(tmNew.tm_mon <0){
        tmNew.tm_year--;
        tmNew.tm_mon += 12;
    }
    ////////////////////////////////////
    return mktime_gm(&tmNew);
}
//--------------------------------------------------------------------------------------------------------
size_t Storage::mapSize(std::map<std::time_t,std::vector<BarTick>>& m) {
    size_t iRet{0};
    for (const auto &v:m){
        iRet +=v.second.size();
    }
    return iRet;
};
//--------------------------------------------------------------------------------------------------------
bool Storage::InitializeTicker(int iTickerID,std::stringstream & ssOut, bool bCheckOnly)
{
    std::shared_lock lk(mutexQuotesStoreInit);
    for (auto const &t:vInitializedTickers){
        if (t == iTickerID){
            return true;
        }
    }
    if (bCheckOnly) return false;
    //
    lk.unlock();
    //
    std::unique_lock<std::shared_mutex> ulk(mutexQuotesStoreInit);
    for (auto const &t:vInitializedTickers){
        if (t == iTickerID){
            return true;
        }
    }
    return  InitializeTickerEntry(iTickerID,ssOut);
}
//--------------------------------------------------------------------------------------------------------
bool Storage::InitializeTickerEntry(int iTickerID,std::stringstream& ssOut){//private
    ///========
    if(!std::filesystem::exists(pathStorageDir) ||
            !std::filesystem::is_directory(pathStorageDir)
            ){
        throw std::runtime_error("corrupted ./data  directory");
    }
    std::stringstream ssD;
    ssD <<pathStorageDir.string();
    ssD <<"/"<<iTickerID;
    std::string sTickerDir(ssD.str());
    //--
    if(!std::filesystem::exists(sTickerDir)){
        if(!std::filesystem::create_directory(sTickerDir)){
            ssOut<< "Unable to create ./data/[TickerID] directory";
            return false;
        }
    }
    std::filesystem::path pathTickerDir = std::filesystem::absolute(sTickerDir);
    if(!std::filesystem::is_directory(pathTickerDir)){
        ssOut <<" ./data/[TickerID] - is not directory";
        return false;
    }
    //////////////////
    std::stringstream ssReg;
    ssReg <<"^"<< iTickerID <<"_\\d\\d\\d\\d\\d\\d$";
    const std::regex reControlfile {ssReg.str()};
    std::vector<std::filesystem::directory_entry> vControlFiles;

    std::string sYear{"1990"};
    std::string sMonth{"00"};

    std::copy_if(std::filesystem::directory_iterator{pathTickerDir},{},std::back_inserter(vControlFiles),[&reControlfile](const std::filesystem::directory_entry &c){
                     if ( c.exists() && c.is_regular_file()){
                         std::string ss(c.path().filename().string());
                         if (std::sregex_token_iterator(ss.begin(),ss.end(),reControlfile) != std::sregex_token_iterator())
                             return  true;
                         else
                            return false;
                     }
                     return false;
                 });
    //
    for(const auto &f:vControlFiles){

        std::string sBuff;
        int iState{0};
        std::ifstream ifCF(f.path());
        if(ifCF.good()){
            ifCF >>iState;
            if(iState <=0 || iState > 6){
                ssOut <<"control file "<< f.path().filename()<<" has invalid state value: <"<<iState<<">! Reset to 1.\n";
                ifCF.close();
                std::ofstream ofCF(f.path());
                if(ofCF.good()){
                    ofCF<<1;
                }
                else{

                    ssOut<<"Broken File: "<<f.path();
                    return false;
                }
            }
        }
        else{
            ssOut<<"Broken File: "<<f.path();
            return false;;
        }

        std::string sFileName(f.path().filename().string());

        CreateDataFilesForEntry(f.path().string(),sFileName,iState,ssOut);
        /////////////

        std::copy( std::next(sFileName.begin(),sFileName.size()-6)
                  ,std::next(sFileName.begin(),sFileName.size()-2)
                  ,sYear.begin());
        std::copy( std::next(sFileName.begin(),sFileName.size()-2)
                  ,sFileName.end()
                  ,sMonth.begin());

        std::tm tmT;
        tmT.tm_year = std::stoi(sYear)  - 1900;
        tmT.tm_mon  = std::stoi(sMonth) - 1;
        tmT.tm_mday = 1;
        tmT.tm_hour = 0;
        tmT.tm_min = 0;
        tmT.tm_sec = 0;
        tmT.tm_isdst = 0;
        std::time_t t = mktime_gm(&tmT);

        std::pair<int,std::time_t> k{iTickerID,t};
        mpStoreMutexes[k];
        mpWriteMutexes[k];
        mpOptimizeMutexes[k];
        mpStoreStages[k] = iState;
    }
    ///////////////////////////////////////////
    for (auto const &t:vInitializedTickers){
        if (t == iTickerID){
            return true;
        }
    }
    vInitializedTickers.push_back(iTickerID);
    return true;

}
//--------------------------------------------------------------------------------------------------------
void Storage::CreateDataFilesForEntry(std::string sFileName, std::string sFileNameShort, int iState,std::stringstream& ssOut){//private
    std::stringstream ss;
    ss <<sFileName<<"_1";
    if(vStorage1[iState - 1] && !std::filesystem::exists(ss.str())){
        if (sFileNameShort.size()>0)
            ssOut <<"data file ["<< sFileNameShort << "_1] is absent. Recreate.\n";
        std::ofstream ofF(ss.str());
    }
    if(!vStorage1[iState - 1] && std::filesystem::exists(ss.str())){
        if (sFileNameShort.size()>0)
            ssOut <<"data file ["<< sFileNameShort << "_1] need to free. Delete.\n";
        std::filesystem::remove(ss.str());
    }


    ss.clear();
    ss.str("");
    ss <<sFileName<<"_2";
    if(vStorage2[iState - 1] && !std::filesystem::exists(ss.str())){
        if (sFileNameShort.size()>0)
            ssOut <<"data file ["<< sFileNameShort << "_2] is absent! Recreate.\n";
        std::ofstream ofF(ss.str());
    }
    if(!vStorage2[iState - 1] && std::filesystem::exists(ss.str())){
        if (sFileNameShort.size()>0)
            ssOut <<"data file ["<< sFileNameShort << "_2] need to free. Delete.\n";
        std::filesystem::remove(ss.str());
    }

    ss.clear();
    ss.str("");
    ss <<sFileName<<"_3";
    if(vStorage3[iState - 1] && !std::filesystem::exists(ss.str())){
        if (sFileNameShort.size()>0)
            ssOut <<"data file ["<< sFileNameShort << "_3] is absent! Recreate.\n";
        std::ofstream ofF(ss.str());
    }
    if(!vStorage3[iState - 1] && std::filesystem::exists(ss.str())){
        if (sFileNameShort.size()>0)
            ssOut <<"data file ["<< sFileNameShort << "_3] need to free. Delete.\n";
        std::filesystem::remove(ss.str());
    }

    ss.clear();
    ss.str("");
    ss <<sFileName<<"_4";
    if(vStorage4[iState - 1] && !std::filesystem::exists(ss.str())){
        if (sFileNameShort.size()>0)
            ssOut <<"data file ["<< sFileNameShort << "_4] is absent! Recreate.\n";
        std::ofstream ofF(ss.str());
    }
    if(!vStorage4[iState - 1] && std::filesystem::exists(ss.str())){
        if (sFileNameShort.size()>0)
            ssOut <<"data file ["<< sFileNameShort << "_4] need to free. Delete.\n";
        std::filesystem::remove(ss.str());
    }
}
//--------------------------------------------------------------------------------------------------------
int Storage::CreateAndGetFileStageForTicker(int iTickerID, std::time_t tMonth, std::stringstream& ssOut)
{

    std::shared_lock lk(mutexQuotesStoreInit);
    tMonth = dateCastToMonth(tMonth);
    std::pair<int,std::time_t> k{iTickerID,tMonth};
    auto It (mpStoreMutexes.find(k));
    if(It != mpStoreMutexes.end()){
        std::shared_lock entryLk(mpStoreMutexes.at(k));
        return GetStageEntryForTicker(iTickerID, tMonth, ssOut);
    }
    else{
        lk.unlock();
        std::unique_lock<std::shared_mutex> ulk(mutexQuotesStoreInit);
        return CreateStageEntryForTicker(iTickerID, tMonth, ssOut);
    }
}
//--------------------------------------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief must be set std::unique_lock<std::shared_mutex> ulk(mutexQuotesStoreInit) befour call!!!
/// \param iTickerID
/// \param tMonth
/// \param ssOut
/// \return
///
int  Storage::CreateStageEntryForTicker(int iTickerID, std::time_t tMonth,std::stringstream& ssOut) // private
{

    std::pair<int,std::time_t> k{iTickerID,tMonth};
    auto It (mpStoreMutexes.find(k));
    if(It != mpStoreMutexes.end()){
        std::shared_lock entryLk(mpStoreMutexes.at(k));
        return GetStageEntryForTicker(iTickerID, tMonth, ssOut);
    }
    //////////////
    std::tm * tmT = threadfree_gmtime(&tMonth);

    std::stringstream ss;
    ss <<pathStorageDir.string();
    ss <<"/"<<iTickerID;
    ss <<"/"<<iTickerID<<"_";
    ss <<std::setfill('0');
    ss <<std::right<< std::setw(4)<<tmT->tm_year+1900;
    ss <<std::right<< std::setw(2)<<(tmT->tm_mon+1);
    std::string sFileName(ss.str());

    int iState{0};

    std::ofstream ofCF(sFileName);
    if(ofCF.good()){
        ofCF<<1;
        iState = 1;
    }
    else{
        ssOut <<"Broken File: "<<sFileName;
        return 0;
    }

    CreateDataFilesForEntry(sFileName,"",iState,ssOut);

    mpStoreMutexes[k];
    mpWriteMutexes[k];
    mpOptimizeMutexes[k];
    mpStoreStages[k] = iState;

    return iState;
}
//--------------------------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////
/// \brief must be set std::shared_lock entryLk(mpStoreMutexes[k]) befour call!!!
/// \param iTickerID
/// \param tMonth
/// \param ssOut
/// \return
///
int  Storage::GetStageEntryForTicker(int iTickerID, std::time_t tMonth,std::stringstream& ssOut) // private
{
    //
    std::tm * tmT = threadfree_gmtime(&tMonth);
    std::pair<int,std::time_t> k{iTickerID,tMonth};

    std::stringstream ss;
    ss <<pathStorageDir.string();
    ss <<"/"<<iTickerID;
    ss <<"/"<<iTickerID<<"_";
    ss <<std::setfill('0');
    ss <<std::right<< std::setw(4)<<tmT->tm_year+1900;
    ss <<std::right<< std::setw(2)<<(tmT->tm_mon+1);
    std::string sFileName(ss.str());

    int iState{0};
    std::ifstream ifCF(sFileName, std::ios_base::in);
    //if(ifCF.good()){
    if(ifCF){
        ifCF >>iState;
        if(iState <=0 || iState > 6){
            ssOut <<"control file "<< sFileName<<" has invalid state value: <"<<iState<<">!\n";
            return 0;
        }
    }
    else{
        ifCF >>iState;
        if (ifCF.rdstate() == std::ios_base::goodbit) { ssOut << "stream state is goodbit\n";}
        if (ifCF.rdstate() == std::ios_base::badbit) { ssOut << "stream state is badbit\n";}
        if (ifCF.rdstate() == std::ios_base::failbit) { ssOut << "stream state is failbit\n";}
        if (ifCF.rdstate() == std::ios_base::eofbit) { ssOut << "stream state is eofbit\n";}
        ssOut<<"state: ["<<iState<<"]\n";
        ssOut<<"Broken control file: "<<sFileName;
        return 0;
    }
    mpStoreStages[k] = iState;
    return  iState;
}
//--------------------------------------------------------------------------------------------------------
bool Storage::WriteBarToStore(int /*iTickerID*/, Bar &/*b*/, std::stringstream & /*ssOut*/)
{

//    std::shared_lock lk(mutexQuotesStoreInit);
//    //
//    std::time_t tMonth = dateCastToMonth(b.Period());
//    std::pair<int,std::time_t> k{iTickerID,tMonth};
//    auto It (mpStoreMutexes.find(k));
//    if(It == mpStoreMutexes.end()){
//        lk.unlock();
//        std::unique_lock<std::shared_mutex> ulk(mutexQuotesStoreInit);
//        if(CreateStageEntryForTicker(iTickerID, tMonth, ssOut) == 0){
//            ssOut <<"cannot create data storage file for:"<<iTickerID;
//            return false;
//        }
//        ulk.unlock();
//        lk.lock();
//        It = mpStoreMutexes.find(k);
//        if(It == mpStoreMutexes.end()){
//            ssOut <<"Logic error during creating data storage file for:"<<iTickerID;
//            return false;
//        }
//    }
//    //
//    std::shared_lock entryLk(mpStoreMutexes.at(k));
//    std::unique_lock entryWriteLk (mpWriteMutexes.at(k));
//    ////////////////////////////////////////////
//    //int iStage = GetStageEntryForTicker(iTickerID, tMonth, ssOut);
//    int iStage = mpStoreStages.at(k);
//    if (iStage <1 || iStage >6){
//        ssOut <<"Cannot get state for data storage file for:"<<iTickerID;
//        return false;
//    }
//    std::tm * tmT = threadfree_gmtime(&tMonth);

//    std::stringstream ss;
//    ss <<pathStorageDir.string();
//    ss <<"/"<<iTickerID;
//    ss <<"/"<<iTickerID<<"_";
//    ss <<std::setfill('0');
//    ss <<std::right<< std::setw(4)<<tmT->tm_year+1900;
//    ss <<std::right<< std::setw(2)<<(tmT->tm_mon+1);
//    ss <<"_"<<vStorageW[iStage-1];
//    std::string sFileName(ss.str());
//    ///
//    //ssOut <<"File to write:"<<sFileName<<"\n";

//    std::ofstream fileW (sFileName,std::ios_base::app | std::ios_base::binary);

//    data_type tp = data_type::new_sec;
//    fileW.write((char*)&tp,sizeof (tp));

//    double d = b.Open();
//    fileW.write((char*)&d,sizeof (d));
//    d = b.High();
//    fileW.write((char*)&d,sizeof (d));
//    d = b.Low();
//    fileW.write((char*)&d,sizeof (d));
//    d = b.Close();
//    fileW.write((char*)&d,sizeof (d));

//    unsigned long i = b.Volume();
//    fileW.write((char*)&i,sizeof (i));

//    time_t tT = b.Period();
//    fileW.write((char*)&tT,sizeof (tT));

//    ////////////////////////////////////////////
//    return true;
    return false;
}
//--------------------------------------------------------------------------------------------------------
bool Storage::slotParseLine(dataFinQuotesParse & parseDt, std::istringstream & issLine, Bar &b)
{

    parseDt.t_iCurrN = 0;

    try{
        while (std::getline(issLine,parseDt.t_sWordBuff,parseDt.Delimiter())){
            trim(parseDt.t_sWordBuff);
            parseDt.issTmp().clear();
            parseDt.issTmp().str(parseDt.t_sWordBuff);

            switch(parseDt.fields()[parseDt.t_iCurrN]){
            case dataFinQuotesParse::fieldType::TICKER:
                parseDt.t_sSign = parseDt.t_sWordBuff;
                if (parseDt.Sign().size() ==0){
                    parseDt.setDefaultSign(parseDt.t_sSign);
                }
                else if(parseDt.Sign() != parseDt.t_sSign){
                    parseDt.ossErr() << "Ticker sign mismatch";
                    return false;
                }
                break;
            case dataFinQuotesParse::fieldType::PER:
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
            case dataFinQuotesParse::fieldType::DATE:
                //20210322,
                if(parseDt.t_sWordBuff.size()>=8){
                    copy(parseDt.t_sWordBuff.begin()    ,parseDt.t_sWordBuff.begin() + 4,parseDt.t_sYear.begin());
                    copy(parseDt.t_sWordBuff.begin() + 4,parseDt.t_sWordBuff.begin() + 6,parseDt.t_sMonth.begin());
                    copy(parseDt.t_sWordBuff.begin() + 6,parseDt.t_sWordBuff.begin() + 8,parseDt.t_sDay.begin());
                    parseDt.t_tp.tm_year = std::stoi(parseDt.t_sYear) - 1900;
                    parseDt.t_tp.tm_mon = std::stoi(parseDt.t_sMonth) - 1;
                    parseDt.t_tp.tm_mday = std::stoi(parseDt.t_sDay);
                }
                else{
                    parseDt.ossErr() << "Wrong file format: wrong date field value";
                    return false;
                }
                break;
            case dataFinQuotesParse::fieldType::TIME:
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
            case dataFinQuotesParse::fieldType::OPEN:
                parseDt.issTmp() >> parseDt.t_dTmp; b.setOpen (parseDt.t_dTmp);
                break;
            case dataFinQuotesParse::fieldType::HIGH:
                parseDt.issTmp() >> parseDt.t_dTmp; b.setHigh (parseDt.t_dTmp);
                break;
            case dataFinQuotesParse::fieldType::LOW:
                parseDt.issTmp() >> parseDt.t_dTmp; b.setLow (parseDt.t_dTmp);
                break;
            case dataFinQuotesParse::fieldType::CLOSE:
                parseDt.issTmp() >> parseDt.t_dTmp; b.setClose (parseDt.t_dTmp);
                break;
            case dataFinQuotesParse::fieldType::LAST:
                parseDt.issTmp() >> parseDt.t_dTmp;
                b.setOpen   (parseDt.t_dTmp);
                b.setHigh   (b.Open());
                b.setLow    (b.Open());
                b.setClose  (b.Open());
                break;
            case dataFinQuotesParse::fieldType::VOL:
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

        b.setPeriod(mktime_gm(&parseDt.t_tp));
    }
    catch (std::exception &e){
        parseDt.ossErr() << "Wrong file format:"<<e.what();
        return false;
        }

    return  true;
}

//--------------------------------------------------------------------------------------------------------

bool Storage::WriteMemblockToStore(WriteMutexDefender &defLk,int iTickerID, std::time_t tMonth, char* cBuff,size_t length, std::stringstream & ssOut)
{
    std::shared_lock lk(mutexQuotesStoreInit);
    //
    tMonth = dateCastToMonth(tMonth);

//    char buffer[100];
//    std::tm * ptm = threadfree_gmtime(&tMonth);
//    std::strftime(buffer, 100, "%Y/%m/%d %H:%M:%S", ptm);
//    std::string strB(buffer);
//    {
//        ThreadFreeCout pcout;
//        pcout <<"\n\r";
//        pcout <<"writing: " <<strB<<"\n";
//    }

    std::pair<int,std::time_t> k{iTickerID,tMonth};
    auto It (mpStoreMutexes.find(k));
    if(It == mpStoreMutexes.end()){
        lk.unlock();
        std::unique_lock<std::shared_mutex> ulk(mutexQuotesStoreInit);
        if(CreateStageEntryForTicker(iTickerID, tMonth, ssOut) == 0){
            ssOut <<"cannot create data storage file for TickerID: "<<iTickerID;
            return false;
        }
        ulk.unlock();
        lk.lock();
        It = mpStoreMutexes.find(k);
        if(It == mpStoreMutexes.end()){
            ssOut <<"Logic error during creating data storage file for TickerID: "<<iTickerID;
            return false;
        }
    }

    std::shared_lock entryLk(mpStoreMutexes.at(k));
    //std::unique_lock entryWriteLk (mpWriteMutexes.at(k));
    defLk.Lock(mpWriteMutexes.at(k));
    ////////////////////////////////////////////
    //int iStage = GetStageEntryForTicker(iTickerID, tMonth, ssOut);
    int iStage = mpStoreStages.at(k);
    if (iStage <1 || iStage >6){
        ssOut <<"\n\rCannot get state for data storage file for TickerID: "<<iTickerID;
        return false;
    }
    std::tm * tmT = threadfree_gmtime(&tMonth);

    std::stringstream ss;
    ss <<pathStorageDir.string();
    ss <<"/"<<iTickerID;
    ss <<"/"<<iTickerID<<"_";
    ss <<std::setfill('0');
    ss <<std::right<< std::setw(4)<<tmT->tm_year+1900;
    ss <<std::right<< std::setw(2)<<(tmT->tm_mon+1);
    ss <<"_"<<vStorageW[iStage-1];
    std::string sFileName(ss.str());
    ///

    std::ofstream fileW (sFileName,std::ios_base::app | std::ios_base::binary);

    if(!fileW.write(cBuff,length)){
        ssOut <<"Unsuccessful write operation\n";
        return false;
    }

    ////////////////////////////////////////////
    return true;

}

//--------------------------------------------------------------------------------------------------------
bool Storage::ReadFromStore(int iTickerID, std::time_t tMonth, std::vector<BarTick> & vBarList,
                   std::time_t dtLoadBegin, std::time_t dtLoadEnd,
                   bool bFilterRepo,        Market::SessionTable_type  &vRepoTable,
                   bool bFilterSessionTable,Market::SessionTable_type  &vSessionTable,
                   std::stringstream & ssOut)
{
    char buffer[100];
    std::tm * ptm = threadfree_gmtime(&tMonth);
    std::strftime(buffer, 100, "%Y/%m/%d %H:%M:%S", ptm);
    std::string strM(buffer);


    ////////////////////////////////////////////////

    tMonth = dateCastToMonth(tMonth);
    std::pair<int,std::time_t> k{iTickerID,tMonth};

    std::shared_lock lk(mutexQuotesStoreInit);

    auto It (mpStoreMutexes.find(k));
    if(It != mpStoreMutexes.end()){
        std::shared_lock entryLk(mpStoreMutexes.at(k));
        //std::shared_lock entryWriteLk (mpWriteMutexes.at(k)); // do lock below, befoure use writing file


        int iStage = mpStoreStages.at(k);
        if (iStage <1 || iStage >6){
            ssOut <<"\n\rCannot get state for data storage file for TickerID: "<<iTickerID;
            return false;
        }
        ///
        std::tm * tmT = threadfree_gmtime(&tMonth);


        std::stringstream ss;
        ss <<iTickerID<<"_";
        ss <<std::setfill('0');
        ss <<std::right<< std::setw(4)<<tmT->tm_year+1900;
        ss <<std::right<< std::setw(2)<<(tmT->tm_mon+1);
        ss <<"_";

        std::map<std::time_t,std::vector<BarTick>> mvHolder;


        if (vStorageL1[iStage - 1]>0){
            std::stringstream ssNamePart;
            ssNamePart << ss.str()<<vStorageL1[iStage-1];
            if (!ReadFromStoreFile(iTickerID, tMonth,mvHolder,dtLoadBegin, dtLoadEnd,ssNamePart.str(),
                                   bFilterRepo,vRepoTable,
                                   bFilterSessionTable,vSessionTable,
                                   ssOut)){
                return false;
            }
        }

        if (vStorageL2[iStage - 1]>0){
            std::stringstream ssNamePart;
            ssNamePart << ss.str()<<vStorageL2[iStage-1];
            if (!ReadFromStoreFile(iTickerID, tMonth,mvHolder,dtLoadBegin, dtLoadEnd,ssNamePart.str(),
                                   bFilterRepo,vRepoTable,
                                   bFilterSessionTable,vSessionTable,
                                   ssOut)){
                return false;
            }
        }

        std::shared_lock entryWriteLk (mpWriteMutexes.at(k)); // lock

        if (vStorageW[iStage - 1]>0){
            std::stringstream ssNamePart;
            ssNamePart << ss.str()<<vStorageW[iStage-1];
            if (!ReadFromStoreFile(iTickerID, tMonth,mvHolder,dtLoadBegin, dtLoadEnd,ssNamePart.str(),
                                   bFilterRepo,vRepoTable,
                                   bFilterSessionTable,vSessionTable,
                                   ssOut)){
                return false;
            }
        }

        size_t iRecords{0};
        for(auto &e:mvHolder){
            iRecords += e.second.size();
        }

        vBarList.reserve(iRecords);
        for(auto &e:mvHolder){
            std::copy(e.second.begin(),e.second.end(),std::back_inserter(vBarList));
        }

        /////////////////////////////////////////////////////////////
        vBarList.shrink_to_fit();
        ssOut<<"active records: <"<<vBarList.size()<<">\n";

    }
    else{
        //ssOut <<"no data files for: " <<strM<<"\n";;
        //return true;
    }
    return true;
}
//--------------------------------------------------------------------------------------------------------
bool Storage::ReadFromStoreFile(int iTickerID, std::time_t /*tMonth*/, std::map<std::time_t,std::vector<BarTick>> &mvHolder,
                   std::time_t dtLoadBegin, std::time_t dtLoadEnd,
                   std::string strNamePart,
                   bool bFilterRepo,        Market::SessionTable_type  &vRepoTable,
                   bool bFilterSessionTable,Market::SessionTable_type  &vSessionTable,
                   std::stringstream & ssOut)
{
    std::stringstream ss;

    ss.str("");
    ss.clear();
    ss << pathStorageDir.string();
    ss <<"/"<<iTickerID<<"/";
    ss <<strNamePart;

    std::string sFileName(ss.str());
    ////
    bool bLastWasRepo{false};
    bool bLastWasSessionTable{false};
    std::time_t tLastTime{0};
    //////////////////////////////////////////////////////////////////////////////////////////////////

    std::ifstream fileR (sFileName,std::ios_base::in | std::ios_base::binary);
    if(fileR){

        fileR.seekg(0,std::ios::end);
        size_t filesize = fileR.tellg();
        fileR.seekg(0, std::ios::beg);

        BarTick b(0,0,0);
        BarTickMemcopier bM(b);
        const int iBlockSize (  sizeof (Storage::data_type)
                        + sizeof (b.Close())
                        + sizeof (b.Volume())
                        + sizeof (b.Period())
                        );
        char cBuffer[iBlockSize];
        int iInBuffPointer{0};
        int iState{0};

        const size_t iRecordsMax {filesize/iBlockSize};


        //std::map<std::time_t,std::vector<BarTick>> mvHolder;

        ssOut<<"Storage contains "<<iRecordsMax<<" records.\n";


        std::time_t tmStartDel{0};
        std::time_t tmEndDel{0};
        bool bWasRawDelete{false};


        while(fileR.read(cBuffer,iBlockSize)){

            iInBuffPointer = 0;

            memcpy(&iState,     cBuffer + iInBuffPointer, sizeof (Storage::data_type));   iInBuffPointer += sizeof (Storage::data_type);
            memcpy(&bM.Close(), cBuffer + iInBuffPointer, sizeof (bM.Close()));           iInBuffPointer += sizeof (bM.Close());
            memcpy(&bM.Volume(),cBuffer + iInBuffPointer, sizeof (bM.Volume()));          iInBuffPointer += sizeof (bM.Volume());
            memcpy(&bM.Period(),cBuffer + iInBuffPointer, sizeof (bM.Period()));          iInBuffPointer += sizeof (bM.Period());
            ///////////////
            if (iState == Storage::data_type::del_from){
                if (bWasRawDelete){
                    ssOut<<"Broken delete sequence in storage file: "<<sFileName;
                    return false;
                }
                tmStartDel = b.Period();
                bWasRawDelete = true;
            }
            else if (iState == Storage::data_type::del_to){
                tmEndDel = b.Period();

                for(auto & e:mvHolder){
                    if (e.first >= tmStartDel && e.first <= tmEndDel)
                        e.second.clear();
                }
                bWasRawDelete = false;
            }
            else{
                if (bWasRawDelete){
                    ssOut<<"Broken delete sequence in storage file: "<<sFileName;
                    return false;
                }
            }
            if (b.Period() >= dtLoadBegin && b.Period() <= dtLoadEnd){

                if (iState == Storage::data_type::usual || iState == Storage::data_type::new_sec){
                    if (tLastTime != b.Period()){
                        tLastTime = b.Period();
                        if (bFilterRepo ){
                            bLastWasRepo = Market::IsInSessionTabe(vRepoTable,tLastTime);
                        }
                        if (bFilterSessionTable){
                            bLastWasSessionTable = Market::IsInSessionTabe(vSessionTable,tLastTime);
                        }
                    }

                    if ((!bFilterRepo         ||  (bFilterRepo && !bLastWasRepo)) &&
                        (!bFilterSessionTable ||  (bFilterSessionTable && bLastWasSessionTable))
                            )
                    {
                        if (iState == Storage::data_type::new_sec){
                            mvHolder[b.Period()].clear();

                        }
                        mvHolder[b.Period()].push_back(b);
                    }
                }
            }
            /////////////////////////////////////////////////////////////
            if(this_thread_flagInterrup.isSet()){
                ssOut<<"loading process interrupted\n";
                return false;
            }
        }
        /////////////////////////////////////////////////////////////
        /////////////////////////////////////////////////////////////

        ssOut<<"Read from file: "<<strNamePart<<"\n";
        return true;
    }
    else{
        if (fileR.rdstate() == std::ios_base::goodbit) { ssOut << "stream state is goodbit\n";}
        if (fileR.rdstate() == std::ios_base::badbit) { ssOut << "stream state is badbit\n";}
        if (fileR.rdstate() == std::ios_base::failbit) { ssOut << "stream state is failbit\n";}
        if (fileR.rdstate() == std::ios_base::eofbit) { ssOut << "stream state is eofbit\n";}

        ssOut<<"Broken storage file: "<<sFileName;
        return false;
    }
    return true;
}
//--------------------------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////
// file switch stages:
//      1   2   3   4   5   6   (7)
//  1.  W   L2  L2  U   W   W   (W)
//  2.  U   W   W   W   L2  L2  (U)
//  3.  U   U   S   L1  L1  L1  (U)
//  4.  L1  L1  L1  U   U   S   (L1)
//
// Description:
//  W   - write (append)
//  U   - undefined (unused)
//  L1  - read only (read order: first)
//  L2  - read only (read order: second)
//  S   - shrinked (compilation of L1 + L2)

//    ,vStorageW  {1,2,2,2,1,1}
//    ,vStorageL1 {4,4,4,3,3,3}
//    ,vStorageL2 {0,1,1,0,2,2}
//    ,vStorageS  {0,0,3,0,0,4}

//    ,vStorage1  {true ,true ,true ,false,true ,true }
//    ,vStorage2  {false,true ,true ,true ,true ,true }
//    ,vStorage3  {false,false,true ,true ,true ,true }
//    ,vStorage4  {true ,true ,true ,false,false,true }
//

bool Storage::OptimizeStore(int iTickerID, std::time_t tMonth, bool & bToPlanNextShift,
                   std::stringstream & ssOut)
{

    bToPlanNextShift = false;

    char buffer[100];
    std::tm * ptm = threadfree_gmtime(&tMonth);
    std::strftime(buffer, 100, "%Y/%m/%d %H:%M:%S", ptm);
    std::string strM(buffer);
    ////////////////////////////////////////////////
    tMonth = dateCastToMonth(tMonth);
    std::pair<int,std::time_t> k{iTickerID,tMonth};
    std::shared_lock lkInit(mutexQuotesStoreInit);
    ////////////////////////////////////////////////
    Market::SessionTable_type  vRepoNull{};

    auto It (mpOptimizeMutexes.find(k));
    if(It != mpOptimizeMutexes.end()){
        std::unique_lock lk(mpOptimizeMutexes.at(k),std::defer_lock);
        if (lk.try_lock()){

            std::vector<std::vector<BarTick>> vBarList;

            int iStage = mpStoreStages.at(k);
            if (iStage <1 || iStage >6){
                ssOut <<"\n\rCannot get state for data storage file for TickerID: "<<iTickerID;
                return false;
            }
            ///
            int iNewStage = iStage >= 6 ? 1 : iStage + 1;
            ///

            std::tm * tmT = threadfree_gmtime(&tMonth);

            std::stringstream ssControlFileNamePart;
            ssControlFileNamePart <<iTickerID<<"_";
            ssControlFileNamePart <<std::setfill('0');
            ssControlFileNamePart <<std::right<< std::setw(4)<<tmT->tm_year+1900;
            ssControlFileNamePart <<std::right<< std::setw(2)<<(tmT->tm_mon+1);

            std::stringstream ssControlPath;
            ssControlPath << pathStorageDir.string();
            ssControlPath <<"/"<<iTickerID<<"/";
            ssControlPath <<ssControlFileNamePart.str();


            //if( iStage <= 5) // TODO: delete. for tests
            {

            if (vStorageS[iStage - 1] == 0){



                SwitchStage(k,ssControlPath.str(), ssControlFileNamePart.str(), iNewStage,ssOut);

                bToPlanNextShift = true;
                //return true;
            }
            else{

                std::time_t dtLoadBegin = Bar::DateAccommodate(tMonth,Bar::eInterval::pMonth);
                std::time_t dtLoadEnd = Bar::DateAccommodate(tMonth,Bar::eInterval::pMonth,true) - 1;


                std::map<std::time_t,std::vector<BarTick>> mvHolder;

                if (vStorageL1[iStage - 1]>0){
                    std::stringstream ssNamePart;
                    ssNamePart << ssControlFileNamePart.str()<<"_"<<vStorageL1[iStage-1];
                    if (!ReadFromStoreFile(iTickerID, tMonth,mvHolder,dtLoadBegin, dtLoadEnd,ssNamePart.str(),false,vRepoNull,false,vRepoNull,ssOut)){
                        return false;
                    }
                }


                if (vStorageL2[iStage - 1]>0){
                    std::stringstream ssNamePart;
                    ssNamePart << ssControlFileNamePart.str()<<"_"<<vStorageL2[iStage-1];
                    if (!ReadFromStoreFile(iTickerID, tMonth,mvHolder,dtLoadBegin, dtLoadEnd,ssNamePart.str(),false,vRepoNull,false,vRepoNull,ssOut)){
                        return false;
                    }
                }


                std::stringstream ssFullName;
                ssFullName <<ssControlPath.str()<<"_"<<vStorageS[iStage-1];

                if (!WriteMapToStore(ssFullName.str(),  mvHolder, ssOut)){
                    return false;
                }

                SwitchStage(k,ssControlPath.str(), ssControlFileNamePart.str(), iNewStage, ssOut);

                //bToPlanNextShift = false;
                //return true;
            }
            }
        }
        else{
            //now optimisation is going on somewere
        }
    }
    else{
        //no data files to optimize
    }
    return true;
}
//--------------------------------------------------------------------------------------------------------
bool Storage::SwitchStage(std::pair<int,std::time_t> k, std::string sPath, std::string sFileNamePart, int iNewStage,
                          std::stringstream & ssOut)
{
    std::unique_lock entryLk(mpStoreMutexes.at(k));
    mpStoreStages[k] = iNewStage;

    std::ofstream ofCF(sPath);
    if(ofCF.good()){
        ofCF<<iNewStage;
    }
    else{
        ssOut<<"Broken File: "<<sPath;
        return false;
    }

    std::stringstream ssDevNull;
    CreateDataFilesForEntry(sPath,sFileNamePart,iNewStage,ssDevNull);
    ssOut << ssDevNull.str(); // TODO: delete. for tests
    return true;
}
//--------------------------------------------------------------------------------------------------------

bool Storage::WriteMapToStore(std::string sFileName, std::map<std::time_t,std::vector<BarTick>> & mvHolder, std::stringstream & ssOut)
{
    BarTick bb(0,0,0);
    BarTickMemcopier bM(bb);
    const int iOutBuffMax {65535};
    char cOutBuff[iOutBuffMax];
    int iOutBuffPointer {0};
    Storage::data_type iState = Storage::data_type::new_sec;

    int iBlockSize (  sizeof (Storage::data_type)
                    + sizeof (bb.Close())
                    + sizeof (bb.Volume())
                    + sizeof (bb.Period())
                    );

//    if (std::filesystem::exists(sFileName) && !std::filesystem::remove(sFileName)){
//        ssOut <<"Cannot clear storage file\n";
//        return false;
//    }

    std::ofstream fileW (sFileName,/*std::ios_base::app |*/ std::ios_base::binary);

    if(fileW){

        for (const auto & v:mvHolder){
            if (iOutBuffPointer!=0 && iOutBuffPointer + iBlockSize * v.second.size() >  iOutBuffMax ){
                ///write
                if(!fileW.write(cOutBuff,iOutBuffPointer)){
                    ssOut <<"Unsuccessful write operation\n";
                    return false;
                }
                iOutBuffPointer = 0;
            }
            auto It (v.second.begin());
            if (It != v.second.end()){

                bb = *It;

                iState = Storage::data_type::new_sec;
                memcpy(cOutBuff + iOutBuffPointer,&iState,      sizeof (Storage::data_type));      iOutBuffPointer += sizeof (Storage::data_type);
                memcpy(cOutBuff + iOutBuffPointer,&bM.Close(),  sizeof (bM.Close()));           iOutBuffPointer += sizeof (bM.Close());
                memcpy(cOutBuff + iOutBuffPointer,&bM.Volume(), sizeof (bM.Volume()));          iOutBuffPointer += sizeof (bM.Volume());
                memcpy(cOutBuff + iOutBuffPointer,&bM.Period(), sizeof (bM.Period()));          iOutBuffPointer += sizeof (bM.Period());


                iState = Storage::data_type::usual;
                It++;

            }
            while (It != v.second.end()){
                if (iOutBuffPointer + iBlockSize >  iOutBuffMax ){
                    ///write
                    if(!fileW.write(cOutBuff,iOutBuffPointer)){
                        ssOut <<"Unsuccessful write operation\n";
                        return false;
                    }
                    iOutBuffPointer = 0;
                }
                bb = *It;

                memcpy(cOutBuff + iOutBuffPointer,&iState,   sizeof (Storage::data_type));      iOutBuffPointer += sizeof (Storage::data_type);
                memcpy(cOutBuff + iOutBuffPointer,&bM.Close(),  sizeof (bM.Close()));           iOutBuffPointer += sizeof (bM.Close());
                memcpy(cOutBuff + iOutBuffPointer,&bM.Volume(), sizeof (bM.Volume()));          iOutBuffPointer += sizeof (bM.Volume());
                memcpy(cOutBuff + iOutBuffPointer,&bM.Period(), sizeof (bM.Period()));          iOutBuffPointer += sizeof (bM.Period());

                if(this_thread_flagInterrup.isSet()){
                    ssOut<<"loading process interrupted";
                    return  false;
                }

                It++;
            }
        }
        // write
        if (iOutBuffPointer > 0){
            if(!fileW.write(cOutBuff,iOutBuffPointer)){
                ssOut <<"Unsuccessful write operation\n";
                return false;
            }
        }
    }
    else{
        if (fileW.rdstate() == std::ios_base::goodbit) { ssOut << "stream state is goodbit\n";}
        if (fileW.rdstate() == std::ios_base::badbit) { ssOut << "stream state is badbit\n";}
        if (fileW.rdstate() == std::ios_base::failbit) { ssOut << "stream state is failbit\n";}
        if (fileW.rdstate() == std::ios_base::eofbit) { ssOut << "stream state is eofbit\n";}

        ssOut<<"Broken storage file: "<<sFileName;
        return false;
    }

    ////////////////////////////////////////////
    return true;

}
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
