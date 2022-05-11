#cmake版本最低要求
cmake_minimum_required(VERSION 2.6)

#项目名+语言
project(testserver C CXX)

enable_testing()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

# only build examples if this is the main project
if(CMAKE_PROJECT_NAME STREQUAL "muduo")
  option(MUDUO_BUILD_EXAMPLES "Build Muduo examples" ON)
endif()

#编译器c++选项
set(CXX_FLAGS
 -g
 # -DVALGRIND
 -DCHECK_PTHREAD_RETURN_VALUE
 -D_FILE_OFFSET_BITS=64
 -Wall
 -Wextra
 -Werror
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
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

include_directories(${Boost_INCLUDE_DIRS})
#这一句是不能删除的，否则会导致不能找到文件
include_directories(${PROJECT_SOURCE_DIR})

string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
message(STATUS "CXX_FLAGS = " ${CMAKE_CXX_FLAGS} " " ${CMAKE_CXX_FLAGS_${BUILD_TYPE}})

#添加子目录
#cmake处理
add_subdirectory(muduo/base)
add_subdirectory(muduo/net)
add_subdirectory(chatserver)
add_subdirectory(mysqlapi)
#将子目录install的目录包含
include_directories(muduo/base)
include_directories(muduo/net)
include_directories(chatserver)
add_executable(testserver test_construct.cpp)

#将库链接
target_link_libraries(${PROJECT_NAME}
    muduo_base
    muduo_net
    chat_lib
    mysql_lib
)

