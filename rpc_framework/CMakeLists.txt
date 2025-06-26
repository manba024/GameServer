cmake_minimum_required(VERSION 3.10)
project(RPCFramework VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 编译选项
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
endif()

# 检测平台
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_definitions(-D__LINUX__)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    add_definitions(-D__APPLE__)
elseif(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    add_definitions(-D__FREEBSD__)
endif()

# 查找依赖库
find_package(Threads REQUIRED)

# 包含目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# 源文件
set(RPC_SOURCES
    src/json_serializer.cpp
    src/tcp_transport.cpp
    src/rpc_server.cpp
    src/rpc_client.cpp
)

# 注意：目前只实现了基础的TCP传输层
# 未来可以根据需要添加平台特定的实现：
# - Linux: epoll_transport.cpp
# - macOS/FreeBSD: kqueue_transport.cpp
# - Windows: iocp_transport.cpp

# 创建静态库
add_library(rpc_static STATIC ${RPC_SOURCES})
target_link_libraries(rpc_static Threads::Threads)

# 创建动态库
add_library(rpc_shared SHARED ${RPC_SOURCES})
target_link_libraries(rpc_shared Threads::Threads)

# 设置库的输出名称
set_target_properties(rpc_static PROPERTIES OUTPUT_NAME rpc)
set_target_properties(rpc_shared PROPERTIES OUTPUT_NAME rpc)

# 示例程序
add_executable(calculator_demo examples/calculator_demo.cpp)
target_link_libraries(calculator_demo rpc_static Threads::Threads)

# 添加简单的HTTP示例
add_executable(http_demo examples/http_demo.cpp)
target_link_libraries(http_demo rpc_static Threads::Threads)

# 性能测试程序
add_executable(benchmark examples/benchmark.cpp)
target_link_libraries(benchmark rpc_static Threads::Threads)

# 安装配置
install(TARGETS rpc_static rpc_shared
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/
    DESTINATION include/rpc
    FILES_MATCHING PATTERN "*.h"
)

install(TARGETS calculator_demo http_demo benchmark
    RUNTIME DESTINATION bin
)

# 打包配置
set(CPACK_PACKAGE_NAME "RPCFramework")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "高性能C++ RPC框架")
set(CPACK_PACKAGE_VENDOR "RPC Framework Team")
set(CPACK_GENERATOR "TGZ;ZIP")

include(CPack)

# 测试配置
enable_testing()

# 添加基本测试
add_test(NAME calculator_test
    COMMAND calculator_demo
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# 打印构建信息
message(STATUS "RPC Framework Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  System: ${CMAKE_SYSTEM_NAME}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Install Prefix: ${CMAKE_INSTALL_PREFIX}")

# 编译命令
message(STATUS "Build Commands:")
message(STATUS "  mkdir build && cd build")
message(STATUS "  cmake ..")
message(STATUS "  make -j$(nproc)")
message(STATUS "  make install")

# 运行示例
message(STATUS "Run Examples:")
message(STATUS "  ./calculator_demo")
message(STATUS "  ./http_demo")
message(STATUS "  ./benchmark") 