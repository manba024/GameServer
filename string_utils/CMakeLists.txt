# string_utils/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(string_utils)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SOURCES src/string_utils.cpp)
add_library(string_utils ${SOURCES})

target_include_directories(string_utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# 可选：添加测试可执行文件
add_executable(string_utils_test src/string_utils_test.cpp)
target_include_directories(string_utils_test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(string_utils_test string_utils)