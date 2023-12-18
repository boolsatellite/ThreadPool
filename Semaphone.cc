//
// Created by satellite on 13/12/2023.
//

#include "Semaphone.h"

Semaphone::Semaphone(int limit) : resLimit_(limit) , cond_(){  }

void Semaphone::post() {
    std::unique_lock<std::mutex> lock(mtx_);
    resLimit_++;
    cond_.notify_all();
}

void Semaphone::wait() {
    std::unique_lock<std::mutex> lock(mtx_);
    cond_.wait(lock , [&]()->auto { return resLimit_ > 0;});
    resLimit_--;
}
