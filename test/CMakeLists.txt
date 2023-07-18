cmake_minimum_required(VERSION 3.0.0)
project(test_translator VERSION 0.1.0)

add_subdirectory(tool)

find_package(GTest REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/../src)

aux_source_directory(${PROJECT_SOURCE_DIR} SRC_LIST)

add_executable(${PROJECT_NAME} ${SRC_LIST})

target_link_libraries(
    ${PROJECT_NAME}
    gtest
    gtest_main
    gmock
    gmock_main
    test_tool
    translator
)

gtest_discover_tests(${PROJECT_NAME})
