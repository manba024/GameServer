cmake_minimum_required(VERSION 3.10)
project(coroutine)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")

# 包含头文件目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# 收集源文件
file(GLOB SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# 创建协程库
add_library(coroutine STATIC ${SOURCES})

# 设置库的头文件目录
target_include_directories(coroutine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# 链接必要的系统库
if(UNIX AND NOT APPLE)
    target_link_libraries(coroutine pthread)
endif()

# 创建基本示例程序
add_executable(coroutine_basic_example 
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/basic_example.cpp
)
target_link_libraries(coroutine_basic_example coroutine)

# 创建高级示例程序
add_executable(coroutine_advanced_example 
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/advanced_example.cpp
)
target_link_libraries(coroutine_advanced_example coroutine)

# 设置可执行文件输出目录
set_target_properties(coroutine_basic_example coroutine_advanced_example
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# 打印信息
message(STATUS "Coroutine library configuration:")
message(STATUS "  - C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  - Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  - Output directory: ${CMAKE_BINARY_DIR}/bin")
message(STATUS "  - Examples: coroutine_basic_example, coroutine_advanced_example") 