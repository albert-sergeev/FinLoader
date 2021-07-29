#include "workerloaderfinam.h"

#include<sstream>
#include<chrono>

workerLoaderFinam::workerLoaderFinam()
{

}


void workerLoaderFinam::worker()
{
    CThreadFreeCout fout;
    fout<<"workerLoaderFinam in\n";

    std::string str;
    std::ostringstream ss(str);


    for ( int i=0; i < 10000000;++i){
        ss<<i<<"ddd\n";
    }


    fout<<"workerLoaderFinam out\n";
}
