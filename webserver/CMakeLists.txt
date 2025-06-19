cmake_minimum_required(VERSION 3.10)
project(webserver)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SOURCES src/*.cpp)
add_library(webserver ${SOURCES})

target_include_directories(webserver PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include) 