cmake_minimum_required(VERSION 3.10.2)

project (testserver C CXX)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

enable_testing()

set(CXX_FLAGS
-g
 # -DVALGRIND
 -DCHECK_PTHREAD_RETURN_VALUE
 -D_FILE_OFFSET_BITS=64
 -Wall
 -Wextra
 -Wconversion
 -Wno-unused-parameter
 -Wold-style-cast
 -Woverloaded-virtual
 -Wpointer-arith
 -Wshadow
 -Wwrite-strings
 -march=native
 # -MMD
 -std=c++11
 -rdynamic
 -lpthread
 )

if(CMAKE_BUILD_BITS EQUAL 32)
 list(APPEND CXX_FLAGS "-m32")
endif()
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
 list(APPEND CXX_FLAGS "-Wno-null-dereference")
 list(APPEND CXX_FLAGS "-Wno-sign-conversion")
 list(APPEND CXX_FLAGS "-Wno-unused-local-typedef")
 list(APPEND CXX_FLAGS "-Wthread-safety")
 list(REMOVE_ITEM CXX_FLAGS "-rdynamic")
endif()

#每一次listAppend都会在string后加入；，需要replace去除
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

aux_source_directory(. DIRSRCS)
include_directories(./ ./net ./base)

aux_source_directory(base SRC_LIST1)
aux_source_directory(net SRC_LIST2)

string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
add_executable(testserver ${DIRSRCS} ${SRC_LIST1} ${SRC_LIST2})
target_link_libraries(${PROJECT_NAME}  ${CMAKE_THREAD_LIBS_INIT})

