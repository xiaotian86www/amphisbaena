cmake_minimum_required(VERSION 3.0.0)
project(http_server VERSION 0.1.0)

aux_source_directory(${PROJECT_SOURCE_DIR} SRC_LIST)

add_library(${PROJECT_NAME} SHARED ${SRC_LIST})

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION 0.1.0 SOVERSION 0)

target_link_libraries(${PROJECT_NAME} PRIVATE
    fmt
    llhttp_shared
    amphisbaena_core
    uds_server
    http_parser
)
