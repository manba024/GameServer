cmake_minimum_required(VERSION 3.10)
project(timer)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(timer_wheel src/timer_wheel.cpp)
add_library(timer_factory src/timer_factory.cpp)

target_link_libraries(timer_factory timer_wheel)

target_include_directories(timer_wheel PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(timer_factory PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(timer_test src/timer_test.cpp)
target_link_libraries(timer_test timer_factory timer_wheel) 