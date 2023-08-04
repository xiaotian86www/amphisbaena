cmake_minimum_required(VERSION 3.0.0)
project(test_amphisbaena VERSION 0.1.0)

find_package(GTest REQUIRED)

include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(tool)
add_subdirectory(mock/plugin)
add_subdirectory(plugin)

aux_source_directory(${PROJECT_SOURCE_DIR} SRC_LIST)

add_executable(${PROJECT_NAME} ${SRC_LIST})

target_link_libraries(${PROJECT_NAME} PRIVATE
    gtest
    gmock
    amphisbaena_core
    quickfix
    test_tool
    test_http_to_fix
    test_fix_client
    test_http_server
)

gtest_discover_tests(${PROJECT_NAME})
