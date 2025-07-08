cmake_minimum_required(VERSION 3.14)

# Find Python and pybind11
find_package(Python COMPONENTS Interpreter Development REQUIRED)

# Try to find pybind11 installed, otherwise fetch it
find_package(pybind11 QUIET)
if(NOT pybind11_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        pybind11
        URL https://github.com/pybind/pybind11/archive/v2.11.1.tar.gz
    )
    FetchContent_MakeAvailable(pybind11)
endif()

# Python module
pybind11_add_module(drawing_cpp_py 
    bindings.cpp
)

# Link with our C++ library
target_link_libraries(drawing_cpp_py PRIVATE drawing_cpp)

# Set properties
set_target_properties(drawing_cpp_py PROPERTIES
    OUTPUT_NAME "drawing_cpp"
    POSITION_INDEPENDENT_CODE ON
)

# Install the Python module
install(TARGETS drawing_cpp_py
    LIBRARY DESTINATION ${Python_SITELIB}
)