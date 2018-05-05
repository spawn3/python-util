cmake_minimum_required(VERSION 3.0)
project(hello)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -std=c11 -Wall -Werror")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

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
    )
build_exe(hello "${HELLO_SOURCE_FILES}")
