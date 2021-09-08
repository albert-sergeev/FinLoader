#include "amipipeholder.h"

#include<filesystem>
#include<iostream>
#include<regex>
#include<iomanip>
#include<fstream>
#include<ostream>
#include<chrono>
#include<sstream>
#include "trimutils.h"

//-------------------------------------------------------------------------------------------------
AmiPipeHolder::AmiPipeHolder()
{

}

//-------------------------------------------------------------------------------------------------
AmiPipeHolder::pipes_type AmiPipeHolder::ScanActivePipes()
{
    AmiPipeHolder::pipes_type mRet;

    std::string sPipeDir = "\\\\.\\pipe\\";
    std::stringstream ssFilePath("");


    std::filesystem::path pathTickerDir = std::filesystem::absolute(sPipeDir);
//    if(!std::filesystem::is_directory(pathTickerDir)){
//        ssOut <<" ./data/[TickerID] - is not directory";
//        return false;
//    }
    //////////////////
    std::stringstream ssReg;
    std::stringstream ssRegSign;

    //AmiBroker2QUIK_TQBR.SBER_TICKS
    ssReg <<"^AmiBroker2QUIK_(.*)_TICKS$";
    const std::regex reAmiPipe {ssReg.str()};

    ssRegSign <<"(?:(?![.]).)+$";
    const std::regex reAmiSign {ssRegSign.str()};

    std::vector<std::filesystem::directory_entry> vPipes;

    std::string sSign;
    std::string sBind;

    for (const std::filesystem::directory_entry &fl:std::filesystem::directory_iterator{pathTickerDir}){

        if ( fl.exists() && fl.is_regular_file()){
            std::string ss(fl.path().filename().string());
            const auto ItPipe = std::sregex_token_iterator(ss.begin(),ss.end(),reAmiPipe);
            if ( ItPipe != std::sregex_token_iterator()){
                const auto ItQuik = std::sregex_token_iterator(ss.begin(),ss.end(),reAmiPipe,1);
                sBind = *ItQuik;
                if (ItQuik != std::sregex_token_iterator()){
                    const auto ItSign = std::sregex_token_iterator(sBind.begin(),sBind.end(),reAmiSign);
                    sSign = "";
                    if (ItSign != std::sregex_token_iterator()){
                        sSign = *ItSign;
                    }
                    ssFilePath.str("");
                    ssFilePath.clear();
                    ssFilePath <<sPipeDir<<(*ItPipe);
                    mRet[sBind] = {0,{*ItPipe,sSign,ssFilePath.str(),0,0}};
                }
            }
        }
    }
    return mRet;
}

//-------------------------------------------------------------------------------------------------
void AmiPipeHolder::CheckPipes(std::vector<Ticker> &vT,
                               AmiPipeHolder::pipes_type & mBindedPipesActive,
                               AmiPipeHolder::pipes_type & mBindedPipesOff,
                               AmiPipeHolder::pipes_type &mFreePipes,
                               std::vector<int> &mUnconnectedPipes
                               )
{
    mBindedPipesActive.clear();
    mBindedPipesOff.clear();
    mFreePipes.clear();
    //
    AmiPipeHolder::pipes_type mPipes = AmiPipeHolder::ScanActivePipes();
    //
    std::string sTmp;
    for(const auto &t:vT){
        sTmp = trim(t.TickerSignQuik());
        if(sTmp.size() > 0 && mPipes.find(sTmp) != mPipes.end()){
            if (t.AutoLoad()){
                mPipes[sTmp].second = {std::get<0>(mPipes[sTmp].second),
                                       std::get<1>(mPipes[sTmp].second),
                                       std::get<2>(mPipes[sTmp].second),
                                       t.TickerID(),
                                       1};
            }
            else{
                mPipes[sTmp].second = {std::get<0>(mPipes[sTmp].second),
                                       std::get<1>(mPipes[sTmp].second),
                                       std::get<2>(mPipes[sTmp].second),
                                       t.TickerID(),
                                       2};
            }

        }
        else{
            if (t.AutoLoad()){
                mUnconnectedPipes.push_back(t.TickerID());
            }
        }
    }
    ///
    for(const auto &p:mPipes){
        if (std::get<4>(p.second.second) == 1){
            mBindedPipesActive[p.first] = p.second;
        }
        else if (std::get<4>(p.second.second) == 2){
            mBindedPipesOff[p.first] = p.second;
        }
        else{
            mFreePipes[p.first] = p.second;
        }
    }
}
//-------------------------------------------------------------------------------------------------
void AmiPipeHolder::RefreshActiveSockets(pipes_type& pActive,
                          pipes_type& pOff,
                          BlockFreeQueue<dataAmiPipeAnswer>&queuePipeAnswers)
{
    for (auto &m:mPipesHalted){m.second.first =0;}
    for (auto &m:mPipesConnected){m.second.first =0;}
    //
    for(const auto & p:pActive){
        auto ItHalted (mPipesHalted.find(p.first));
        if ( ItHalted == mPipesHalted.end()){
            auto ItConnected (mPipesConnected.find(p.first));
            if ( ItConnected == mPipesConnected.end()){

                bool bOpend{false};
                try{
                    std::ifstream file{std::get<2>(p.second.second)};
                    if (file.good()){
                        mPipesConnected[p.first].first  = 1;
                        mPipesConnected[p.first].second = {std::get<3>(p.second.second),
                                                            std::move(file)};
                        //
                        dataAmiPipeAnswer answ;
                        answ.Type = dataAmiPipeAnswer::PipeConnected;
                        answ.iTickerID = std::get<3>(p.second.second);
                        queuePipeAnswers.Push(answ);
                        bOpend = true;
                    }
                }
                catch(std::exception &){
                    ;
                }
                if (!bOpend){
                    mPipesHalted[p.first].first  = 1;
                    mPipesHalted[p.first].second = {std::get<3>(p.second.second),
                                                    std::ifstream{}
                                                   };
                    //
                    dataAmiPipeAnswer answ;
                    answ.Type = dataAmiPipeAnswer::PipeHalted;
                    answ.iTickerID = std::get<3>(p.second.second);
                    queuePipeAnswers.Push(answ);
                }
            }
            else{
                ItConnected->second.first = 1;
            }
        }
        else{
            ItHalted->second.first = 1;
        }
    }
    //////////////////////////////////////////////////////////
    auto ItConnected = mPipesConnected.begin();
    while (ItConnected != mPipesConnected.end()){
        if (ItConnected->second.first == 0){
            /// TODO: disconnect;
            ItConnected->second.second.second.close();
            ///
            dataAmiPipeAnswer answ;
            if(pOff.find(ItConnected->first)!=pOff.end())    answ.Type = dataAmiPipeAnswer::PipeOff;
            else                                            answ.Type = dataAmiPipeAnswer::PipeDisconnected;

            answ.iTickerID = ItConnected->second.second.first;
            queuePipeAnswers.Push(answ);
            //
            auto ItNext = std::next(ItConnected);
            mPipesConnected.erase(ItConnected);
            ItConnected = ItNext;
        }
        else{
            ItConnected++;
        }
    }
    //
    auto ItHulted = mPipesHalted.begin();
    while (ItHulted != mPipesHalted.end()){
        if (ItHulted->second.first == 0){

            auto ItNext = std::next(ItHulted);
            mPipesHalted.erase(ItHulted);
            ItHulted = ItNext;
        }
        else{
            ItHulted++;
        }
    }
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------


