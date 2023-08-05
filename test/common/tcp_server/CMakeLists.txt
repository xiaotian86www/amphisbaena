cmake_minimum_required(VERSION 3.0.0)
project(test_tcp_server VERSION 0.1.0)

aux_source_directory(${PROJECT_SOURCE_DIR} SRC_LIST)

add_library(${PROJECT_NAME} SHARED ${SRC_LIST})

target_link_libraries(${PROJECT_NAME} PRIVATE
    tcp_server
)
