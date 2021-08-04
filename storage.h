#ifndef STORAGE_H
#define STORAGE_H

#include<mutex>
#include<shared_mutex>
#include<filesystem>
#include "ticker.h"


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
///////////////////////////////////////////////////////////////////
/// \brief The Storage class
///

class Storage
{
    bool bInitialized;

    std::filesystem::directory_entry drCurr;

    std::filesystem::path pathCurr;
    std::filesystem::path pathDataDir;
    std::filesystem::path pathStorageDir;
    std::filesystem::path pathMarkersFile;
    std::filesystem::path pathTickersFile;

    //---------------------------------------
    // for tickers configs
    int iTickerMark{1};
    //---------------------------------------
    // for market quotes storage
    std::shared_mutex mutexQuotesStoreInit;
    std::vector<int> vInitializedTickers;
    std::map<std::pair<int,std::time_t>,std::shared_mutex> mpStoreMutexes;
    std::map<std::pair<int,std::time_t>,int> mpStoreStages;

public:

    inline std::string GetCurrentPath() {return  pathCurr;};
    inline std::string GetDataPath()    {return  pathDataDir;};
    inline std::string GetStoragePath() {return  pathStorageDir;};


public:
    //--------------------------------------------------------------------------------------------------------
    Storage();

public:
    //--------------------------------------------------------------------------------------------------------
    void Initialize();

    void LoadMarketConfig(std::vector<Market> & vMarketsLst);
    void SaveMarketConfig(std::vector<Market> & vMarketsLst);
    void SaveMarketConfig(std::vector<Market> && vMarketsLst);

    enum op_type:int { update = 1, remove = 2 };

    void LoadTickerConfig(std::vector<Ticker> & /*vTickersLst*/) ;
    void SaveTickerConfig(const Ticker & /*tT*/, op_type tp = op_type::update) ;
    void SaveTickerConfig(Ticker && /*tT*/, op_type tp = op_type::update) ;



    //--------------------------------------------------------------------------------------------------------
    // stock quotes storage interface
    //

    bool InitializeTicker(int iTickerID);
    int CreateAndGetFileStageForTicker(int iTickerID, std::time_t tMonth);


private:
    //--------------------------------------------------------------------------------------------------------
    void SaveMarketConfigV_1(std::vector<Market> & vMarketsLst);
    void ParsMarketConfigV_1(std::vector<Market> & vMarketsLst, std::ifstream &file);

    void FormatTickerConfigV_1();
    void SaveTickerConfigV_1(const Ticker & /*tT*/, op_type tp = op_type::update);
    void ParsTickerConfigV_1(std::vector<Ticker> & /*vTickersLst*/, std::ifstream & /*file*/);
    //--------------------------------------------------------------------------------------------------------

    std::time_t dateCastToMonth(std::time_t);

};

//--------------------------------------------------------------------------------------------------------
// construct for parsing with any delimeters

template <char D>
struct StringDelimiter : public std::string
{};

template <char D>
std::istream &
operator>>(std::istream & is, StringDelimiter<D> & output)
{
  std::getline(is, output, D);
  return is;
}

//--------------------------------------------------------------------------------------------------------
// trimming functions
static inline std::string & ltrim(std::string &s){
    s.erase(begin(s),std::find_if(s.begin(),s.end(),[](const char t){
        return !std::isspace(t);
    }));
    return s;
}
//
static inline std::string & ltrim(std::string &&s){ return  ltrim(s);}
//
static inline std::string & rtrim(std::string &s){
    s.erase(std::find_if(s.rbegin(),s.rend(),[](unsigned char ch){
        return !std::isspace(ch);
    }).base(),s.end()
            );
    return s;
}
//
static inline std::string & rtrim(std::string &&s){ return  rtrim(s);}
//
static inline std::string & trim(std::string &s){
    ltrim(s);
    rtrim(s);
    return s;
}
//
static inline std::string & trim(std::string &&s){ return  trim(s);}
//
//static inline std::string ltrim_copy(std::string s) {
//    ltrim(s);
//    return s;
//}

//static inline std::string rtrim_copy(std::string s) {
//    rtrim(s);
//    return s;
//}

//static inline std::string trim_copy(std::string s) {
//    trim(s);
//    return s;
//}
//--------------------------------------------------------------------------------------------------------


#endif // STORAGE_H
