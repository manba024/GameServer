cmake_minimum_required(VERSION 3.10)
project(IOMultiplexing)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置编译选项
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")
endif()

# 平台特定设置
if(UNIX AND NOT APPLE)
    # Linux平台
    add_definitions(-D__LINUX__)
elseif(APPLE)
    # macOS平台
    add_definitions(-D__APPLE__)
elseif(CMAKE_SYSTEM_NAME MATCHES "FreeBSD|OpenBSD|NetBSD")
    # BSD平台
    add_definitions(-D__BSD__)
endif()

# 包含头文件目录
include_directories(include)

# 基础源文件
set(SOURCES
    src/select_multiplexer.cpp
    src/poll_multiplexer.cpp
    src/multiplexer_factory.cpp
)

# Linux特有的源文件
if(UNIX AND NOT APPLE)
    list(APPEND SOURCES src/epoll_multiplexer.cpp)
endif()

# BSD/macOS特有的源文件
if(APPLE OR CMAKE_SYSTEM_NAME MATCHES "FreeBSD|OpenBSD|NetBSD")
    list(APPEND SOURCES src/kqueue_multiplexer.cpp)
endif()

# 显示将要编译的源文件
message(STATUS "编译源文件:")
foreach(src ${SOURCES})
    message(STATUS "  ${src}")
endforeach()

# 创建静态库
add_library(io_multiplexing STATIC ${SOURCES})

# 设置头文件路径
target_include_directories(io_multiplexing PUBLIC include)

# 链接线程库
find_package(Threads REQUIRED)
target_link_libraries(io_multiplexing Threads::Threads)

# 构建演示程序
add_executable(demo_basic examples/demo_basic.cpp)
target_link_libraries(demo_basic io_multiplexing)

# Linux特有的触发模式演示程序
if(UNIX AND NOT APPLE)
    add_executable(demo_trigger_modes examples/demo_trigger_modes.cpp)
    target_link_libraries(demo_trigger_modes io_multiplexing)
endif()

# 设置输出目录
set_target_properties(io_multiplexing PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

set_target_properties(demo_basic PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# 安装规则
install(TARGETS io_multiplexing
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
) 