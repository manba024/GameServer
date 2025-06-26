cmake_minimum_required(VERSION 3.12)
project(GameServer VERSION 1.0.0 LANGUAGES CXX)

# C++标准设置
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 编译选项
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -O2)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_compile_options(/W4 /O2)
endif()

# 构建类型设置
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# 项目选项
option(BUILD_TESTS "Build unit tests" OFF)
option(BUILD_EXAMPLES "Build example programs" ON)
option(BUILD_BENCHMARKS "Build benchmark programs" OFF)

# 核心基础库
add_subdirectory(io_multiplexing)
add_subdirectory(threadpool)
add_subdirectory(sorting)

# 工具库
add_subdirectory(timer)
add_subdirectory(string_utils)

# 网络和RPC框架
add_subdirectory(rpc_framework)

# 应用组件
add_subdirectory(leaderboard)
add_subdirectory(distributed_id)

# Web服务器和协程（可选）
# add_subdirectory(webserver)
# add_subdirectory(coroutine)

# 全局安装配置
install(FILES README.md DESTINATION .)

# 打印构建信息
message(STATUS "GameServer Project Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "  Build Tests: ${BUILD_TESTS}")
message(STATUS "  Build Examples: ${BUILD_EXAMPLES}")
message(STATUS "  Build Benchmarks: ${BUILD_BENCHMARKS}")

# 子项目依赖关系说明
message(STATUS "")
message(STATUS "Subprojects:")
message(STATUS "  ✅ io_multiplexing - IO复用框架 (Select/Poll/Epoll/Kqueue)")
message(STATUS "  ✅ threadpool - 线程池库 (Fixed/Cached/Priority)")
message(STATUS "  ✅ sorting - 排序算法库 (Bubble/Selection/Insertion/Quick/Merge)")
message(STATUS "  ✅ timer - 定时器组件")
message(STATUS "  ✅ string_utils - 字符串工具")
message(STATUS "  ✅ rpc_framework - 高性能RPC框架 (TCP/HTTP/UDP + JSON/Binary/MessagePack)")
message(STATUS "  ✅ leaderboard - 排行榜系统")
message(STATUS "  ✅ distributed_id - 分布式ID生成器")
message(STATUS "  ⏸️  webserver - Web服务器 (待启用)")
message(STATUS "  ⏸️  coroutine - 协程库 (待启用)")

# CPack配置
set(CPACK_PACKAGE_NAME "GameServer")
set(CPACK_PACKAGE_VENDOR "GameServer Project")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Game Server Development Framework")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "GameServer")

if(WIN32)
    set(CPACK_GENERATOR "NSIS")
else()
    set(CPACK_GENERATOR "TGZ")
endif()

include(CPack)
