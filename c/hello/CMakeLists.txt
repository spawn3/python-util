cmake_minimum_required(VERSION 3.0)
project(hello C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Werror")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

macro(debug msg)
    message(STATUS "DEBUG ${msg}")
endmacro()

macro(debugValue varName)
    debug("${varName}=\${${varName}}")
endmacro()

set(MyFoo "MyFoo's value")
debugValue(MyFoo)

include_directories(include)

function(build_exe name source_files)
    add_executable(${name} ${source_files})
endfunction()

set(SOURCE_FILES
    src/hello.c
    src/test1.c
    )
build_exe(hello "${SOURCE_FILES}")
