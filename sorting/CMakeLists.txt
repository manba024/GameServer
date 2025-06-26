cmake_minimum_required(VERSION 3.12)
project(SortingLibrary VERSION 1.0.0 LANGUAGES CXX)

# C++标准设置
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 编译选项
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -O2)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_compile_options(/W4 /O2)
endif()

# 包含目录
include_directories(include)

# 源文件列表
set(SORTING_SOURCES
    src/bubble_sort.cpp
    src/selection_sort.cpp
    src/insertion_sort.cpp
    src/quick_sort.cpp
    src/merge_sort.cpp
    src/sort_factory.cpp
)

# 头文件列表（用于IDE显示）
set(SORTING_HEADERS
    include/bubble_sort.h
    include/selection_sort.h
    include/insertion_sort.h
    include/quick_sort.h
    include/merge_sort.h
    include/sort_factory.h
    include/sorting.h
)

# 创建静态库
add_library(sorting_static STATIC ${SORTING_SOURCES} ${SORTING_HEADERS})
target_include_directories(sorting_static PUBLIC include)

# 创建动态库（可选）
add_library(sorting_shared SHARED ${SORTING_SOURCES} ${SORTING_HEADERS})
target_include_directories(sorting_shared PUBLIC include)
set_target_properties(sorting_shared PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)

# 别名库
add_library(Sorting::Static ALIAS sorting_static)
add_library(Sorting::Shared ALIAS sorting_shared)

# 演示程序
add_executable(demo_sorting examples/demo_sorting.cpp)
target_link_libraries(demo_sorting sorting_static)

# 性能测试程序（如果文件存在）
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/examples/performance_test.cpp)
    add_executable(performance_test examples/performance_test.cpp)
    target_link_libraries(performance_test sorting_static)
endif()

# 单元测试（如果启用测试）
option(BUILD_TESTS "Build unit tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    find_package(GTest REQUIRED)
    
    add_executable(sorting_tests tests/test_sorting.cpp)
    target_link_libraries(sorting_tests 
        sorting_static 
        GTest::gtest 
        GTest::gtest_main
    )
    
    add_test(NAME SortingTests COMMAND sorting_tests)
endif()

# 安装配置
install(TARGETS sorting_static sorting_shared
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES ${SORTING_HEADERS}
    DESTINATION include/sorting
)

# 生成配置文件
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/SortingConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# 创建简单的配置文件
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/SortingConfig.cmake"
"# SortingConfig.cmake
get_filename_component(SORTING_CMAKE_DIR \"\${CMAKE_CURRENT_LIST_FILE}\" PATH)
include(\"\${SORTING_CMAKE_DIR}/SortingTargets.cmake\")
")

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/SortingConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/SortingConfigVersion.cmake"
    DESTINATION lib/cmake/Sorting
)

# 打印构建信息
message(STATUS "Sorting Library Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "  Build Tests: ${BUILD_TESTS}")

# 目标属性
set_target_properties(sorting_static PROPERTIES
    OUTPUT_NAME "sorting"
    DEBUG_POSTFIX "_d"
)

set_target_properties(sorting_shared PROPERTIES
    OUTPUT_NAME "sorting"
    DEBUG_POSTFIX "_d"
)

# 编译器特定设置
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(sorting_static PRIVATE -fPIC)
    target_compile_options(sorting_shared PRIVATE -fPIC)
endif()

# 平台特定设置
if(WIN32)
    target_compile_definitions(sorting_shared PRIVATE SORTING_EXPORTS)
endif()

# 文档生成（如果找到Doxygen）
find_package(Doxygen QUIET)
if(DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    
    if(EXISTS ${DOXYGEN_IN})
        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
        add_custom_target(docs
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM
        )
    endif()
endif()

# 代码格式化（如果找到clang-format）
find_program(CLANG_FORMAT_EXECUTABLE clang-format)
if(CLANG_FORMAT_EXECUTABLE)
    file(GLOB_RECURSE ALL_SOURCE_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/examples/*.cpp
    )
    
    add_custom_target(format
        COMMAND ${CLANG_FORMAT_EXECUTABLE} -i ${ALL_SOURCE_FILES}
        COMMENT "Formatting source code with clang-format"
        VERBATIM
    )
endif()

# CPack配置（用于生成安装包）
set(CPACK_PACKAGE_NAME "SortingLibrary")
set(CPACK_PACKAGE_VENDOR "GameServer Project")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "C++ Sorting Algorithm Library")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "SortingLibrary")

if(WIN32)
    set(CPACK_GENERATOR "NSIS")
else()
    set(CPACK_GENERATOR "TGZ")
endif()

include(CPack) 