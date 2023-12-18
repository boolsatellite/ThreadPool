//
// Created by satellite on 13/12/2023.
//

#ifndef THREADPOOL_RESULT_H
#define THREADPOOL_RESULT_H

#include "Semaphone.h"
#include "Any.h"
#include "ThreadPool.h"

class Task;

class Result {
public:
    Result(std::shared_ptr<Task> task , bool isValid = true);
    ~Result() = default;

    void setVal(Any any);
    Any get();

private:
    Any any_;
    Semaphone sem_;
    std::shared_ptr<Task> task_;
    std::atomic_bool isValid_;      //返回值是否有效，任务提交是否成功
};


#endif //THREADPOOL_RESULT_H
