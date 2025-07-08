cmake_minimum_required(VERSION 3.14)

# Google Benchmark
include(FetchContent)
FetchContent_Declare(
    benchmark
    URL https://github.com/google/benchmark/archive/v1.8.0.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Disable benchmark tests")
FetchContent_MakeAvailable(benchmark)

# C++ Benchmarks
add_executable(benchmark_drawing
    benchmark_main.cpp
)

target_link_libraries(benchmark_drawing
    drawing_cpp
    benchmark::benchmark
)

# Set optimization flags
target_compile_options(benchmark_drawing PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-O3>
)