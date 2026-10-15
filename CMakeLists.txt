cmake_minimum_required(VERSION 3.14)

project(ce CXX C)

set(CMAKE_CXX_STANDARD 23)

find_package(SDL3 REQUIRED CONFIG)

add_executable(ce
    src/main.cpp
    src/display.cpp
)

target_include_directories(ce PRIVATE src)

target_link_libraries(ce PRIVATE SDL3::SDL3)

add_custom_command(
    TARGET ce POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:ce>/assets"
    COMMENT "Copying assets to build directory"
)
