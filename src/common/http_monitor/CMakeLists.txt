cmake_minimum_required(VERSION 3.0.0)
project(http_monitor VERSION 0.1.0)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

aux_source_directory(${PROJECT_SOURCE_DIR} SRC_LIST)

add_library(${PROJECT_NAME} STATIC ${SRC_LIST})

target_link_libraries(${PROJECT_NAME} PRIVATE
)
