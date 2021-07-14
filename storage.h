#ifndef STORAGE_H
#define STORAGE_H

#include<filesystem>
#include "ticker.h"



class Storage
{
    bool bInitialized;

    std::filesystem::directory_entry drCurr;

    std::filesystem::path pathCurr;
    std::filesystem::path pathDataDir;
    std::filesystem::path pathStorageDir;
    std::filesystem::path pathMarkersFile;
    std::filesystem::path pathTickersFile;

    int iTickerMark{0};

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

    void LoadTickerConfig(std::vector<Ticker> & /*vTickersLst*/) ;
    void SaveTickerConfig(std::vector<Ticker> & /*vTickersLst*/, int iLeft, int iRight) ;
    void SaveTickerConfig(std::vector<Ticker> && /*vTickersLst*/, int iLeft, int iRight) ;

private:
    //--------------------------------------------------------------------------------------------------------
    void SaveMarketConfigV_1(std::vector<Market> & vMarketsLst);
    void ParsMarketConfigV_1(std::vector<Market> & vMarketsLst, std::ifstream &file);

    void SaveTickerConfigV_1(std::vector<Ticker> & /*vTickersLst*/, int iLeft, int iRight);
    void ParsTickerConfigV_1(std::vector<Ticker> & /*vTickersLst*/, std::ifstream & /*file*/);

};


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

#endif // STORAGE_H
