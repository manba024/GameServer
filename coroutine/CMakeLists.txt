cmake_minimum_required(VERSION 3.10)
project(coroutine)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SOURCES src/*.cpp)
add_library(coroutine ${SOURCES})

target_include_directories(coroutine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include) 