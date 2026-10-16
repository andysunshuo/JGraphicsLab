cmake_minimum_required(VERSION 3.20)
project(JGraphicsLab LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT WIN32)
    message(FATAL_ERROR "当前图形窗口需要 Windows")
endif()

# 程序和测试链接同一份基础管线，防止测试漏掉数学/相机源码或使用假的帧缓冲。
add_library(JRenderBasic STATIC
    src/application/Renderer.cpp src/geometry/OBJLoader.cpp
    src/assets/AssetPath.cpp src/material/Texture.cpp src/material/Material.cpp src/scene/Light.cpp
    src/math/Vec2.cpp src/math/Vec3.cpp src/math/Vec4.cpp
    src/math/Mat3.cpp src/math/Mat4.cpp src/math/Transform.cpp
    src/scene/Camera.cpp src/renderer/FrameBuffer.cpp
    src/renderer/Rasterizer.cpp src/renderer/TriangleRasterizer.cpp)
target_include_directories(JRenderBasic PUBLIC src PRIVATE third_party/tinyobjloader)
target_link_libraries(JRenderBasic PUBLIC windowscodecs ole32)
target_compile_definitions(JRenderBasic PUBLIC UNICODE _UNICODE NOMINMAX
    PRIVATE JGRAPHICSLAB_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets")
if(MSVC)
    target_compile_options(JRenderBasic PUBLIC /utf-8)
endif()

add_executable(JGraphicsLab WIN32 src/application/main.cpp)
target_link_libraries(JGraphicsLab PRIVATE JRenderBasic user32 gdi32 shell32)
file(GLOB_RECURSE JGRAPHICSLAB_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")
target_sources(JRenderBasic PRIVATE ${JGRAPHICSLAB_HEADERS})
get_target_property(JGRAPHICSLAB_SOURCES JRenderBasic SOURCES)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" PREFIX "源码" FILES ${JGRAPHICSLAB_SOURCES})
# 光照/材质/纹理进入共享库；原有光追空文件仅在 IDE 保留，没有调用其功能。
target_sources(JGraphicsLab PRIVATE src/raytracing/Ray.cpp src/raytracing/BVH.cpp)
add_custom_command(TARGET JGraphicsLab POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/assets" "$<TARGET_FILE_DIR:JGraphicsLab>/assets"
    VERBATIM)

include(CTest)
if(BUILD_TESTING)
    add_executable(OBJLoaderTests tests/OBJLoaderTests.cpp)
    target_link_libraries(OBJLoaderTests PRIVATE JRenderBasic)
    target_compile_definitions(OBJLoaderTests PRIVATE TEST_OUTPUT_DIR="${CMAKE_CURRENT_BINARY_DIR}/validation/$<CONFIG>")
    add_test(NAME OBJ与变换及光栅化 COMMAND OBJLoaderTests)
    add_test(NAME 实际窗口入口无显示渲染 COMMAND JGraphicsLab --headless --model cube --output "${CMAKE_CURRENT_BINARY_DIR}/cube-$<CONFIG>.ppm")
    add_executable(ShadingTests tests/ShadingTests.cpp)
    target_link_libraries(ShadingTests PRIVATE JRenderBasic)
    target_compile_definitions(ShadingTests PRIVATE TEST_OUTPUT_DIR="${CMAKE_CURRENT_BINARY_DIR}/shading-validation/$<CONFIG>")
    add_test(NAME 经典着色与材质贴图链路 COMMAND ShadingTests)
    add_test(NAME 配套房间OBJ_MTL_PNG渲染 COMMAND JGraphicsLab --headless --model room --lighting blinn-phong
        --output "${CMAKE_CURRENT_BINARY_DIR}/shading-validation/$<CONFIG>/room.png")
endif()
