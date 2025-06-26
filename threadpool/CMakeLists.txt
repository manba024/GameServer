cmake_minimum_required(VERSION 3.10)
project(ThreadPool)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")

# 包含头文件目录
include_directories(include)

# 源文件
set(SOURCES
    src/thread_pool.cpp
    src/fixed_thread_pool.cpp
    src/cached_thread_pool.cpp
    src/priority_thread_pool.cpp
    src/thread_pool_factory.cpp
)

# 显示将要编译的源文件
message(STATUS "编译源文件:")
foreach(src ${SOURCES})
    message(STATUS "  ${src}")
endforeach()

# 创建静态库
add_library(threadpool STATIC ${SOURCES})

# 创建演示程序
add_executable(demo_threadpool examples/demo_basic.cpp)
target_link_libraries(demo_threadpool threadpool)

# 查找并链接线程库
find_package(Threads REQUIRED)
target_link_libraries(threadpool Threads::Threads)
target_link_libraries(demo_threadpool Threads::Threads)

# 设置输出目录
set_target_properties(threadpool PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

set_target_properties(demo_threadpool PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# 安装配置
install(TARGETS threadpool demo_threadpool
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/ DESTINATION include/threadpool)

# 显示构建信息
message(STATUS "ThreadPool项目配置完成")
message(STATUS "  C++标准: ${CMAKE_CXX_STANDARD}")
message(STATUS "  编译选项: ${CMAKE_CXX_FLAGS}")
message(STATUS "  输出目录: ${CMAKE_BINARY_DIR}") 