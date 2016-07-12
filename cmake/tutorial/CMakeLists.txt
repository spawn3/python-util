cmake_minimum_required(VERSION 3.0)

project (example)
set(CMAKE_BUILD_TYPE Release)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/example1/include
    ${CMAKE_CURRENT_SOURCE_DIR}/example2/include
    )
add_library(example SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/example1/src/example1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/example2/src/example2.c
    )
