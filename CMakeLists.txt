cmake_minimum_required(VERSION 3.14)

project(ce CXX C)

set(CMAKE_CXX_STANDARD 23)

# Find the 64-bit system package
find_package(SDL3 REQUIRED CONFIG)

add_executable(ce src/main.cpp)

target_include_directories(ce PRIVATE src)

# Link SDL3
target_link_libraries(ce PRIVATE SDL3::SDL3)
