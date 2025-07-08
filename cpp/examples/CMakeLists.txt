cmake_minimum_required(VERSION 3.14)

# Demo serialization
add_executable(demo_serialization demo_serialization.cpp)
target_link_libraries(demo_serialization drawing_cpp)
target_include_directories(demo_serialization PRIVATE ../include)