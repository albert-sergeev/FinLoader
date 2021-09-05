#ifndef STORAGE_H
#define STORAGE_H

#include<mutex>
#include<shared_mutex>
#include<filesystem>

#include "ticker.h"
#include "bar.h"
#include "bartick.h"
#include "graph.h"
#include "threadfreecout.h"
#include "datafinquotesparse.h"


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
/// main storage work class
/// 2 for 2
class Storage
{
    bool bInitialized;

    std::string sStoragePath;

    std::filesystem::directory_entry drCurr;

    std::filesystem::path pathCurr;
    std::filesystem::path pathDataDir;
    std::filesystem::path pathStorageDir;
    std::filesystem::path pathMarkersFile;
    //std::filesystem::path pathMarkersSwitcherFile;
    std::filesystem::path pathTickersFile;
    std::filesystem::path pathTickersSwitcherFile;

    //---------------------------------------
    // for tickers configs
    int iTickerMark{1};
    std::shared_mutex mutexTickerConfigFile;
    std::shared_mutex mutexMarketConfigFile;
    //---------------------------------------
    // for market quotes storage
    std::shared_mutex mutexQuotesStoreInit;
    std::vector<int> vInitializedTickers;
    std::map<std::pair<int,std::time_t>,std::shared_mutex> mpStoreMutexes;
    std::map<std::pair<int,std::time_t>,std::shared_mutex> mpWriteMutexes;
    std::map<std::pair<int,std::time_t>,std::shared_mutex> mpOptimizeMutexes;
    std::map<std::pair<int,std::time_t>,int> mpStoreStages;

    const std::vector<int> vStorageW;
    const std::vector<int> vStorageL1;
    const std::vector<int> vStorageL2;
    const std::vector<int> vStorageS;

    const std::vector<bool> vStorage1;
    const std::vector<bool> vStorage2;
    const std::vector<bool> vStorage3;
    const std::vector<bool> vStorage4;

    //std::atomic<int> aIntCount{0};

public:
    //-----------------------------------------------------------
    template<typename T>
    class MutexDefender{
        //std::unique_lock<std::shared_mutex> *lk;
        T *lk;

    public:
        MutexDefender():lk{nullptr}{}
        MutexDefender(MutexDefender&) = delete;
        MutexDefender & operator=(MutexDefender &o){
            lk = std::move(o.lk);
            return *this;
        }

        void Lock(std::shared_mutex &m){
            if( lk != nullptr && lk->mutex() == &m)     {return;}
            if( lk != nullptr && lk->mutex() != &m)     {delete lk;}
            //lk = new std::unique_lock<std::shared_mutex>(m);
            lk = new T(m);
        };
        void Unlock(){
            if (lk !=nullptr){
                delete lk;
                lk = nullptr;
            }
        }
        ~MutexDefender(){
            if (lk !=nullptr){
                delete lk;
            }
        }
    };
    //-----------------------------------------------------------

public:

    inline std::string GetCurrentPath() {return  pathCurr.string();};
    inline std::string GetDataPath()    {return  pathDataDir.string();};
    inline std::string GetStoragePath() {return  pathStorageDir.string();};


    enum data_type:char { usual = 0, new_sec = 1, del_from = 2 , del_to = 3};


public:
    //--------------------------------------------------------------------------------------------------------
    Storage();
    ~Storage(){
        {
            ThreadFreeCout pcout;
            pcout <<"~Storage()\n";
        }
    }

public:
    //--------------------------------------------------------------------------------------------------------
    void Initialize(std::string sPath);

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

    bool InitializeTicker(int iTickerID,std::stringstream & ssOut, bool bCheckOnly = false);
    int CreateAndGetFileStageForTicker(int iTickerID, std::time_t tMonth, std::stringstream & ssOut);
    bool WriteBarToStore(int iTickerID, Bar &b, std::stringstream & ssOut);
    bool WriteMemblockToStore(MutexDefender<std::shared_lock<std::shared_mutex>> &defSk,
                              MutexDefender<std::unique_lock<std::shared_mutex>> &defUk,
                              int iTickerID, std::time_t tMonth, char* cBuff,size_t length, std::stringstream & ssOut);



    bool ReadFromStore(int iTickerID, std::time_t tMonth, std::vector<BarTick> & vBarList,
                       std::time_t dtLoadBegin, std::time_t dtLoadEnd,
                       bool bFilterRepo,        Market::SessionTable_type  &vRepoTable,
                       bool bFilterSessionTable,Market::SessionTable_type  &vSessionTable,
                       std::stringstream & ssOut);


    bool OptimizeStore(int iTickerID, std::time_t tMonth, bool & bToPlanNextShift,
                       std::stringstream & ssOut);

    static bool slotParseLine(dataFinQuotesParse & parseDt, std::istringstream & issLine, Bar &b);

    static std::time_t dateCastToMonth(std::time_t);
    static std::time_t dateAddMonth(std::time_t);
    static std::time_t dateAddMonth(std::time_t, int iMonth);

private:
    //--------------------------------------------------------------------------------------------------------
    void SaveMarketConfigLocal(std::vector<Market> & vMarketsLst);
    void SaveMarketConfigV_1(std::vector<Market> & vMarketsLst);
    void ParsMarketConfigV_1(std::vector<Market> & vMarketsLst, std::ifstream &file);

    void SaveTickerConfigLocal(const Ticker & /*tT*/, op_type tp) ;
    void FormatTickerConfigV_1();
    void SaveTickerConfigV_1(std::filesystem::path  /*pathFile*/,const Ticker & /*tT*/, op_type tp = op_type::update, int iForceMark = 0);
    int ParsTickerConfigV_1(std::vector<Ticker> & /*vTickersLst*/, std::ifstream & /*file*/);

    void FormatTickerConfigV_2();
    void SaveTickerConfigV_2(std::filesystem::path  /*pathFile*/,const Ticker & /*tT*/, op_type tp = op_type::update, int iForceMark = 0);
    int ParsTickerConfigV_2(std::vector<Ticker> & /*vTickersLst*/, std::ifstream & /*file*/);

    void SwitchTickersConfigFile();
    std::filesystem::path ReadTickersConfigFileName(bool bOld);
    void CompressTickerConfigFile(std::vector<Ticker> & /*vTickersLst*/);
    //--------------------------------------------------------------------------------------------------------
    bool InitializeTickerEntry(int iTickerID,std::stringstream & ssOut);
    int CreateStageEntryForTicker(int iTickerID, std::time_t tMonth,std::stringstream& ssOut);
    int GetStageEntryForTicker(int iTickerID, std::time_t tMonth,std::stringstream& ssOut);
    void CreateDataFilesForEntry(std::string sFileName, std::string sFileNameShort, int iState,std::stringstream& ssOut);


    bool ReadFromStoreFile(int iTickerID, std::time_t tMonth, std::map<std::time_t,std::vector<BarTick>> &mvHolder,
                       std::time_t dtLoadBegin, std::time_t dtLoadEnd,
                       std::string sFileName,
                       bool bFilterRepo,        Market::SessionTable_type  &vRepoTable,
                       bool bFilterSessionTable,Market::SessionTable_type  &vSessionTable,
                       std::stringstream & ssOut);

    bool WriteMapToStore(std::string sFilename, std::map<std::time_t,std::vector<BarTick>>& mvHolder, std::stringstream & ssOut);

    bool SwitchStage(std::pair<int,std::time_t> k, std::string sPath, std::string sFileNamePart, int iNewStage,
                              std::stringstream & ssOut);

    static size_t mapSize(std::map<std::time_t,std::vector<BarTick>> &m);

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


#endif // STORAGE_H
