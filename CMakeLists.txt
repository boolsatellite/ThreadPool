cmake_minimum_required(VERSION 3.16)
project(ThreadPool)

set(CMAKE_CXX_STANDARD 17)

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

include_directories(${CMAKE_SOURCE_DIR})
message(${CMAKE_SOURCE_DIR}=======)

add_executable(ThreadPool
        ThreadPool.cc
        Thread.cc
        main.cc
        Semaphone.cc
        Result.cc)

target_link_libraries(ThreadPool pthread)

