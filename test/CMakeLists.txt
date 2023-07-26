cmake_minimum_required(VERSION 3.0.0)
project(test_translator VERSION 0.1.0)

find_package(GTest REQUIRED)

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/../src)

add_subdirectory(tool)
add_subdirectory(mock/builder)

aux_source_directory(${PROJECT_SOURCE_DIR} SRC_LIST)
aux_source_directory(${PROJECT_SOURCE_DIR}/impl SRC_LIST)
aux_source_directory(${PROJECT_SOURCE_DIR}/builder SRC_LIST)

add_executable(${PROJECT_NAME} ${SRC_LIST})

target_link_libraries(
    ${PROJECT_NAME}
    gtest
    gtest_main
    gmock
    gmock_main
    test_tool
    translator
    http_to_fix
)

gtest_discover_tests(${PROJECT_NAME})
