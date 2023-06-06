cmake_minimum_required(VERSION 3.0.0)
project(test_translator VERSION 0.1.0)

include(CTest)
enable_testing()

find_package(GTest REQUIRED)

aux_source_directory(${PROJECT_SOURCE_DIR} SRC_LIST)

add_executable(${PROJECT_NAME} ${SRC_LIST})

target_link_libraries(
    ${PROJECT_NAME}
    gtest
    gtest_main
    gmock
    gmock_main
)

gtest_discover_tests(${PROJECT_NAME})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)