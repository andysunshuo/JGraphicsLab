cmake_minimum_required(VERSION 3.20)
project(JGraphicsLab LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT WIN32)
    message(FATAL_ERROR "当前窗口与纹理解码模块需要 Windows")
endif()
add_library(JRenderLab STATIC
    src/Application/Application.cpp
    src/Application/ModelDemos.cpp
    src/Assets/HdrLoader.cpp
    src/Assets/ObjLoader.cpp
    src/Assets/ResourcePaths.cpp
    src/Assets/TextureLoader.cpp
    src/Camera/Camera.cpp
    src/Camera/Transforms.cpp
    src/Core/Color.cpp
    src/Core/Math.cpp
    src/Geometry/Geometry.cpp
    src/Geometry/Mesh.cpp
    src/Geometry/Surface.cpp
    src/Homework/DemoData.cpp
    src/Homework/HW01.cpp
    src/Homework/HW02.cpp
    src/Homework/HW03.cpp
    src/Homework/HW04.cpp
    src/Homework/HW05.cpp
    src/Homework/HW06.cpp
    src/Homework/HW07.cpp
    src/Homework/HW08.cpp
    src/Homework/HW09.cpp
    src/Homework/HW10.cpp
    src/Homework/HW11.cpp
    src/Homework/HW12.cpp
    src/Homework/HW13.cpp
    src/Homework/HW14.cpp
    src/Homework/HW15.cpp
    src/Homework/HW16.cpp
    src/Homework/HW17.cpp
    src/Homework/HW18.cpp
    src/Homework/HW19.cpp
    src/Homework/HW20.cpp
    src/Homework/HW21.cpp
    src/Homework/HW22.cpp
    src/Homework/HW23.cpp
    src/Homework/HW24.cpp
    src/Homework/HW25.cpp
    src/Homework/HW26.cpp
    src/Homework/HW27.cpp
    src/Homework/HW28.cpp
    src/Homework/HW29.cpp
    src/Homework/HW30.cpp
    src/Homework/HW31.cpp
    src/Homework/HW32.cpp
    src/Homework/HW33.cpp
    src/Homework/HW34.cpp
    src/Homework/HW35.cpp
    src/Homework/HW36.cpp
    src/Homework/HW37.cpp
    src/Homework/HW38.cpp
    src/Homework/HW39.cpp
    src/Homework/HW40.cpp
    src/Homework/HW41.cpp
    src/Homework/HW42.cpp
    src/Homework/HW43.cpp
    src/Homework/HW44.cpp
    src/Homework/HW45.cpp
    src/Homework/HW46.cpp
    src/Homework/HW47.cpp
    src/Homework/HW48.cpp
    src/Homework/HW49.cpp
    src/Homework/AssetHomework.cpp
    src/Homework/HW50_1.cpp
    src/Homework/HW50_2.cpp
    src/Homework/HW50_3.cpp
    src/Homework/HW50_4.cpp
    src/Homework/HW50_5.cpp
    src/Homework/HW50_6.cpp
    src/Homework/HW50_7.cpp
    src/Homework/HW50_8.cpp
    src/Homework/HW50_9.cpp
    src/Homework/HW50_10.cpp
    src/Homework/HW50.cpp
    src/Homework/Registry.cpp
    src/Homework/Runner.cpp
    src/Homework/LearningDemos.cpp
    src/Homework/TestSupport.cpp
    src/Renderer/AssetPreview.cpp
    src/Renderer/Clipping.cpp
    src/Renderer/DepthBuffer.cpp
    src/Renderer/Framebuffer.cpp
    src/Renderer/Interpolation.cpp
    src/Renderer/MSAA.cpp
    src/Renderer/ImageProcessing.cpp
    src/Renderer/LearningPipeline.cpp
    src/Renderer/MeshRenderer.cpp
    src/Renderer/Rasterizer2D.cpp
    src/Renderer/Rasterizer3D.cpp
)
target_include_directories(JRenderLab PUBLIC src PRIVATE third_party/tinyobjloader third_party/stb)
get_target_property(JRENDERLAB_SOURCES JRenderLab SOURCES)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" PREFIX "源码" FILES ${JRENDERLAB_SOURCES})
target_compile_definitions(JRenderLab PUBLIC UNICODE _UNICODE NOMINMAX PRIVATE JRENDERLAB_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/Assets")
if(MSVC)
    target_compile_options(JRenderLab PUBLIC /utf-8)
endif()
target_link_libraries(JRenderLab PUBLIC windowscodecs ole32 gdi32)
add_executable(JGraphicsLab WIN32 src/Application/RasterWindow.cpp)
target_link_libraries(JGraphicsLab PRIVATE JRenderLab user32 gdi32 shell32)
add_executable(JRenderLabCLI src/Application/ConsoleMain.cpp)
target_link_libraries(JRenderLabCLI PRIVATE JRenderLab)
include(CTest)
if(BUILD_TESTING)
    add_executable(JRenderLabTests tests/IntegrationTests.cpp)
    target_link_libraries(JRenderLabTests PRIVATE JRenderLab)
    target_include_directories(JRenderLabTests PRIVATE tests)
    target_compile_definitions(JRenderLabTests PRIVATE JRENDERLAB_TEST_CONFIG="$<CONFIG>")
    add_test(NAME 数学与调度及回归 COMMAND JRenderLabTests --core)
    add_test(NAME Cube与Teapot COMMAND JRenderLabTests --models-small)
    add_test(NAME Bunny与Dragon COMMAND JRenderLabTests --models-large)
    add_executable(AssetHomeworkTests tests/AssetHomeworkTests.cpp)
    target_link_libraries(AssetHomeworkTests PRIVATE JRenderLab)
    target_compile_definitions(AssetHomeworkTests PRIVATE JRENDERLAB_TEST_CONFIG="$<CONFIG>")
    add_test(NAME HW50资源展示 COMMAND AssetHomeworkTests)
    add_executable(AdvancedHomeworkTests tests/AdvancedHomeworkTests.cpp)
    target_link_libraries(AdvancedHomeworkTests PRIVATE JRenderLab)
    target_compile_definitions(AdvancedHomeworkTests PRIVATE JRENDERLAB_TEST_CONFIG="$<CONFIG>")
    add_test(NAME HW32至HW50完整作业 COMMAND AdvancedHomeworkTests)
endif()
# 部署时显式复制资源，开发构建使用源目录回退，不依赖 VS 工作目录。
install(TARGETS JGraphicsLab JRenderLabCLI RUNTIME DESTINATION .)
install(DIRECTORY Assets DESTINATION .)
