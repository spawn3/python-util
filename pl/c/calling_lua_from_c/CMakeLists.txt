cmake_minimum_required (VERSION 3.0)
project (lua_embed C)

find_package(Lua REQUIRED)

include_directories (
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${LUA_INCLUDE_DIR}
    )

set (SOURCES
    main.c
    )

add_executable (${PROJECT_NAME} ${SOURCES} ${LUA_LIBRARIES})
target_link_libraries (${PROJECT_NAME} lua)
