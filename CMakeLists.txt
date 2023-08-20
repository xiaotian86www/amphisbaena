cmake_minimum_required(VERSION 3.0.0)
project(amphisbaena VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# set(CMAKE_CXX_VISIBILITY_PRESET hidden)

find_package(Boost 1.82.0 REQUIRED COMPONENTS context coroutine)

add_subdirectory(thirdparty/quickfix)
add_subdirectory(thirdparty/llhttp)
add_subdirectory(thirdparty/fmt)
add_subdirectory(thirdparty/googletest)
add_subdirectory(thirdparty/rapidjson)
add_subdirectory(thirdparty/spdlog)

include(CTest)
enable_testing()

if (MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

include_directories(${Boost_INCLUDE_DIRS})
include_directories(thirdparty/quickfix/include)
include_directories(thirdparty/llhttp/include)
include_directories(thirdparty/fmt/include)
include_directories(thirdparty/googletest/googletest/include)
include_directories(thirdparty/googletest/googlemock/include)
include_directories(thirdparty/rapidjson/include)
include_directories(thirdparty/spdlog/include)

include_directories(include)
include_directories(src)

add_subdirectory(src)
add_subdirectory(test)

add_executable(${PROJECT_NAME} application.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE amphisbaena_core)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
