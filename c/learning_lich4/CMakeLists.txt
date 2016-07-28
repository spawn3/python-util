cmake_minimum_required(VERSION 2.8)
project(learning_lich4)

set(CMAKE_C_FLAGS "-g -Wall -Werror -rdynamic -D_GUN_SOURCE -D_REENTRANT")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(LICH_HOME "/root/huayun/lich4")

set(SOURCE_FILES
    src/main.c
    )

include_directories(
    ${LICH_HOME}/include
    ${LICH_HOME}/config/include
    ${LICH_HOME}/ylib/include
    )

add_executable(learning_lich4 ${SOURCE_FILES})
target_link_libraries(lich4)
