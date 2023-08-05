cmake_minimum_required(VERSION 3.0.0)
project(http_parser VERSION 0.1.0)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

aux_source_directory(${PROJECT_SOURCE_DIR} SRC_LIST)

add_library(${PROJECT_NAME} STATIC ${SRC_LIST})

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION 0.1.0 SOVERSION 0)

target_link_libraries(${PROJECT_NAME} PRIVATE
    llhttp)
