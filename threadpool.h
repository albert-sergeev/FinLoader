#ifndef THREADPOOL_H
#define THREADPOOL_H

#include<iostream>
#include<sstream>
#include<thread>
#include<atomic>
#include<algorithm>
#include<mutex>
#include<future>
#include<functional>
#include<vector>
#include<queue>
#include<chrono>

using namespace std::chrono_literals;

#include "threadfreecout.h"

/////////////////////////////////////////////////////////////////////////////////////////
/// \brief The flagInterrupt class
///
class flagInterrupt
{
    std::atomic<bool> bInt;
public:
    flagInterrupt(){};
    void set()      {bInt.store(true);};
    bool isSet()    {return  bInt.load();};
};

inline thread_local flagInterrupt this_thread_flagInterrup;
/////////////////////////////////////////////////////////////////////////////////////////
//inline thread_local int WorkerThreadID;
//inline std::atomic<int> WorkerThreadCounter{1};

/////////////////////////////////////////////////////////////////////////////////////////
/// \brief Cover for safe join std::vector<std::thread>
///
template <typename ThreadType>
class join_threads{
    std::vector<ThreadType> &threads;
public:
    join_threads() = delete;
    join_threads(std::vector<ThreadType> & thr):threads{thr}{;};
    ~join_threads(){
        for(int i = 0 ; i < (int)threads.size(); ++i){
            if(threads[i].joinable())
                threads[i].join();
        }
        {
            ThreadFreeCout pcout;
            pcout <<"~join_threads() {"<<(&threads)<<"}\n";
        }
    };
};

/////////////////////////////////////////////////////////////////////////////////////////
/// \brief Wrapper for std::packaged_task<func> for using futures
///
class FuncWrapper{
    struct impl_base{
        virtual void call()=0;
        virtual ~impl_base(){;};
    };
    template<typename F>
    struct impl_type:impl_base{
        F ff;
        impl_type(F &&f):ff{std::move(f)}{;}
        virtual void call(){ff();};
    };
    std::unique_ptr<impl_base> impl;
public:

    template<typename F>
    FuncWrapper(F &&f): impl(new impl_type<F>(std::move(f))){};
    void operator()() {impl->call();};

    FuncWrapper() = default;
    FuncWrapper(FuncWrapper&& o):impl(std::move(o.impl)){};
    FuncWrapper& operator=(FuncWrapper&& o){impl = (std::move(o.impl)); return *this;};

    FuncWrapper(FuncWrapper& o) = delete;
    FuncWrapper(const FuncWrapper& o) = delete;
    FuncWrapper& operator=(const FuncWrapper& o) = delete;
};


/////////////////////////////////////////////////////////////////////////////////////////
/// \brief The ThreadPool class
////////////////////////////////////////////////////////////////////////////////////////////

class ThreadPool
{
private:
    int threadMaxCount;
    std::atomic_bool bQuit;
    std::mutex mut_qeue;
    std::mutex mut_treads;
    std::queue<FuncWrapper> work_queue;

    std::vector<std::thread> threads;
    std::vector<std::thread> threadsDeleted;



    std::vector<std::future<bool>>  vFutures;
    std::vector<flagInterrupt *>  vInterruptFlags;

    join_threads<std::thread> joinerDeleted;
    join_threads<std::thread> joiner;


    //---------------------------------------------------------------------------------------------------
    void worker(std::promise<bool> pr,std::promise<flagInterrupt *> prIt){

        prIt.set_value(&this_thread_flagInterrup);

        while(!bQuit && !work_queue.empty()){

            FuncWrapper task;
            std::unique_lock lk(mut_qeue);
            if (!work_queue.empty()){
                task = std::move(work_queue.front());
                work_queue.pop();
                lk.unlock();

                task();
            }
        }
        pr.set_value(true);
    };
    //---------------------------------------------------------------------------------------------------
public:
    inline size_t ActiveThreads()   const {return threads.size();};
    inline size_t TasksToDo()       const {return work_queue.size();};
    inline size_t MaxThreads()      const {return threadMaxCount;}

public:
    //---------------------------------------------------------------------------------------------------


