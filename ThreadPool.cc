//
// Created by satellite on 2023-12-07.
//

#include "ThreadPool.h"
#include "Thread.h"
#include "Result.h"
#include <iostream>
#include <thread>

const int TASK_QUE_MAX = INT32_MAX;
const int THREAD_MAX = 30;
const int THREAD_MAX_IDLE_TIME = 10;


ThreadPool::ThreadPool()
        : initThreadSize_(4), taskSize_(0), taskQueMax_(TASK_QUE_MAX), mode_(PoolMod::MODE_FIXED), isPoolrunning(false),
          idleThreadSize_(0), threadMax_(THREAD_MAX), curThreadSize_(0) {
}

ThreadPool::~ThreadPool() {
    isPoolrunning = false;
    std::unique_lock<std::mutex> lock(taskQueMtx_);
    notEmpty_.notify_all();
    exitCond_.wait(lock , [&]()->auto {return threads_.size() == 0;});
}

void ThreadPool::setMode(PoolMod mode) {
    if (checkRunningState())
        return;
    mode_ = mode;
}

void ThreadPool::setTaskQueMax(int threshhold) {
    if (checkRunningState()) return;
    taskQueMax_ = threshhold;
}

void ThreadPool::setThreadMax(int max) {
    if (checkRunningState()) return;
    if (mode_ == PoolMod::MODE_CACHED) {
        threadMax_ = max;
    }
}

void ThreadPool::start(int initThreadSize) {
    initThreadSize_ = initThreadSize;
    curThreadSize_ = initThreadSize;
    isPoolrunning = true;
    //创建线程对象
    for (int i = 0; i < initThreadSize_; ++i) {
        //threads_.emplace_back(std::make_unique<Thread>([this] { threadFunc(); }));    //lambda 表达式是纯右值表达式
        auto ptr = std::make_unique<Thread>([this](auto && PH1) { threadFunc(std::forward<decltype(PH1)>(PH1)); });
        threads_.emplace(ptr->getId() , std::move(ptr) );
    }
    for (int i = 0; i < initThreadSize_; ++i) {
        threads_[i]->start();
        idleThreadSize_++;
    }
}

Result ThreadPool::submitTask(const std::shared_ptr<Task> &task) {
    std::unique_lock<std::mutex> lock(taskQueMtx_);
    if (!notFull_.wait_for(lock, std::chrono::seconds(1), [&]() -> auto { return tasksQue_.size() < taskQueMax_; })) {
        //提交失败
        std::cerr << "task queue is full , submit task fail===========" << std::endl;
        return Result(task, false);
    }
    //有空余将任务放入队列中
    tasksQue_.emplace(task);
    taskSize_++;
    //因为放了任务，任务队列就不空了，noEmpty通知
    notEmpty_.notify_all();

    //catched模式：
    if (mode_ == PoolMod::MODE_CACHED && taskSize_ > idleThreadSize_
        && curThreadSize_ < threadMax_) {
        //创建新线程
        //threads_.emplace_back(std::make_unique<Thread>([&]() -> bool { threadFunc(); }));
        std::cout << ">>>>> create new thread <<<<<<" << std::this_thread::get_id() << std::endl;
        auto ptr = std::make_unique<Thread>([this](int PH1){ threadFunc(PH1);});
        int id = ptr->getId();
        threads_.emplace(id , std::move(ptr) );
        threads_[id]->start();
        idleThreadSize_++;
        curThreadSize_++;
    }
    return Result(task);
}


void ThreadPool::threadFunc(int threadid)  // 线程函数返回，相应的线程也就结束了
{
    auto lastTime = std::chrono::high_resolution_clock().now();

    // 所有任务必须执行完成，线程池才可以回收所有线程资源
    for (;;)
    {
        std::shared_ptr<Task> task;
        {
            // 先获取锁
            std::unique_lock<std::mutex> lock(taskQueMtx_);

            std::cout << "tid:" << std::this_thread::get_id()
                      << "尝试获取任务..." << std::endl;

            // cached模式下，有可能已经创建了很多的线程，但是空闲时间超过60s，应该把多余的线程
            // 结束回收掉（超过initThreadSize_数量的线程要进行回收）
            // 当前时间 - 上一次线程执行的时间 > 60s

            // 每一秒中返回一次   怎么区分：超时返回？还是有任务待执行返回
            // 锁 + 双重判断
            while (tasksQue_.size() == 0)
            {
                // 线程池要结束，回收线程资源
                if (!isPoolrunning)
                {
                    threads_.erase(threadid); // std::this_thread::getid()
                    std::cout << "threadid:" << std::this_thread::get_id() << " exit!"
                              << std::endl;
                    exitCond_.notify_all();
                    return; // 线程函数结束，线程结束
                }

                if (mode_ == PoolMod::MODE_CACHED)
                {
                    // 条件变量，超时返回了
                    if (std::cv_status::timeout ==
                        notEmpty_.wait_for(lock, std::chrono::seconds(1)))
                    {
                        auto now = std::chrono::high_resolution_clock().now();
                        auto dur = std::chrono::duration_cast<std::chrono::seconds>(now - lastTime);
                        if (dur.count() >= THREAD_MAX_IDLE_TIME
                            && curThreadSize_ > initThreadSize_)
                        {
                            // 开始回收当前线程
                            // 记录线程数量的相关变量的值修改
                            // 把线程对象从线程列表容器中删除   没有办法 threadFunc《=》thread对象
                            // threadid => thread对象 => 删除
                            threads_.erase(threadid); // std::this_thread::getid()
                            curThreadSize_--;
                            idleThreadSize_--;

                            std::cout << "threadid:" << std::this_thread::get_id() << " exit!"
                                      << std::endl;
                            return;
                        }
                    }
                }
                else
                {
                    // 等待notEmpty条件
                    notEmpty_.wait(lock);
                }

                //if (!isPoolRunning_)
                //{
                //	threads_.erase(threadid); // std::this_thread::getid()
                //	std::cout << "threadid:" << std::this_thread::get_id() << " exit!"
                //		<< std::endl;
                //	exitCond_.notify_all();
                //	return; // 结束线程函数，就是结束当前线程了!
                //}
            }

            idleThreadSize_--;

            std::cout << "tid:" << std::this_thread::get_id()
                      << "获取任务成功..." << std::endl;

            // 从任务队列种取一个任务出来
            task = tasksQue_.front();
            tasksQue_.pop();
            taskSize_--;

            // 如果依然有剩余任务，继续通知其它得线程执行任务
            if (tasksQue_.size() > 0)
            {
                notEmpty_.notify_all();
            }

            // 取出一个任务，进行通知，通知可以继续提交生产任务
            notFull_.notify_all();
        } // 就应该把锁释放掉

        // 当前线程负责执行这个任务
        if (task != nullptr)
        {
            // task->run(); // 执行任务；把任务的返回值setVal方法给到Result
            task->exec();
        }

        idleThreadSize_++;
        lastTime = std::chrono::high_resolution_clock().now(); // 更新线程执行完任务的时间
    }
}


bool ThreadPool::checkRunningState() const {
    return isPoolrunning;
}


void Task::exec() {
    if (result_ != nullptr)
        result_->setVal(run());
}

void Task::setResult(Result *res) {
    result_ = res;
}

Task::Task() : result_(nullptr) {}
