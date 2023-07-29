cmake_minimum_required(VERSION 3.0.0)
project(mock_build VERSION 0.1.0)

add_library(builder1 SHARED builder1.cpp)

target_link_libraries(builder1 PRIVATE
    translator_core)
    
add_library(builder11 SHARED builder11.cpp)

target_link_libraries(builder11 PRIVATE
    translator_core)
    
add_library(builder12 SHARED builder12.cpp)

target_link_libraries(builder12 PRIVATE
    translator_core)
    
add_library(builder13 SHARED builder13.cpp)

target_link_libraries(builder13 PRIVATE
    translator_core)
