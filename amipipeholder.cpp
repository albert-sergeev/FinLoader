#include "amipipeholder.h"

#include<filesystem>
#include<iostream>
#include<regex>
#include<iomanip>
#include<fstream>
#include<ostream>
#include<chrono>
#include<sstream>


AmiPipeHolder::AmiPipeHolder()
{

}


std::vector<std::string> AmiPipeHolder::CheckActivePipes()
{

    std::string sPipeDir = "\\\\.\\pipe\\";

    std::filesystem::path pathTickerDir = std::filesystem::absolute(sPipeDir);
//    if(!std::filesystem::is_directory(pathTickerDir)){
//        ssOut <<" ./data/[TickerID] - is not directory";
//        return false;
//    }
    //////////////////
    std::stringstream ssReg;
    //AmiBroker2QUIK_TQBR.SBER_TICKS
    ssReg <<"^AmiBroker2QUIK_(.*)_TICKS$";
    const std::regex reAmiPipe {ssReg.str()};
    std::vector<std::filesystem::directory_entry> vPipes;

    std::copy_if(std::filesystem::directory_iterator{pathTickerDir},{},std::back_inserter(vPipes),[&reAmiPipe](const std::filesystem::directory_entry &c){
                     if ( c.exists() /*&& c.is_regular_file()*/){
                         std::string ss(c.path().filename().string());
                         if (std::sregex_token_iterator(ss.begin(),ss.end(),reAmiPipe) != std::sregex_token_iterator())
                             return  true;
                         else
                            return false;
                     }
                     return false;
                 });
    vActivePipes.clear();

    std::vector<std::string> vRet;
    for (const auto &e:vPipes){
        vActivePipes.push_back(e);
        vRet.push_back(e.path().filename().string());

    }

    return vRet;
}

