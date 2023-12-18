//
// Created by satellite on 13/12/2023.
//

#include "Result.h"

Result::Result(std::shared_ptr<Task> task, bool isValid)
    :task_(task) , isValid_(isValid)
{
    task_->setResult(this);
}

void Result::setVal(Any any) {
    any_ = std::move(any);
    sem_.post();
}

Any Result::get() {
    if(!isValid_) {
        return Any{};
    }
    sem_.wait();
    return std::move(any_);
}
