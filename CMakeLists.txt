cmake_minimum_required(VERSION 3.10)
project(GameServer)

# 添加子项目
add_subdirectory(timer)
add_subdirectory(webserver)
add_subdirectory(coroutine)
add_subdirectory(string_utils)
