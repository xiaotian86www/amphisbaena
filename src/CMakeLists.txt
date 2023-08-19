cmake_minimum_required(VERSION 3.0.0)
project(amphisbaena_core VERSION 0.1.0)

add_subdirectory(plugin)
add_subdirectory(common)

find_package(Boost 1.82.0 REQUIRED COMPONENTS context coroutine)

aux_source_directory(${PROJECT_SOURCE_DIR} SRC_LIST)

add_library(${PROJECT_NAME} SHARED ${SRC_LIST})

target_link_libraries(${PROJECT_NAME} PRIVATE
    Boost::context
    Boost::coroutine
)
