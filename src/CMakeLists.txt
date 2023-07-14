cmake_minimum_required(VERSION 3.0.0)
project(translator VERSION 0.1.0)

find_package(Boost REQUIRED)

aux_source_directory(${PROJECT_SOURCE_DIR} SRC_LIST)
aux_source_directory(${PROJECT_SOURCE_DIR}/detail SRC_LIST)

add_library(${PROJECT_NAME} ${SRC_LIST})

target_link_libraries(${PROJECT_NAME}
    boost_context
    boost_coroutine
    llhttp
    quickfix)
