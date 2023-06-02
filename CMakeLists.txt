
cmake_minimum_required(VERSION 3.5)

project(test)

set(CMAKE_CXX_STANDARD 17)

file(GLOB TEST_LIST test/*.cpp)

file(GLOB SRC_LIST src/*.cpp)

add_executable(test ${SRC_LIST} ${TEST_LIST})

target_include_directories(test PRIVATE ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(test gtest gmock gmock_main)
