cmake_minimum_required(VERSION 3.10)
project(my_string_utils_demo)

# 设置C++标准为C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SOURCES src/*.cpp)

add_executable(my_string_utils_demo ${SOURCES}) 