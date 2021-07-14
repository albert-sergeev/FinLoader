#include "storage.h"

#include<filesystem>
#include<iostream>
#include<fstream>
#include<ostream>

//using namespace std::filesystem;

//--------------------------------------------------------------------------------------------------------
Storage::Storage():bInitialized{false}
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
        std::vector<Market> m{{"MOEXXXX","MOEX"}};
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
        std::ofstream fileTickers(pathTickersFile);
        std::vector<Ticker> t;
        SaveTickerConfig(t,0,0);
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
        if(std::stol(sBuff) == 1){
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
    fileMarket <<"1\n";
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
void Storage::SaveTickerConfig(std::vector<Ticker> & vTickersLst, int iLeft, int iRight)
{
    SaveTickerConfigV_1(vTickersLst,iLeft,iRight);
};
//--------------------------------------------------------------------------------------------------------
void Storage::SaveTickerConfig(std::vector<Ticker> && vTickersLst, int iLeft, int iRight)
{
    SaveTickerConfig(vTickersLst,iLeft,iRight);
};
//--------------------------------------------------------------------------------------------------------
void Storage::SaveTickerConfigV_1(std::vector<Ticker> & vTickersLst, int iLeft, int iRight)
{
    std::ofstream fileTicker(pathTickersFile,std::ios_base::app);
    //fileTicker <<"1\n";
    //
    for(int i = iLeft; i < (int)vTickersLst.size() && i <= iRight; ++i){

        fileTicker<<(++iTickerMark)<<",";

        fileTicker<<vTickersLst[i].MarketID()<<",";
        fileTicker<<vTickersLst[i].TickerID()<<",";

        fileTicker<<vTickersLst[i].TickerName()<<",";
        fileTicker<<vTickersLst[i].TickerSign()<<",";
        fileTicker<<vTickersLst[i].TickerSignFinam()<<",";
        fileTicker<<vTickersLst[i].TickerSignQuik()<<",";
        fileTicker<<vTickersLst[i].AutoLoad()<<",";
        fileTicker<<vTickersLst[i].UpToSys()<<",";

        fileTicker<<"\n";
    }
};

//--------------------------------------------------------------------------------------------------------
void Storage::LoadTickerConfig(std::vector<Ticker> & vTickersLst)
{
    if (!bInitialized) Initialize();
    //
    std::string sBuff;
    std::istringstream iss{sBuff};
    std::ifstream fileTicker(pathTickersFile.c_str());

    if (fileTicker.good()){
        ParsTickerConfigV_1(vTickersLst,fileTicker);
    }
    else{
        std::stringstream ss;
        ss <<"error reading file: "<<pathTickersFile;
        throw std::runtime_error(ss.str());
    }


//    if(std::getline(fileTicker,sBuff)){
//        if(std::stol(sBuff) == 1){
//            ParsTickerConfigV_1(vTickersLst,fileTicker);
//        }
//        else{
//            throw std::runtime_error("wrong file format ./data/Tickers.dat!");
//        }
//    }
//    else{
//        std::stringstream ss;
//        ss <<"error reading file: "<<pathTickersFile;
//        throw std::runtime_error(ss.str());
//    }

};
//--------------------------------------------------------------------------------------------------------
void Storage::ParsTickerConfigV_1(std::vector<Ticker> & vTickersLst, std::ifstream & file)
{
    int iMark{0};

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

        if(vS.size()<9){
            std::stringstream ss;
            ss <<"error parsing file. wrong format: "<<pathTickersFile;
            throw std::runtime_error(ss.str());
        }
        copy(vS.begin(),vS.end(),std::ostream_iterator<std::string>(std::cout,","));std::cout<<"\n";

        //
        iMark                   =std::stoi(vS[0]);
        iMarketID               =std::stoi(vS[1]);
        iTickerID               =std::stoi(vS[2]);

        Ticker t{iTickerID,vS[3],vS[4],iMarketID};

        t.SetTickerSignFinam    (vS[5]);
        t.SetTickerSignQuik     (vS[6]);
        t.SetAutoLoad           (std::stoi(vS[7]));
        t.SetUpToSys            (std::stoi(vS[8]));


        auto ItM (mM.find(iTickerID));

        if(ItM != mM.end()){
           vTickersLst[ItM->second] = t;
        }
        else{
            mM[iTickerID] = vTickersLst.size();
            vTickersLst.push_back(t);
        }
    }


};

//--------------------------------------------------------------------------------------------------------
