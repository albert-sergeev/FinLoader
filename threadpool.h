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

#include "threadfreecout.h"

/////////////////////////////////////////////////////////////////////////////////////////
/// ThreadPool class and its demanded classes
/// Class used to manage multythread tasks. Controls their lifetime, creates new, deletes etc.
/// see below
/////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////
/// \brief The flagInterrupt class. Cover for thread local atomic bool variable used to interrupt process
/// using future/promise mechanics to transmit variable refference to main process so it can set it
///
class flagInterrupt
{
    std::atomic<bool> bInt;
public:
    flagInterrupt(){};
    void set()      {bInt.store(true);};
    bool isSet()    {return  bInt.load();};
};

// place to store interrupt flag
inline thread_local flagInterrupt this_thread_flagInterrup;

/////////////////////////////////////////////////////////////////////////////////////////
// var to count active processes. Processes must increment var on entering and decrement on leaving.
// Usualy using ActiveProcessCounter cover for it

inline std::atomic<int> aActiveProcCounter{0};

/////////////////////////////////////////////////////////////////////////////////////////
/// \brief Cover for safe join for std::vector<std::thread>
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
    };
};

/////////////////////////////////////////////////////////////////////////////////////////
/// \brief Wrapper for std::packaged_task<func> for using futures
/// Idia is to create copy constructor for non copyable class (std::packaged_task<func>)
/// so we through inheritance mechanism  make instance cover for std::packaged_task,
/// and then move the created object by reference to the base class if neaded
/// so we can use this wrapper with copy-demanded containers using move constructors
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
/// \brief The ThreadPool class. Used to manage multythread tasks.
///
/// Task provided through AddTask(std::packaged_task<func> f).
/// Task pushed to work_queue and then if neaded run new process.
/// New task is supplied with two promises: one to get back refference to thread_local interrupt flag
/// and second - for set on exit from task.
/// AddTask return the future to the result of the task, so if the caller saves it, the return value can be obtained.
/// By the way doing checking for finished tasks and freeing threadpool process slots
///
////////////////////////////////////////////////////////////////////////////////////////////
class ThreadPool
{
private:
    int threadMaxCount;                 // max threads limit
    std::atomic_bool bQuit;             // flag to mark work finished
    std::mutex mut_qeue;                // mutex to protect queue whith tasks
    std::mutex mut_treads;              // mutex to protect queue whith threads
    std::queue<FuncWrapper> work_queue; // queue for task

    std::vector<std::thread> threads;               // container for work threads
    std::vector<std::thread> threadsDeleted;        // container for threads  which needs to delete

    std::vector<std::future<bool>>  vFutures;       // cover for futures to mark threads exit               !one dimention with threads
    std::vector<flagInterrupt *>  vInterruptFlags;  // cover for reference to interupt flags of processes   !one dimention with threads

    join_threads<std::thread> joinerDeleted;    // defender for joinig threadsDeleted
    join_threads<std::thread> joiner;           // defender for joinig threads

