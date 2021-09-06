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
                    mRet[sBind] = {0,{*ItPipe,sSign,fl}};
                }
            }
        }
    }
    return mRet;
}

//-------------------------------------------------------------------------------------------------
void AmiPipeHolder::CheckPipes(std::vector<Ticker> &vT,AmiPipeHolder::pipes_type & mBindedPipes, AmiPipeHolder::pipes_type &mFreePipes)
{
    mBindedPipes.clear();
    mFreePipes.clear();
    //
    AmiPipeHolder::pipes_type mPipes = AmiPipeHolder::ScanActivePipes();
    //
    std::string sTmp;
    for(const auto &t:vT){
        sTmp = trim(t.TickerSignQuik());
        if(sTmp.size() > 0 && mPipes.find(sTmp) != mPipes.end()){
            mPipes[sTmp].first = 1;
        }
    }
    ///
    for(const auto &p:mPipes){
        if (p.second.first != 0){
            mBindedPipes[p.first] = p.second;
        }
        else{
            mFreePipes[p.first] = p.second;
        }
    }
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------


