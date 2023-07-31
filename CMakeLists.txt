cmake_minimum_required(VERSION 3.0.0)
project(amphisbaena VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_VISIBILITY_PRESET hidden)

include(CTest)
enable_testing()

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/src)

add_subdirectory(src)
add_subdirectory(test)

add_executable(${PROJECT_NAME} application.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE amphisbaena_core)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
