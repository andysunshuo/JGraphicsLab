
cmake_minimum_required(VERSION 3.20)

project(JGraphicsLab LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(JGraphicsLab WIN32
    src/main.cpp
    src/Renderer.cpp
    src/RasterWindow.h
)

target_compile_definitions(JGraphicsLab PRIVATE
    UNICODE
    _UNICODE
)

target_link_libraries(JGraphicsLab PRIVATE
    user32
    gdi32
)
