cmake_minimum_required(VERSION 3.0)
project(hello)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    src/hello.c
    src/test1.c
    )

include_directories(include)

add_executable(hello ${SOURCE_FILES})
