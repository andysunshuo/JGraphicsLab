cmake_minimum_required(VERSION 3.20)
project(JGraphicsLab LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT WIN32)
    message(FATAL_ERROR "当前图形窗口需要 Windows")
endif()

add_executable(JGraphicsLab WIN32
    src/main.cpp src/Renderer.cpp src/RasterWindow.h src/core/Vec3.h
    src/geometry/Mesh.h src/geometry/OBJLoader.h src/geometry/OBJLoader.cpp
    src/rasterizer/Vertex.h src/rasterizer/Transform.h
    src/rasterizer/TriangleRasterizer.h src/rasterizer/TriangleRasterizer.cpp)
target_include_directories(JGraphicsLab PRIVATE src third_party/tinyobjloader)
target_compile_definitions(JGraphicsLab PRIVATE UNICODE _UNICODE NOMINMAX
    JGRAPHICSLAB_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets")
if(MSVC)
    target_compile_options(JGraphicsLab PRIVATE /utf-8)
endif()
target_link_libraries(JGraphicsLab PRIVATE user32 gdi32)
# exe 旁部署资源，消除对 Visual Studio 启动工作目录的依赖。
add_custom_command(TARGET JGraphicsLab POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets" "$<TARGET_FILE_DIR:JGraphicsLab>/assets"
    VERBATIM)

include(CTest)
if(BUILD_TESTING)
    add_executable(OBJLoaderTests tests/OBJLoaderTests.cpp src/geometry/OBJLoader.cpp
        src/Renderer.cpp src/rasterizer/TriangleRasterizer.cpp)
    target_include_directories(OBJLoaderTests PRIVATE src third_party/tinyobjloader)
    target_compile_definitions(OBJLoaderTests PRIVATE UNICODE _UNICODE NOMINMAX
        JGRAPHICSLAB_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets"
        TEST_OUTPUT_DIR="${CMAKE_CURRENT_BINARY_DIR}/validation/$<CONFIG>")
    if(MSVC)
        target_compile_options(OBJLoaderTests PRIVATE /utf-8)
    endif()
    add_test(NAME OBJ加载与场景绘制 COMMAND OBJLoaderTests)
endif()
