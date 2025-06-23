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

# 持久化相关库
add_library(leaderboard_persistent src/leaderboard_persistent.cpp)
add_library(storage_file src/storage_file.cpp)
add_library(storage_sqlite src/storage_sqlite.cpp)
add_library(storage_factory src/storage_factory.cpp)

# 设置包含目录
target_include_directories(leaderboard_vector PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(leaderboard_set PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(leaderboard_heap PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(leaderboard_skiplist PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(leaderboard_factory PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(leaderboard_persistent PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(storage_file PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(storage_sqlite PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(storage_factory PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# 工厂库链接所有实现
target_link_libraries(leaderboard_factory 
    leaderboard_vector 
    leaderboard_set 
    leaderboard_heap 
    leaderboard_skiplist
)

# 存储工厂库链接所有存储实现
target_link_libraries(storage_factory
    storage_file
    storage_sqlite
    leaderboard_factory
)

# 持久化库链接依赖
target_link_libraries(leaderboard_persistent
    leaderboard_factory
    storage_factory
)

# 查找SQLite3库
find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(SQLITE3 sqlite3)
    if(SQLITE3_FOUND)
        target_link_libraries(storage_sqlite ${SQLITE3_LIBRARIES})
        target_include_directories(storage_sqlite PRIVATE ${SQLITE3_INCLUDE_DIRS})
        target_compile_options(storage_sqlite PRIVATE ${SQLITE3_CFLAGS_OTHER})
    else()
        # 如果pkg-config找不到，尝试直接链接
        target_link_libraries(storage_sqlite sqlite3)
    endif()
else()
    # 如果没有pkg-config，直接链接
    target_link_libraries(storage_sqlite sqlite3)
endif()

# 原有的测试程序
# add_executable(leaderboard_test src/leaderboard_test.cpp)
# target_link_libraries(leaderboard_test leaderboard_factory)

# 新的综合测试程序
add_executable(test_all_implementations src/test_all_implementations.cpp)
target_link_libraries(test_all_implementations leaderboard_factory)

# 持久化测试程序
add_executable(test_persistent_leaderboard src/test_persistent_leaderboard.cpp)
target_link_libraries(test_persistent_leaderboard 
    leaderboard_persistent
    storage_factory
    leaderboard_factory
) 