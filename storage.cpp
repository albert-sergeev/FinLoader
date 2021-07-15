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
        std::vector<Market> m{{"ММВБ","MICEX_SHR_T"}};
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
