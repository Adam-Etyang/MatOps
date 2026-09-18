cmake_minimum_required(VERSION 3.20)

project(MatOps VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(MATOPS_BUILD_TESTS "Build MatOps tests" ON)
option(MATOPS_BUILD_BENCHMARKS "Build MatOps benchmarks" ON)

add_library(matops_core STATIC
    src/matrix.cpp
    src/gemm/gemm_dispatch.cpp
    src/runtime/cpu_features.cpp
    src/runtime/thread_pool.cpp
)

target_include_directories(matops_core
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_compile_features(matops_core PUBLIC cxx_std_20)

add_executable(matops
    src/main.cpp
)

target_link_libraries(matops PRIVATE matops_core)

function(matops_enable_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive-)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

matops_enable_warnings(matops_core)
matops_enable_warnings(matops)

if(MATOPS_BUILD_TESTS)
    enable_testing()

    add_executable(matops_tests
        tests/smoke.cpp
    )
    target_link_libraries(matops_tests PRIVATE matops_core)
    matops_enable_warnings(matops_tests)
    add_test(NAME matops.smoke COMMAND matops_tests)
endif()

if(MATOPS_BUILD_BENCHMARKS)
    add_executable(matops_benchmarks
        benchmarks/gemm_benchmark.cpp
    )
    target_link_libraries(matops_benchmarks PRIVATE matops_core)
    matops_enable_warnings(matops_benchmarks)
endif()
