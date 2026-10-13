#include "RasterWindow.h"
#include "geometry/OBJLoader.h"
#include "rasterizer/Transform.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <windows.h>

namespace
{
Raster::Color pixels[Raster::WIDTH * Raster::HEIGHT]{};
void Check(bool result, const char *message)
{
    std::cout << (result ? "PASS：" : "FAIL：") << message << '\n';
    if (!result)
        throw std::runtime_error(message);
}
} // namespace
// 验收使用与窗口相同的像素坐标约定，直接调用实际 RenderScene。
namespace Raster
{
Color MakeColor(int r, int g, int b)
{
    return ((r & 255u) << 16) | ((g & 255u) << 8) | (b & 255u);
}
void Clear(Color color)
{
    std::fill(std::begin(pixels), std::end(pixels), color);
}
void PutPixel(int x, int y, Color color)
{
    if (x >= 0 && y >= 0 && x < WIDTH && y < HEIGHT)
        pixels[(HEIGHT - 1 - y) * WIDTH + x] = color;
}
} // namespace Raster
void RenderScene();
int main()
{
    SetConsoleOutputCP(CP_UTF8);
    try
    {
        const auto triangle = OBJLoader::Load("assets/models/triangle.obj");
        Check(triangle.vertices.size() == 3 && triangle.indices.size() == 3, "当前三角形 OBJ 加载成功");
        const auto cube = OBJLoader::Load("assets/models/cube.obj");
        Check(cube.indices.size() == 36 && cube.vertices.size() == 24, "Cube 缺失 UV 的 1//1 格式保留法线硬边");
        const auto complex = OBJLoader::Load("assets/models/low_poly_icosahedron.obj");
        Check(complex.indices.size() == 54, "现有多面模型全部 18 个面均加载");
        Check(OBJLoader::Load("assets/models/不存在.obj").indices.empty(), "文件不存在时安全返回空 Mesh");
        const auto directory = std::filesystem::path(TEST_OUTPUT_DIR);
        std::filesystem::create_directories(directory);
        const auto fixture = directory / "fixture.obj";
        const auto write = [&](const std::string &text) {
            std::ofstream file(fixture, std::ios::binary);
            file << text;
            if (!file)
                throw std::runtime_error("无法写入测试输入");
        };
        write("v 0 0 0\nv 1 0 0\nv 1 1 0\nv 0 1 0\nf -4 -3 -2 -1\n");
        Check(OBJLoader::Load(fixture.string()).indices.size() == 6, "无 UV/法线的负索引四边形正确三角化");
        write("v 0 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 99\n");
        Check(OBJLoader::Load(fixture.string()).vertices.empty(), "非法位置索引拒绝越界读取");
        write("v 0 0 0\nv 1 0 0\nv 0 1 0\nvt 0 0\nf 1/99 2/1 3/1\n");
        Check(OBJLoader::Load(fixture.string()).vertices.empty(), "非法纹理索引拒绝越界读取");
        const auto screen = ToScreen(triangle.vertices[0].position, Raster::WIDTH, Raster::HEIGHT);
        Check(screen.x == 25 && screen.y == 25, "原左下原点屏幕变换保持一致");
        RenderScene();
        Check(pixels[(99 - 25) * 100 + 25] == Raster::MakeColor(255, 0, 0) &&
                  pixels[(99 - 25) * 100 + 75] == Raster::MakeColor(0, 255, 0) &&
                  pixels[(99 - 75) * 100 + 50] == Raster::MakeColor(0, 0, 255),
              "真实 RenderScene 产生预期的三个彩色顶点");
        Check(std::count_if(std::begin(pixels), std::end(pixels), [](auto color) { return color != 0; }) == 3,
              "保留当前顶点演示输出，没有改写成其他作业");
        std::ofstream frame(directory / "triangle.ppm", std::ios::binary);
        frame << "P6\n100 100\n255\n";
        for (auto color : pixels)
        {
            const char rgb[3] = {static_cast<char>(color >> 16), static_cast<char>(color >> 8),
                                 static_cast<char>(color)};
            frame.write(rgb, 3);
        }
        Check(static_cast<bool>(frame), "当前实际绘图结果成功导出");
        return 0;
    }
    catch (const std::exception &error)
    {
        std::cerr << "验收失败：" << error.what() << '\n';
        return 1;
    }
}
