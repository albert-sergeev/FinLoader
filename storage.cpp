#include "storage.h"

#include<filesystem>
#include<iostream>
#include<regex>
#include<iomanip>
#include<fstream>
#include<ostream>
#include<chrono>

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

    ,vStorage1  {true ,true ,true ,false,true ,true ,}
    ,vStorage2  {false,true ,true ,true ,true ,true ,}
    ,vStorage3  {false,false,true ,true ,true ,true ,}
    ,vStorage4  {true ,true ,true ,false,false,true ,}
{


}
//--------------------------------------------------------------------------------------------------------
void Storage::Initialize()
{

    /// for speed optimisation
    std::ios::sync_with_stdio(false);

    //std::filesystem::path pathCurr;
    //std::filesystem::path pathDataDir;
    //std::filesystem::path pathStorageDir;
    //
    pathCurr = std::filesystem::current_path();
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
        std::vector<Market> m{{trim("ММВБ"),trim("MICEX_SHR_T")}};
        SaveMarketConfig(m);
        if(!std::filesystem::exists(pathMarkersFile)){
            throw std::runtime_error("Error create file: ./data/markets.dat");
        }
    }
    if(   !std::filesystem::is_regular_file(pathMarkersFile)){
        throw std::runtime_error("./data/markets.dat - has wrong type");
    }
    ///========
    pathTickersFile = std::filesystem::absolute(pathDataDir/"tickers.dat");
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
    if (!bInitialized) Initialize();
    //
    std::string sBuff;
    std::istringstream iss{sBuff};
    std::ifstream fileMarket(pathMarkersFile.c_str());
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
    SaveMarketConfigV_1(vMarketsLst);
}
void Storage::SaveMarketConfig(std::vector<Market> && vMarketsLst)
{
    SaveMarketConfig(vMarketsLst);
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
void Storage::SaveTickerConfig(const Ticker & tT, op_type tp)
{
    SaveTickerConfigV_1(tT,tp);
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
void Storage::SaveTickerConfigV_1(const Ticker & tT, op_type tp)
{

        std::ofstream fileTicker(pathTickersFile,std::ios_base::app);

        fileTicker<<(iTickerMark++)<<",";

        fileTicker<<tp<<",";

        fileTicker<<tT.MarketID()<<",";
        fileTicker<<tT.TickerID()<<",";

        fileTicker<<tT.TickerName()<<",";
        fileTicker<<tT.TickerSign()<<",";
        fileTicker<<tT.TickerSignFinam()<<",";
        fileTicker<<tT.TickerSignQuik()<<",";
        fileTicker<<tT.AutoLoad()<<",";
        fileTicker<<tT.UpToSys()<<",";

        fileTicker<<"\n";

};

//--------------------------------------------------------------------------------------------------------
void Storage::LoadTickerConfig(std::vector<Ticker> & vTickersLst)
{
    if (!bInitialized) Initialize();
    //
    std::string sBuff;
    std::istringstream iss{sBuff};
    std::ifstream fileTicker(pathTickersFile.c_str());

//    if (fileTicker.good()){
//        ParsTickerConfigV_1(vTickersLst,fileTicker);
//    }
//    else{
//        std::stringstream ss;
//        ss <<"error reading file: "<<pathTickersFile;
//        throw std::runtime_error(ss.str());
//    }


    if(std::getline(fileTicker,sBuff)){
        if(sBuff == "v1"){
            ParsTickerConfigV_1(vTickersLst,fileTicker);
        }
        else{
            throw std::runtime_error("wrong file format ./data/Tickers.dat!");
        }
    }
    else{
        std::stringstream ss;
        ss <<"error reading file: "<<pathTickersFile;
        throw std::runtime_error(ss.str());
    }

};
//--------------------------------------------------------------------------------------------------------
void Storage::ParsTickerConfigV_1(std::vector<Ticker> & vTickersLst, std::ifstream & file)
{
    int iMark{0};
    op_type tp;

    int iMarketID{0};
    int iTickerID{0};


    std::map<int,int> mM;

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

        if(vS.size()<10){
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


        auto ItM (mM.find(t.TickerID()));

        if(ItM != mM.end()){
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


};
//--------------------------------------------------------------------------------------------------------

std::time_t Storage::dateCastToMonth(std::time_t t)
{
    std::tm* tmT = std::localtime(&t);
    std::tm tmNew;

    tmNew.tm_year   = tmT->tm_year;
    tmNew.tm_mon    = tmT->tm_mon;
    tmNew.tm_mday   = 1;
    tmNew.tm_hour   = 0;
    tmNew.tm_min    = 0;
    tmNew.tm_sec    = 0;
    tmNew.tm_isdst   = tmT->tm_isdst;

    return std::mktime(&tmNew);
}
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
    ssD <<pathStorageDir.c_str();
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
                         std::string ss(c.path().filename());
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

        std::string sFileName(f.path().filename());

        CreateDataFilesForEntry(f.path(),sFileName,iState,ssOut);
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
        std::time_t t = std::mktime(&tmT);

        std::pair<int,std::time_t> k{iTickerID,t};
        mpStoreMutexes[k];
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
            ssOut <<"data file ["<< sFileNameShort << "_1] is absent! Recreate.\n";
        std::ofstream ofF(ss.str());
    }

    ss.clear();
    ss.str("");
    ss <<sFileName<<"_2";
    if(vStorage2[iState - 1] && !std::filesystem::exists(ss.str())){
        if (sFileNameShort.size()>0)
            ssOut <<"data file ["<< sFileNameShort << "_2] is absent! Recreate.\n";
        std::ofstream ofF(ss.str());
    }
    ss.clear();
    ss.str("");
    ss <<sFileName<<"_3";
    if(vStorage3[iState - 1] && !std::filesystem::exists(ss.str())){
        if (sFileNameShort.size()>0)
            ssOut <<"data file ["<< sFileNameShort << "_3] is absent! Recreate.\n";
        std::ofstream ofF(ss.str());
    }
    ss.clear();
    ss.str("");
    ss <<sFileName<<"_4";
    if(vStorage4[iState - 1] && !std::filesystem::exists(ss.str())){
        if (sFileNameShort.size()>0)
            ssOut <<"data file ["<< sFileNameShort << "_4] is absent! Recreate.\n";
        std::ofstream ofF(ss.str());
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
        std::shared_lock entryLk(mpStoreMutexes[k]);
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
        std::shared_lock entryLk(mpStoreMutexes[k]);
        return GetStageEntryForTicker(iTickerID, tMonth, ssOut);
    }
    //////////////
    std::tm * tmT = std::localtime(&tMonth);

    std::stringstream ss;
    ss <<pathStorageDir.c_str();
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
    std::tm * tmT = std::localtime(&tMonth);

    std::stringstream ss;
    ss <<pathStorageDir.c_str();
    ss <<"/"<<iTickerID;
    ss <<"/"<<iTickerID<<"_";
    ss <<std::setfill('0');
    ss <<std::right<< std::setw(4)<<tmT->tm_year+1900;
    ss <<std::right<< std::setw(2)<<(tmT->tm_mon+1);
    std::string sFileName(ss.str());

    int iState{0};
    std::ifstream ifCF(sFileName);
    if(ifCF.good()){
        ifCF >>iState;
        if(iState <=0 || iState > 6){
            ssOut <<"control file "<< sFileName<<" has invalid state value: <"<<iState<<">! Reset to 1.\n";
            ifCF.close();
            std::ofstream ofCF(sFileName);
            if(ofCF.good()){
                ofCF<<1;
                iState = 1;
            }
            else{
                ssOut<<"Broken File: "<<sFileName;
                return 0;
            }
        }
    }
    else{
        ssOut<<"Broken File: "<<sFileName;
        return 0;
    }
    return  iState;
}
//--------------------------------------------------------------------------------------------------------
bool Storage::WriteBarToStore(int iTickerID, Bar &b, std::stringstream & ssOut)
{
    std::shared_lock lk(mutexQuotesStoreInit);
    //
    std::time_t tMonth = dateCastToMonth(b.Period());
    std::pair<int,std::time_t> k{iTickerID,tMonth};
    auto It (mpStoreMutexes.find(k));
    if(It == mpStoreMutexes.end()){
        lk.unlock();
        std::unique_lock<std::shared_mutex> ulk(mutexQuotesStoreInit);
        if(CreateStageEntryForTicker(iTickerID, tMonth, ssOut) == 0){
            ssOut <<"cannot create data storage file for:"<<iTickerID;
            return false;
        }
        ulk.unlock();
        lk.lock();
        It = mpStoreMutexes.find(k);
        if(It == mpStoreMutexes.end()){
            ssOut <<"Logic error during creating data storage file for:"<<iTickerID;
            return false;
        }
    }
    //
    std::unique_lock entryLk(mpStoreMutexes[k]);
    ////////////////////////////////////////////
    int iStage = GetStageEntryForTicker(iTickerID, tMonth, ssOut);
    if (iStage <1 || iStage >6){
        ssOut <<"Cannot get state for data storage file for:"<<iTickerID;
        return false;
    }
    std::tm * tmT = std::localtime(&tMonth);

    std::stringstream ss;
    ss <<pathStorageDir.c_str();
    ss <<"/"<<iTickerID;
    ss <<"/"<<iTickerID<<"_";
    ss <<std::setfill('0');
    ss <<std::right<< std::setw(4)<<tmT->tm_year+1900;
    ss <<std::right<< std::setw(2)<<(tmT->tm_mon+1);
    ss <<"_"<<vStorageW[iStage-1];
    std::string sFileName(ss.str());
    ///
    //ssOut <<"File to write:"<<sFileName<<"\n";

    //std::ofstream fileW (sFileName,std::ios_base::app | std::ios_base::binary);
    std::ofstream fileW (sFileName, std::ios_base::binary);

    data_type tp = data_type::new_sec;
    fileW.write((char*)&tp,sizeof (tp));

    double d = b.Open();
    fileW.write((char*)&d,sizeof (d));
    d = b.High();
    fileW.write((char*)&d,sizeof (d));
    d = b.Low();
    fileW.write((char*)&d,sizeof (d));
    d = b.Close();
    fileW.write((char*)&d,sizeof (d));

    int i = b.Volume();
    fileW.write((char*)&i,sizeof (i));

    time_t tT = b.Period();
    fileW.write((char*)&tT,sizeof (tT));

    ////////////////////////////////////////////
    return true;
}
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
