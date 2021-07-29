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
    std::mutex mut_treads;
    std::queue<FuncWrapper> work_queue;

    std::vector<std::thread> threads;
    std::vector<std::future<int>>  vFuteresTrd;
    std::vector<std::promise<int>> vPromisesTrd;

    std::vector<std::thread> threadsDeleted;
//    std::vector<std::future<int>>  vFuteresTrdDeleted;
//    std::vector<std::promise<int>> vPromisesTrdDeleted;

    join_threads joiner;
    join_threads joinerDeleted;

    //---------------------------------------------------------------------------------------------------
    void worker(std::promise<int> && pr){
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
//        {
//            CThreadFreeCout fout;
//            fout<<"worker pre out\n";
//        }
        try{
            pr.set_value(1);
        }
        catch(std::exception &ex){
            {
            CThreadFreeCout fout;
            fout<<"exeption!\n";
            fout<<ex.what();
            /*
             * terminate called after throwing an instance of 'std::future_error'
  what():  std::future_error: No associated state
*/
            }
            throw;
        }
//        {
//            CThreadFreeCout fout;
//            fout<<"worker out\n";
//        }
    };
    //---------------------------------------------------------------------------------------------------
public:
    inline size_t ActiveThreads()   const {return threads.size();};
    inline size_t TasksToDo()       const {return work_queue.size();};

public:
    //---------------------------------------------------------------------------------------------------


    ThreadPool():bQuit{false},joiner{threads},joinerDeleted{threadsDeleted}{
        threadMaxCount = std::thread::hardware_concurrency() > 0 ? std::thread::hardware_concurrency() : 1;
        try{ ;}
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
    std::future<typename std::result_of<T()>::type> AddTask(T f)
    {

//        {
//            CThreadFreeCout fout;
//            fout<<"shrinking in \n";
//        }
        ShrinkFinishedTreads();
//        {
//            CThreadFreeCout fout;
//            fout<<"shrinking out \n";
//        }

        std::packaged_task<typename std::result_of<T()>::type()> task(std::move(f));
        std::future<typename std::result_of<T()>::type> res(task.get_future());

        {
        std::unique_lock lk(mut_qeue);
        work_queue.push(std::move(task));
        }


        //if(threadMaxCount > (int)threads.size())
        {
            try{
                std::unique_lock lkT(mut_treads);
                std::promise<int> pr;
                vFuteresTrd.emplace_back(pr.get_future());
                vPromisesTrd.emplace_back(std::move(pr));
                int iInd = threads.size();

                threads.emplace_back(std::thread(&ThreadPool::worker,this,std::move(vPromisesTrd[iInd])));

            }
            catch(...){
                // Out of memory?
                if(threads.size() <=0)
                {
                    bQuit = true;
                    std::cout<<"can't create new thread";
                    throw;
                }
            }
        }
        DeleteFinishedTreads();
        return  res;
    }
    //---------------------------------------------------------------------------------------------------
    void ShrinkFinishedTreads()
    {
        std::unique_lock lkT(mut_treads);
        std::future_status stTmp;
        for(int i = 0; i < (int)threads.size(); ++i){
            if(!vFuteresTrd[i].valid() ||
                    (stTmp = vFuteresTrd[i].wait_for(1ms)) == std::future_status::ready
                    ){
//                if(vFuteresTrd[i].valid()){
//                    vFuteresTrd[i].get();
//                }

                threadsDeleted.emplace_back(std::move(threads[i]));
                //vFuteresTrdDeleted.emplace_back(std::move(vFuteresTrd[i]));
                //vPromisesTrdDeleted.emplace_back(std::move(vPromisesTrd[i]));

                std::swap(threads[i],threads.back());
                std::swap(vFuteresTrd[i],vFuteresTrd.back());
                std::swap(vPromisesTrd[i],vPromisesTrd.back());

                threads.pop_back();
                vFuteresTrd.pop_back();
                vPromisesTrd.pop_back();
            }
        }
    }

    //---------------------------------------------------------------------------------------------------
    void DeleteFinishedTreads(){
        std::unique_lock lkT(mut_treads);
        for(int i = 0; i < (int)threadsDeleted.size(); ++i){
            threadsDeleted[i].join();
        }
        threadsDeleted.clear();
        //vPromisesTrdDeleted.clear();
        //vFuteresTrdDeleted.clear();
    }
    //---------------------------------------------------------------------------------------------------
};



#endif // THREADPOOL_H