    ThreadPool(int MaxThreads = std::thread::hardware_concurrency())
        :bQuit{false},joinerDeleted{threadsDeleted},joiner{threads}{

        MaxThreads = MaxThreads > (int)std::thread::hardware_concurrency() ?  std::thread::hardware_concurrency() : MaxThreads;
        threadMaxCount = MaxThreads > 0 ? MaxThreads : 1;

        //std::cout <<"Max threads: "<<threadMaxCount<<"\n";
        try{
//            Interruptible_thread rhrd ([]{std::cout<<"here we are!\n";});
//            Interruptible_thread rhrd1 (&ThreadPool::worker,this);
//            rhrd.join();
//            rhrd1.join();
            ;}
        catch(...){
            bQuit = true;
            throw;
        }
    };
    //---------------------------------------------------------------------------------------------------
    ~ThreadPool(){
        bQuit = true;
        Interrupt();
        {
            ThreadFreeCout pcout;
            pcout <<"~ThreadPool()\n";
        }
    }
    //---------------------------------------------------------------------------------------------------
    void Interrupt(){
        std::unique_lock lkT(mut_treads);
        for(int i = 0; i < vInterruptFlags.size(); ++i){
            if (threads[i].joinable()){
                vInterruptFlags[i]->set();
            }
        }
        vInterruptFlags.clear();
    }
    //---------------------------------------------------------------------------------------------------
    void Halt(){
        bQuit = true;
        Interrupt();
        {
            ThreadFreeCout pcout;
            pcout <<"ThreadPool::Halt()\n";
        }
        for(int i = 0 ; i < (int)threads.size(); ++i){
            if(threads[i].joinable())
                threads[i].join();
        }
        for(int i = 0 ; i < (int)threadsDeleted.size(); ++i){
            if(threadsDeleted[i].joinable())
                threadsDeleted[i].join();
        }
    }
    //---------------------------------------------------------------------------------------------------
    template<typename T>
    std::future<typename std::result_of<T()>::type> AddTask(T f)
    {


        ShrinkFinishedTreads();

        std::packaged_task<typename std::result_of<T()>::type()> task(std::move(f));
        std::future<typename std::result_of<T()>::type> res(task.get_future());

        if (bQuit){
            return res;
        }

        {
        std::unique_lock lk(mut_qeue);
        work_queue.push(std::move(task));
        }

        if(threadMaxCount > (int)threads.size())
        {
            try{
                std::unique_lock lkT(mut_treads);

                std::promise<bool> pr;
                vFutures.push_back(pr.get_future());

                std::promise<flagInterrupt *> pIt;
                std::future<flagInterrupt *> f = pIt.get_future();

                threads.emplace_back(std::thread(&ThreadPool::worker,this,std::move(pr),std::move(pIt)));

                vInterruptFlags.push_back(f.get());

            }
            catch(...){
                // Out of memory?
                if(threads.size() <=0)
                {
                    bQuit = true;
                    throw std::runtime_error("can't create new thread");
                }
            }
        }
        DeleteFinishedTreads();
        return  res;
    }
    //---------------------------------------------------------------------------------------------------

private:
    //---------------------------------------------------------------------------------------------------
    void ShrinkFinishedTreads()
    {
        if (!bQuit)
        {
        std::unique_lock lkT(mut_treads);

            for(int i = 0; i < (int)threads.size(); ++i){
                if(vFutures[i].wait_for(std::chrono::microseconds(1)) == std::future_status::ready){
                //if(threads[i].IsFinished()){

                    threadsDeleted.emplace_back(std::move(threads[i]));

                    std::swap(threads[i],threads.back());
                    std::swap(vFutures[i],vFutures.back());
                    //std::swap(vPromises[i],vPromises.back());
                    std::swap(vInterruptFlags[i],vInterruptFlags.back());

                    threads.pop_back();
                    vFutures.pop_back();
                    //vPromises.pop_back();
                    vInterruptFlags.pop_back();
                }
            }
        }
//        {
//            ThreadFreeCout fcout;
//            fcout<<"assigned threads: "<<threads.size()<<"\n";
//        }
    }

    //---------------------------------------------------------------------------------------------------
    void DeleteFinishedTreads(){
        std::unique_lock lkT(mut_treads);
        for(int i = 0; i < (int)threadsDeleted.size(); ++i){
            if (threadsDeleted[i].joinable()){
                threadsDeleted[i].join();
            }
        }
        threadsDeleted.clear();
    }
    //---------------------------------------------------------------------------------------------------
};



#endif // THREADPOOL_H
