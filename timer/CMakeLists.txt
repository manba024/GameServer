cmake_minimum_required(VERSION 3.10)
project(timer)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB SOURCES src/timer.cpp)
add_library(timer ${SOURCES})

target_include_directories(timer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(timer_test src/timer_test.cpp)
target_link_libraries(timer_test timer) 