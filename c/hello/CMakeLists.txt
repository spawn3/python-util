cmake_minimum_required(VERSION 3.0)
project(hello C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -std=c11 -Wall -Werror")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

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
    /usr/local/include/hiredis-vip
    )
link_directories(
    /usr/local/lib
    )

function(build_exe name source_files)
    add_executable(${name} ${source_files})
endfunction()

set(HELLO_SOURCE_FILES
    src/hello.c
    src/test1.c
    )
build_exe(hello "${HELLO_SOURCE_FILES}")


set(REDIS_SOURCE_FILES
    src/redis.c)
build_exe(redis "${REDIS_SOURCE_FILES}")
target_link_libraries(redis hiredis_vip gdsl)

set(VALGRIND_SOURCE_FILES
    src/valgrind.c
    )
build_exe(val "${VALGRIND_SOURCE_FILES}")

set(HASH_SOURCE_FILES
    src/hash.c
    )
build_exe(hash "${HASH_SOURCE_FILES}")
target_link_libraries(hash gdsl)
