//
// Created by satellite on 2023-12-08.
//

#ifndef THREADPOOL_THREAD_H
#define THREADPOOL_THREAD_H

#include <functional>



class Thread {
public:
    using ThreadFunc= std::function<void(int)>;
    explicit Thread(ThreadFunc cb);
    ~Thread();

    void start();
    int getId () const;

private:
    ThreadFunc func_;
    int threadId_;
    static int generateId_;
};

#endif //THREADPOOL_THREAD_H
