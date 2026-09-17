cmake_minimum_required(VERSION 3.20)

project(MatOps VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(matops
    src/main.cpp
)

target_compile_features(matops PRIVATE cxx_std_20)

if(MSVC)
    target_compile_options(matops PRIVATE /W4 /permissive-)
else()
    target_compile_options(matops PRIVATE -Wall -Wextra -Wpedantic)
endif()
