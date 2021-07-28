#ifndef THREADPOOL_H
#define THREADPOOL_H

#include<iostream>
#include<sstream>
#include<thread>
#include<atomic>
#include<mutex>
#include<future>
#include<functional>
#include<vector>
#include<queue>

/////////////////////////////////////////////////////////////////////////////////////////
/// \brief Cover for safe join std::vector<std::thread>
///
class join_threads{
    std::vector<std::thread> &threads;
public:
    join_threads() = delete;
    join_threads(std::vector<std::thread> & thr):threads{thr}{;};
    ~join_threads(){
        for(int i = 0 ; i < (int)threads.size(); ++i){
            threads[i].join();
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
    //std::queue<std::function<void()>> work_queue;
    std::queue<FuncWrapper> work_queue;
    std::vector<std::thread> threads;
    //std::vector<std::thread> threads;
    join_threads joiner;

    //---------------------------------------------------------------------------------------------------
    void worker(){
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
    };
    //---------------------------------------------------------------------------------------------------
public:
    inline size_t ActiveThreads()   const {return threads.size();};
    inline size_t TasksToDo()       const {return work_queue.size();};

public:
    //---------------------------------------------------------------------------------------------------
    ThreadPool():bQuit{false},joiner{threads}{
        threadMaxCount = std::thread::hardware_concurrency() > 0 ? std::thread::hardware_concurrency() : 1;
        try{ // to check, create thread with empty task queue
            threads.push_back(std::thread(&ThreadPool::worker,this));
        }
        catch(...){
            bQuit = true;
            throw;
        }
    };
    //---------------------------------------------------------------------------------------------------
    ~ThreadPool(){
        bQuit = true;
    }
    //---------------------------------------------------------------------------------------------------
    template<typename T>
    void AddTask(T f){
        {
            std::packaged_task<typename std::result_of<T()>::type()> task(std::move(f));
            std::future<typename std::result_of<T()>::type> res(task.get_future());

            std::unique_lock lk(mut_qeue);
            work_queue.push(std::move(task));
            //work_queue.push(std::function<void()>(f));
        }
        if(threadMaxCount > (int)threads.size()){
            try{
                threads.push_back(std::thread(&ThreadPool::worker,this));
            }
            catch(...){
                // Out of memory?
                if(threads.size() <=0){
                bQuit = true;
                throw;
                }
            }
        }
    }
    //---------------------------------------------------------------------------------------------------
    //---------------------------------------------------------------------------------------------------
    //---------------------------------------------------------------------------------------------------
};

/////////////////////////////////////////////////////////////////////////////////////////
/// \brief Utility class to threadsafe cout (for debug)
///
class CThreadFreeCout:public std::stringstream
{
    static inline std::mutex mut;
public:
    CThreadFreeCout(){;};

    ~CThreadFreeCout(){
        std::lock_guard<std::mutex> lk(mut);
        std::cout<<this->rdbuf();
        std::cout.flush();
    }
};

#endif // THREADPOOL_H
