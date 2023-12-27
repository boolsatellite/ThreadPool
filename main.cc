//
// Created by satellite on 2023-12-08.
//
#include "ThreadPool.h"
#include "Thread.h"
#include <thread>
#include <chrono>
#include <iostream>
#include <unistd.h>

class MyTask : public Task {
public:
    MyTask(int begin , int end)
        : begin_(begin)
        , end_(end)
    {}

    Any run() override {
        std::cout <<"run tid " << std::this_thread::get_id() << " begin\n";

        int sum = 0;
        for(int i=begin_ ; i <= end_ ; i++) {
            sum += i;
        }
        ::sleep(30);
        std::cout <<"run tid " << std::this_thread::get_id() << " end\n";
        return sum;
    }

private:
    int begin_;
    int end_;
};


int main() {
    {
        ThreadPool threadPool;
        threadPool.setMode(PoolMod::MODE_CACHED);
        threadPool.start(4);
        Result res1 = threadPool.submitTask(std::make_shared<MyTask>(1, 1000));
        Result res2 = threadPool.submitTask(std::make_shared<MyTask>(1, 1000));
        Result res3 = threadPool.submitTask(std::make_shared<MyTask>(1, 1000));
        Result res4 = threadPool.submitTask(std::make_shared<MyTask>(1, 1000));
        Result res5 = threadPool.submitTask(std::make_shared<MyTask>(1, 1000));
        Result res6 = threadPool.submitTask(std::make_shared<MyTask>(1, 1000));
/*
        int ret1 = res1.get().cast<int>();
        std::cout << "sum = " << (ret1) << std::endl;
*/
    }
    std::cout << "main over" << std::endl;
}
