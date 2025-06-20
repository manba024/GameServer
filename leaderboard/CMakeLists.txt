cmake_minimum_required(VERSION 3.10)
project(leaderboard)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# 创建各个实现的库
add_library(leaderboard_vector src/leaderboard_vector.cpp)
add_library(leaderboard_set src/leaderboard_set.cpp)
add_library(leaderboard_heap src/leaderboard_heap.cpp)
add_library(leaderboard_skiplist src/leaderboard_skiplist.cpp)
add_library(leaderboard_factory src/leaderboard_factory.cpp)

# 设置包含目录
target_include_directories(leaderboard_vector PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(leaderboard_set PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(leaderboard_heap PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(leaderboard_skiplist PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(leaderboard_factory PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# 工厂库链接所有实现
target_link_libraries(leaderboard_factory 
    leaderboard_vector 
    leaderboard_set 
    leaderboard_heap 
    leaderboard_skiplist
)

# 原有的测试程序
# add_executable(leaderboard_test src/leaderboard_test.cpp)
# target_link_libraries(leaderboard_test leaderboard_factory)

# 新的综合测试程序
add_executable(test_all_implementations src/test_all_implementations.cpp)
target_link_libraries(test_all_implementations leaderboard_factory) 