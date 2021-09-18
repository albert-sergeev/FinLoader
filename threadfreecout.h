#ifndef THREADFREECOUT_H
#define THREADFREECOUT_H

#include<iostream>
#include<sstream>
#include<mutex>

class ThreadFreeCout:public std::stringstream
{
    static inline std::mutex mut;
public:
    ThreadFreeCout(){;};
    ~ThreadFreeCout(){
        std::lock_guard<std::mutex> lk(mut);
        if (this->peek() != ThreadFreeCout::traits_type::eof()){
            std::cout<<this->rdbuf();
            std::cout.flush();
        }
    }
};

#endif // THREADFREECOUT_H
