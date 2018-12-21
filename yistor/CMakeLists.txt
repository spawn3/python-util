cmake_minimum_required(VERSION 2.8)
project(yistor)

set (YISTOR "${CMAKE_CURRENT_SOURCE_DIR}")

set(YISTOR_SRC
    ${YISTOR}/src/utils/main.cpp
    )
add_executable(yistor ${YISTOR_SRC})
# target_link_libraries(lich.admin ${CMAKE_C_LIBS})
