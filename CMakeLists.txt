
cmake_minimum_required(VERSION 3.5)

project(test)

set(SRC_LIST
    src/coroutine.cpp
    test/main.cpp
    test/test_coroutine.cpp
)

add_executable(test ${SRC_LIST})

target_include_directories(test PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(test gtest gmock)