    //---------------------------------------------------------------------------------------------------
    // base procedure for threads
    void worker(std::promise<bool> pr,std::promise<flagInterrupt *> prIt){

        // transmit refference for interrupt flag
        prIt.set_value(&this_thread_flagInterrup);

        //loop until have tasks or was global exit
        while(!bQuit && !work_queue.empty()){

            FuncWrapper task;
            std::unique_lock lk(mut_qeue);
            if (!work_queue.empty()){
                task = std::move(work_queue.front());
                work_queue.pop();
                lk.unlock();

                // task running
                task();
            }
        }
        // transmit exit event
        pr.set_value(true);
    };
    //---------------------------------------------------------------------------------------------------
public:
    // basic user interface
    inline size_t ActiveThreads()   const {return threads.size();};
    inline size_t TasksToDo()       const {return work_queue.size();};
    inline size_t MaxThreads()      const {return threadMaxCount;}

public:
    //---------------------------------------------------------------------------------------------------
    /// \brief ThreadPool cConstructor
    /// \param MaxThreads cannot be less than 1 or more than std::thread::hardware_concurrency()
    ///
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
    // on destruction set global exit flag and do Interupt for all processes
    ~ThreadPool(){
        bQuit = true;
        Interrupt();
    }
    //---------------------------------------------------------------------------------------------------
    // send interrupt signal to all process through stored refferences on thread_local lags
    void Interrupt(){
        std::unique_lock lkT(mut_treads);
        for(size_t i = 0; i < vInterruptFlags.size(); ++i){
            if (threads[i].joinable()){
                if(vFutures[i].wait_for(std::chrono::microseconds(1)) != std::future_status::ready){
                    vInterruptFlags[i]->set();
                }
            }
        }
    }
    //---------------------------------------------------------------------------------------------------
    // same as destructor, but used by owner of class
    void Halt(){
        bQuit = true;
        Interrupt();
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
    /// Main add process class
    /// <T> must be a func with no parameters, but can return a value
    /// if needed <T> may be lambda covering more complicated func
    ///
    template<typename T>
    std::future<typename std::result_of<T()>::type> AddTask(T f)
    {
        // algorithm
        // 1. check, if there are finished process, to free threadpool slots
        // 2. package task to std::packaged_task and get future to result
        // 3. push task to main queue
        // 4. if there are free slots, run new thread, set all promises to their pools
        // 5. delete finished process if there are some

        //////////////////////////////////////////////////////////////////////
        // 1. check, if there are finished process, to free threadpool slots

        ShrinkFinishedTreads();

        //////////////////////////////////////////////////////////////////////
        // 2. package task to std::packaged_task and get future to result

        std::packaged_task<typename std::result_of<T()>::type()> task(std::move(f));
        std::future<typename std::result_of<T()>::type> res(task.get_future());

        if (bQuit){
            return res;
        }

        //////////////////////////////////////////////////////////////////////
        // 3. push task to main queue

        {
        std::unique_lock lk(mut_qeue);
        work_queue.push(std::move(task));
        }

        //////////////////////////////////////////////////////////////////////
        // 4. if there are free slots, run new thread, set all promises to their pools
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

        //////////////////////////////////////////////////////////////////////
        // 5. delete finished process if there are some
        DeleteFinishedTreads();
        return  res;
    }
    //---------------------------------------------------------------------------------------------------

private:
    //---------------------------------------------------------------------------------------------------
    /// \brief ShrinkFinishedTreads func to free threadpool slot from finished processes
    ///
    void ShrinkFinishedTreads()
    {
        // algorithm
        // 1. lock mutex
        // 2. loop through threads and check if futures for finished event was set
        // 3. if so, move thread to threadsDeleted and free containers
        if (!bQuit)
        {
        /////////////////////////////////////////////////////////////
        // 1. lock mutex

        std::unique_lock lkT(mut_treads);

            /////////////////////////////////////////////////////////////
            // 2. loop through threads and check if futures for finished event was set

            for(int i = 0; i < (int)threads.size(); ++i){
                if(vFutures[i].wait_for(std::chrono::microseconds(1)) == std::future_status::ready){
                //if(threads[i].IsFinished()){

                    /////////////////////////////////////////////////////////////
                    // 3. if so, move thread to threadsDeleted and free containers

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
    }

    //---------------------------------------------------------------------------------------------------
    /// delete finished threads
    ///
    void DeleteFinishedTreads(){
        // lock & join. then clear
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

///////////////////////////////////////////////////////////////////////////////////
/// \brief The ActiveProcessCounter class for safely manipulate aActiveProcCounter
/// processes must create instanses of the class, when they become active and destroy them when stoped
/// used for calculate active processes for display on GUI
///
class ActiveProcessCounter
{
public:
    ActiveProcessCounter(){
        int iCount = aActiveProcCounter.load();
        while(!aActiveProcCounter.compare_exchange_weak(iCount,iCount + 1)){;}
    }
    ~ActiveProcessCounter(){
        int iCount = aActiveProcCounter.load();
        while(!aActiveProcCounter.compare_exchange_weak(iCount,iCount - 1)){;}
    }
};



#endif // THREADPOOL_H
