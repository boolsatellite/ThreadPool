//
// Created by satellite on 13/12/2023.
//

#ifndef THREADPOOL_SEMAPHONE_H
#define THREADPOOL_SEMAPHONE_H


#include <thread>
#include <mutex>
#include <condition_variable>

class Semaphone {
public:
    explicit Semaphone(int limit = 0);
    ~Semaphone() = default;

    void wait();
    void post();


private:
    int resLimit_;
    std::mutex mtx_;
    std::condition_variable cond_;

};


#endif //THREADPOOL_SEMAPHONE_H
