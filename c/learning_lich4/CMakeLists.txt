cmake_minimum_required(VERSION 2.8)
project(learning_lich4)

set(LICH_HOME "/root/huayun/lich4")

set(CMAKE_C_FLAGS "-g -Wall -Werror -rdynamic -D_GUN_SOURCE -D_REENTRANT")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(
    ${LICH_HOME}/include
    ${LICH_HOME}/config/include
    ${LICH_HOME}/ylib/include
    ${LICH_HOME}/ynet/include
    ${LICH_HOME}/cluster/include
    include
    )

link_directories(
    /opt/fusionstack/lich/lib
    )

set(SOURCE_FILES
    src/main.c
    src/utils.c
    )

add_executable(learning_lich4 ${SOURCE_FILES})
target_link_libraries(learning_lich4 lich4 sqlite3 uuid pthread aio crypt crypto ssl rt m)
