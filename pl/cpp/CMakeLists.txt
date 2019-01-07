cmake_minimum_required(VERSION 3.0)
project(hello)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -std=c11 -Wall -Werror")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -Wall -Werror")

add_definitions(-std=c++11)

macro(debug msg)
    message(STATUS "DEBUG ${msg}")
endmacro()

macro(debugValue varName)
    debug("${varName}=\${${varName}}")
endmacro()

set(MyFoo "MyFoo's value")
debugValue(MyFoo)

include_directories(
    /usr/local/include
    include
    )

link_directories(
    /usr/local/lib
    )

function(build_exe name source_files)
    add_executable(${name} ${source_files})
endfunction()

set(HELLO_SOURCE_FILES
    src/hello.cc
    src/useless.cc)
build_exe(hello "${HELLO_SOURCE_FILES}")

set(BIT_SOURCE_FILES src/bit.cc)
build_exe(bit "${BIT_SOURCE_FILES}")
target_link_libraries(bit jemalloc)

set(LDB_SOURCE_FILES src/ldb.cc)
add_executable(ldb ${LDB_SOURCE_FILES})
# target_link_libraries(ldb pthread snappy leveldb)  // ERROR
target_link_libraries(ldb leveldb pthread snappy)

set(ALGO_SOURCE_FILES src/algo.cc)
build_exe(algo "${ALGO_SOURCE_FILES}")

set(TH_SOURCE_FILES src/th.cc)
add_executable(th ${TH_SOURCE_FILES})
target_link_libraries(th pthread)

add_library(magic SHARED src/magic.cc)
