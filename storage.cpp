#include "storage.h"

#include<filesystem>
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
    bInitialized = true;
}
//--------------------------------------------------------------------------------------------------------
void Storage::LoadMarketConfig(std::vector<Market> & vMarketsLst)
{
    if (!bInitialized) Initialize();
    //
    ///=======
    /// for speed optimisation
    //std::ios::sync_with_stdio(false);
    ///=======
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

    ///=======
    //std::ios::sync_with_stdio(true);
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
    //
    //std::ios::sync_with_stdio(false);
    //
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
    //
    //std::ios::sync_with_stdio(true);
    //
}
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
