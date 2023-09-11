cmake_minimum_required(VERSION 3.0.0)
project(mock_build VERSION 0.1.0)

add_library(plugin1 SHARED plugin1.cpp)

target_link_libraries(plugin1 PRIVATE
    http_message
    amphisbaena_core)
    
add_library(plugin11 SHARED plugin11.cpp)

target_link_libraries(plugin11 PRIVATE
    http_message
    amphisbaena_core)
