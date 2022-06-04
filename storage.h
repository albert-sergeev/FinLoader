/****************************************************************************
*  This is part of FinLoader
*  Copyright (C) 2021  Albert Sergeyev
*  Contact: albert.s.sergeev@mail.ru
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <https://www.gnu.org/licenses/>.
****************************************************************************/

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
// SS-table file switch stages:
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


///////////////////////////////////////////////////////////////////
/// \brief The Storage class used for followed purposes:
/// 1. main programm path manipulation
/// 2. markets&tickers config files save/load
/// 3. main database SS-file manipulation
/// 4. logsiles saving
class Storage
{
    bool bInitialized;

    std::string sStoragePath;

    std::filesystem::directory_entry drCurr;

    std::filesystem::path pathCurr;
    std::filesystem::path pathDataDir;
    std::filesystem::path pathStorageDir;
    std::filesystem::path pathMarkersFile;
    std::filesystem::path pathTickersFile;
    std::filesystem::path pathTickersSwitcherFile;

    //---------------------------------------
    // for markets/tickers configs
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


public:
    //=======================================================================================================
    /// utility class to manipulate mutexes for writing to SS-file.
    /// idea is to check if mutex from parameter is same as stored,
    /// and if so do nothing, else - locking.
    /// shared or unique lock defined by template
    ///
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
    //=======================================================================================================

public:
    //=======================================================================================================
    // creating and init interface
    Storage();
    ~Storage(){;}

    /// init  must always be called at start. mostly initializes paths, and create absent files
    ///
    void Initialize(std::string sPath);

    /// public enums
    ///
    enum data_type:char { usual = 0, new_sec = 1, del_from = 2 , del_to = 3};

public:
    //=======================================================================================================
    // main programm path public interface. if Initialize(...) had not run has undefined behavor

    inline std::string GetCurrentPath() {return  pathCurr.string();};
    inline std::string GetDataPath()    {return  pathDataDir.string();};
    inline std::string GetStoragePath() {return  pathStorageDir.string();};

public:
    //=======================================================================================================
    // market config interface
    //
    // market config files has usual text-with-delimiter structure, one file for config without any complicated things,
    // so do usual text read/write with parse

    void LoadMarketConfig(std::vector<Market> & vMarketsLst);
    void SaveMarketConfig(std::vector<Market> & vMarketsLst);
    void SaveMarketConfig(std::vector<Market> && vMarketsLst);

private:

    void SaveMarketConfigLocal(std::vector<Market> & vMarketsLst);
    void SaveMarketConfigV_1(std::vector<Market> & vMarketsLst);
    void ParsMarketConfigV_1(std::vector<Market> & vMarketsLst, std::ifstream &file);

    void SaveMarketConfigV_2(std::vector<Market> & vMarketsLst);
    void ParsMarketConfigV_2(std::vector<Market> & vMarketsLst, std::ifstream &file);

public:
    //=======================================================================================================
    // tickers config interface

    enum op_type:int { update = 1, remove = 2 };

    void LoadTickerConfig(std::vector<Ticker> & /*vTickersLst*/) ;
    void SaveTickerConfig(const Ticker & /*tT*/, op_type tp = op_type::update) ;
    void SaveTickerConfig(Ticker && /*tT*/, op_type tp = op_type::update) ;

private:

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

public:
    //=======================================================================================================
    // log files interface

    int SaveToLogfile(const std::string &str,const  std::string & strLogFileName,
                      const int iCurrentLogfileNumb, const size_t iMasLogfileSize, const int iMaxLogfiles);

public:
    //=======================================================================================================
    // stock quotes storage interface

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
                       std::stringstream & ssOut,
                       size_t &iCollisionCount);


    bool OptimizeStore(int iTickerID, std::time_t tMonth, bool & bToPlanNextShift,
                       std::stringstream & ssOut);

    static bool slotParseLine(dataFinQuotesParse & parseDt, std::istringstream & issLine, Bar &b);

    static std::time_t dateCastToMonth(std::time_t);
    static std::time_t dateAddMonth(std::time_t);
    static std::time_t dateAddMonth(std::time_t, int iMonth);

private:

    bool InitializeTickerEntry(int iTickerID,std::stringstream & ssOut);
    int CreateStageEntryForTicker(int iTickerID, std::time_t tMonth,std::stringstream& ssOut);
    int GetStageEntryForTicker(int iTickerID, std::time_t tMonth,std::stringstream& ssOut);
    void CreateDataFilesForEntry(std::string sFileName, std::string sFileNameShort, int iState,std::stringstream& ssOut);


    bool ReadFromStoreFile(int iTickerID, std::time_t tMonth, std::map<std::time_t,std::vector<BarTick>> &mvHolder,
                       std::time_t dtLoadBegin, std::time_t dtLoadEnd,
                       std::string sFileName,
                       bool bFilterRepo,        Market::SessionTable_type  &vRepoTable,
                       bool bFilterSessionTable,Market::SessionTable_type  &vSessionTable,
                       std::stringstream & ssOut,
                       size_t &iCollisionCount);

    bool WriteMapToStore(std::string sFilename, std::map<std::time_t,std::vector<BarTick>>& mvHolder, std::stringstream & ssOut);

    bool SwitchStage(std::pair<int,std::time_t> k, std::string sPath, std::string sFileNamePart, int iNewStage,
                              std::stringstream & ssOut);

    static size_t mapSize(std::map<std::time_t,std::vector<BarTick>> &m);
    //=======================================================================================================

};



//--------------------------------------------------------------------------------------------------------
// constructions for parsing with any delimeters

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
