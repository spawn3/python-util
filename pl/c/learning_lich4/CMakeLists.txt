cmake_minimum_required(VERSION 2.8)
project(learning_lich4 C)

set(LICH_HOME "/home/gj/huayun/lich4")

set(CMAKE_C_FLAGS "-g -W -Wall -Werror -rdynamic -D_GUN_SOURCE -D_REENTRANT")
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

function(build_tool name source_files libs)
    add_executable(${name} ${source_files})
    target_link_libraries(${name} ${libs})
endfunction()

set(LIBS lich4 sqlite3 uuid pthread aio crypt crypto ssl rt m)

set(LEARNING_LICH4_SOURCE_FILES
    src/main.c
    src/utils.c
    )
build_tool(learning_lich4 "${LEARNING_LICH4_SOURCE_FILES}" "${LIBS}")

set(LICH_FS_SOURCE_FILES
    src/lich.fs.c
    src/attr.c
    src/copy.c
    src/license.c
    src/utils.c
    )
build_tool(lich.fs "${LICH_FS_SOURCE_FILES}" "${LIBS}")

set(LICH_INSPECT_SOURCE_FILES
    src/lich.inspect.c
    src/recovery.c
    src/md5sum.c
    src/license.c
    src/hierarch.c
    )
build_tool(lich.inspect "${LICH_INSPECT_SOURCE_FILES}" "${LIBS}")

set(LICH_BD_SOURCE_FILES
    src/lichbd_utils.c
    src/attr.c
    src/copy.c
    src/option.c
    src/license.c
    src/utils.c
    )
build_tool(lich.bd "${LICH_BD_SOURCE_FILES}" "${LIBS}")

set(LICH_SNAPSHOT_SOURCE_FILES
    src/lich.snapshot.c
    src/license.c
    src/utils.c
    )
build_tool(lich.snapshot "${LICH_SNAPSHOT_SOURCE_FILES}" "${LIBS}")
