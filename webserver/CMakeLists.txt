cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(WebServer)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")

# 包含头文件目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# 收集所有源文件
file(GLOB_RECURSE SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# 排除main.cpp，因为它将单独用于可执行文件
list(REMOVE_ITEM SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

# 创建静态库
add_library(webserver_lib STATIC ${SOURCES})

# 创建可执行文件
add_executable(webserver ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

# 链接库
target_link_libraries(webserver webserver_lib)

# 设置可执行文件输出目录
set_target_properties(webserver PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# 创建resources目录和基本HTML文件
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/resources")

# 创建一个基本的index.html文件
file(WRITE "${CMAKE_BINARY_DIR}/resources/index.html"
"<!DOCTYPE html>
<html>
<head>
    <title>WebServer</title>
    <meta charset=\"UTF-8\">
</head>
<body>
    <h1>欢迎使用WebServer</h1>
    <p>这是一个高性能的C++ WebServer实现</p>
    <p>服务器正在正常运行！</p>
</body>
</html>
")

# 创建404错误页面
file(WRITE "${CMAKE_BINARY_DIR}/resources/404.html"
"<!DOCTYPE html>
<html>
<head>
    <title>404 Not Found</title>
    <meta charset=\"UTF-8\">
</head>
<body>
    <h1>404 - 页面未找到</h1>
    <p>抱歉，您请求的页面不存在。</p>
</body>
</html>
")

# 创建日志目录
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/log")

# 打印信息
message(STATUS "WebServer build configuration:")
message(STATUS "  - Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  - C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  - Output directory: ${CMAKE_BINARY_DIR}/bin")
message(STATUS "  - Resources directory: ${CMAKE_BINARY_DIR}/resources")
message(STATUS "  - Log directory: ${CMAKE_BINARY_DIR}/log") 