cmake_minimum_required(VERSION 3.20)

project(MatOps VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(MATOPS_BUILD_TESTS "Build MatOps tests" ON)
option(MATOPS_BUILD_BENCHMARKS "Build MatOps benchmarks" ON)
set(MATOPS_GEMM_MC 64 CACHE STRING "GEMM M cache block size")
set(MATOPS_GEMM_NC 512 CACHE STRING "GEMM N cache block size")
set(MATOPS_GEMM_KC 256 CACHE STRING "GEMM K cache block size")

foreach(block_var IN ITEMS MATOPS_GEMM_MC MATOPS_GEMM_NC MATOPS_GEMM_KC)
    if(NOT "${${block_var}}" MATCHES "^[1-9][0-9]*$")
        message(FATAL_ERROR "${block_var} must be a positive integer")
    endif()
endforeach()

math(EXPR MATOPS_GEMM_MC_REMAINDER "${MATOPS_GEMM_MC} % 4")
math(EXPR MATOPS_GEMM_NC_REMAINDER "${MATOPS_GEMM_NC} % 4")
if(NOT MATOPS_GEMM_MC_REMAINDER EQUAL 0 OR
   NOT MATOPS_GEMM_NC_REMAINDER EQUAL 0)
    message(FATAL_ERROR "MATOPS_GEMM_MC and MATOPS_GEMM_NC must be multiples of 4")
endif()

add_library(matops_core STATIC
    src/matrix.cpp
    src/gemm/gemm_dispatch.cpp
    src/gebp/kernels/SIMD_kernel_float.cpp
    src/runtime/cpu_features.cpp
    src/runtime/thread_pool.cpp
)

target_include_directories(matops_core
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_compile_features(matops_core PUBLIC cxx_std_20)
target_compile_definitions(matops_core
    PUBLIC
        MATOPS_GEMM_MC=${MATOPS_GEMM_MC}
        MATOPS_GEMM_NC=${MATOPS_GEMM_NC}
        MATOPS_GEMM_KC=${MATOPS_GEMM_KC}
)

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

    find_package(GTest CONFIG QUIET)
    if(NOT GTest_FOUND)
        include(FetchContent)
        set(BUILD_GMOCK OFF CACHE BOOL "Build GoogleMock" FORCE)
        FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.17.0
            GIT_SHALLOW TRUE
        )
        FetchContent_MakeAvailable(googletest)
    endif()

    include(GoogleTest)

    add_executable(matops_tests
        tests/smoke.cpp
        tests/gebp_test.cpp
        tests/gemm_test.cpp
    )
    target_link_libraries(matops_tests PRIVATE matops_core GTest::gtest_main)
    target_include_directories(matops_tests PRIVATE ${PROJECT_SOURCE_DIR}/src)
    matops_enable_warnings(matops_tests)
    gtest_discover_tests(matops_tests DISCOVERY_MODE PRE_TEST)
endif()

if(MATOPS_BUILD_BENCHMARKS)
    add_executable(matops_benchmarks
        benchmarks/gemm_benchmark.cpp
    )
    target_link_libraries(matops_benchmarks PRIVATE matops_core)
    target_include_directories(matops_benchmarks PRIVATE ${PROJECT_SOURCE_DIR}/src)
    matops_enable_warnings(matops_benchmarks)
endif()
