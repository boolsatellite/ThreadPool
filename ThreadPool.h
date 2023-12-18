//
// Created by satellite on 2023-12-07.
//

#ifndef THREADPOOL_THREADPOOL_H
#define THREADPOOL_THREADPOOL_H

#include <vector>
#include <queue>
#include <memory>
#include <atomic>
#include <mutex>
#include <unordered_map>
#include <condition_variable>

#include "Thread.h"
#include "Any.h"
#include "Result.h"

enum class PoolMod {
    MODE_FIXED ,    //固定数量线程
    MODE_CACHED     //线程数量可动态伸缩
};

class Result;

//任务抽象基类
class Task {
public:
    Task();

    virtual Any run() = 0;                                   //用户自定义任务类型，从Task继承，重写run方法
    void exec();

    void setResult(Result* res);

private:
    Result* result_ ;
};

//线程抽象类



class ThreadPool {
public:
    ThreadPool();
    ~ThreadPool();

    void start(int initThreadSize = 4);
    //生产任务
    Result submitTask(const std::shared_ptr<Task>& task);        //使用智能指针保存所管理的Task对象不被释放

    void setMode(PoolMod mode);
    void setTaskQueMax(int threshhold);
    void setThreadMax(int max);

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

private:
    //消费任务
    void threadFunc(int threadId);

    bool checkRunningState() const;

    //std::vector<std::unique_ptr<Thread>> threads_;
    std::unordered_map<int , std::unique_ptr<Thread>> threads_;      //key => threadId   value => Thread对象
    std::queue<std::shared_ptr<Task>> tasksQue_;
    //std::queue<Task*> tasks_; 不可以使用裸指针，当用户传入一个临时的Task对象时，该指针不能保证对象没有析构,故用强智能指针

    std::mutex  taskQueMtx_;                //用于保证任务队列的线程安全
    std::condition_variable notFull_;       //任务队列不满
    std::condition_variable notEmpty_;      //任务队列不空
    std::condition_variable exitCond_;      //等待线程池中线程全部结束

    size_t initThreadSize_;
    std::atomic_uint taskSize_;     //当前队列中任务的数量
    size_t taskQueMax_;             //任务数量的上限
    PoolMod mode_;                  //当前线程池的工作模式
    std::atomic_bool isPoolrunning; //线程池是否运行
    std::atomic_int idleThreadSize_;//空闲线程的数量
    int threadMax_;                 //线程数量的上限
    std::atomic_int curThreadSize_; //当前线程池的线程总数

};

#endif //THREADPOOL_THREADPOOL_H
