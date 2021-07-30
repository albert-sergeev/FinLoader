#include "workerloaderfinam.h"
#include "threadfreecout.h"
#include "threadinterruptible.h"


#include<sstream>
#include<chrono>



workerLoaderFinam::workerLoaderFinam()
{

}


void workerLoaderFinam::worker()
{
    ThreadFreeCout fout;
    fout<<"workerLoaderFinam in\n";

    std::string str;
    std::ostringstream ss(str);


    for ( int i=0; i < 50000000;++i){
        if(this_thread_flagInterrup.isSet())
        {
            fout<<"exit on interrupt\n";
            break;
        }
        ss<<i<<"ddd\n";
    }

    //unsigned long t = (unsigned long)&this_thread_flagInterrup;
    //fout<<"got in thread ["<<t<<"]\n";
    fout<<"workerLoaderFinam out\n";
}


