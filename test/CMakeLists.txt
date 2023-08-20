cmake_minimum_required(VERSION 3.0.0)
project(test_amphisbaena VERSION 0.1.0)

include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(tool)
add_subdirectory(mock/plugin)
add_subdirectory(common)
add_subdirectory(plugin)

aux_source_directory(${PROJECT_SOURCE_DIR} SRC_LIST)

add_executable(${PROJECT_NAME} ${SRC_LIST})

target_link_libraries(${PROJECT_NAME} PRIVATE
    gtest
    gmock
    quickfix
    amphisbaena_core
    test_tool
    test_http_message
    test_uds_server
    test_tcp_server
    test_http_parser
    test_http_to_fix
    test_fix_client
    test_http_server
)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})
