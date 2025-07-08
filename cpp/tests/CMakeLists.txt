cmake_minimum_required(VERSION 3.14)

# Find or fetch Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/release-1.12.1.tar.gz
)
FetchContent_MakeAvailable(googletest)

# Test executable
add_executable(test_drawing
    test_types.cpp
    test_objects.cpp
    test_drawing.cpp
    test_performance.cpp
    test_serialization.cpp
    test_batch_operations.cpp
)

target_link_libraries(test_drawing
    drawing_cpp
    gtest_main
)

# Register tests
include(GoogleTest)
gtest_discover_tests(test_drawing)