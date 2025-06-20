cmake_minimum_required(VERSION 3.10)
project(DistributedIdGenerator)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置头文件目录
include_directories(include)

# 收集源文件
file(GLOB_RECURSE SOURCES "src/*.cpp")
list(REMOVE_ITEM SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/id_generator_test.cpp")

# 创建静态库
add_library(distributed_id_lib STATIC ${SOURCES})

# 创建测试可执行文件
add_executable(id_generator_test src/id_generator_test.cpp)
target_link_libraries(id_generator_test distributed_id_lib)

# 如果需要线程支持
find_package(Threads REQUIRED)
target_link_libraries(distributed_id_lib Threads::Threads)
target_link_libraries(id_generator_test Threads::Threads)

# 编译选项
target_compile_options(distributed_id_lib PRIVATE -Wall -Wextra)
target_compile_options(id_generator_test PRIVATE -Wall -Wextra) 