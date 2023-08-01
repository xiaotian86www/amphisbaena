cmake_minimum_required(VERSION 3.0.0)
project(amphisbaena_core VERSION 0.1.0)

add_subdirectory(plugin)

find_package(Boost REQUIRED)

aux_source_directory(${PROJECT_SOURCE_DIR} SRC_LIST)
aux_source_directory(${PROJECT_SOURCE_DIR}/impl SRC_LIST)

add_library(${PROJECT_NAME} SHARED ${SRC_LIST})

target_link_libraries(${PROJECT_NAME} PRIVATE
    boost_context
    boost_coroutine
    llhttp
    quickfix)
