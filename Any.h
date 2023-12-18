//
// Created by satellite on 13/12/2023.
//

#ifndef THREADPOOL_ANY_H
#define THREADPOOL_ANY_H

#include <memory>

//Any类型：可以接收任意数据的类型
class Any {
public:
    Any() = default;

    template<class T>
    Any(T data)
            : base_ ( std::make_unique<Derive<T>>(data) )
    {
    }

    template<class T>
    T cast() {
        Derive<T>* ptr = dynamic_cast<Derive<T>*>(base_.get());
        if (ptr == nullptr) {
            throw "type is incompatible";
        }
        return ptr->data_;
    }

private:
    class Base {        //基类
    public:
        virtual ~Base() = default;
    };

    template<typename T>
    class Derive : public Base {
    public:
        Derive(T data) : data_(data) {  }

        T data_;
    };

private:
    std::unique_ptr<Base> base_;    //定义指向基类的指针

};



#endif //THREADPOOL_ANY_H
