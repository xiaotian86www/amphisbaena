cmake_minimum_required(VERSION 3.0.0)
project(test_amphisbaena VERSION 0.1.0)

find_package(GTest REQUIRED)

include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(tool)
add_subdirectory(mock/plugin)

aux_source_directory(${PROJECT_SOURCE_DIR} SRC_LIST)
aux_source_directory(${PROJECT_SOURCE_DIR}/impl SRC_LIST)
aux_source_directory(${PROJECT_SOURCE_DIR}/plugin SRC_LIST)

add_executable(${PROJECT_NAME} ${SRC_LIST})

target_link_libraries(${PROJECT_NAME} PRIVATE
    gtest
    gmock
    test_tool
    amphisbaena_core
    http_to_fix
    fix_client
    http_server
)

gtest_discover_tests(${PROJECT_NAME})
