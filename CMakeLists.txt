cmake_minimum_required(VERSION 3.10)

project(ce CXX C)

set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE
    SOURCE_FILES
    src/*.cpp
    src/*.hpp
)

# target_compile_options(your_target PRIVATE -save-temps=obj)
add_executable(ce ${SOURCE_FILES})

include_directories(src)
if(MINGW)
    # target_link_options(gb)
    target_compile_options(ce PRIVATE -std=c++23)
endif()

target_link_libraries(ce)