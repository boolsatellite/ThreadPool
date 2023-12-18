//
// Created by satellite on 2023-12-08.
//

#include "Thread.h"
#include <thread>

int Thread::generateId_ = 0;

Thread::Thread(Thread::ThreadFunc cb)
    : func_(std::move(cb))
    , threadId_(generateId_++)
{
}

Thread::~Thread() {    }

void Thread::start() {
    //创建一个线程来执行线程函数
    std::thread t(func_ , this->getId());

    t.detach();
}

int Thread::getId() const {
    return threadId_;
}
