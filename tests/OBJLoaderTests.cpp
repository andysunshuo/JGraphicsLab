#include "application/Renderer.h"
#include "geometry/OBJLoader.h"
#include "renderer/Rasterizer.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>
#include <windows.h>

namespace
{
void Check(bool ok, const char *message)
{
    std::cout << (ok ? "PASS：" : "FAIL：") << message << '\n';
    if (!ok)
        throw std::runtime_error(message);
}
bool Near(float a, float b)
{
    return std::isfinite(a) && std::abs(a - b) < 1e-4f;
}
std::size_t Visible(Raster::Color background)
{
    return std::count_if(Raster::Pixels(), Raster::Pixels() + Raster::WIDTH * Raster::HEIGHT,
                         [&](auto c) { return c != background; });
}
std::uint64_t Hash()
{
    std::uint64_t hash = 14695981039346656037ull;
    for (int i = 0; i < Raster::WIDTH * Raster::HEIGHT; ++i)
    {
        hash ^= Raster::Pixels()[i];
        hash *= 1099511628211ull;
    }
    return hash;
}
ScreenVertex V(float x, float y, float z, Vec3 color)
{
    ScreenVertex v;
    v.x = x;
    v.y = y;
    v.ndcZ = z;
    v.color = color;
    return v;
}
} // namespace
int main()
{
    SetConsoleOutputCP(CP_UTF8);
    try
    {
        const auto output = std::filesystem::path(TEST_OUTPUT_DIR);
        std::filesystem::create_directories(output);
        Check(Raster::WIDTH == 1000 && Raster::HEIGHT == 1000, "实际颜色与深度缓冲均为 1000×1000");
        const auto translated = Transform::Translation({4, 5, 6}) * Vec4{1, 2, 3, 1};
        Check(Near(translated.x, 5) && Near(translated.y, 7) && Near(translated.z, 9) && Near(translated.w, 1),
              "Translation 接收 Identity 返回值且保留齐次 w");
        const auto model = Transform::Model({0, 0, 0}, {0, 0, 0}, {1, 1, 1});
        const auto unchanged = model * Vec4{1, 2, 3, 1};
        Check(Near(unchanged.x, 1) && Near(unchanged.y, 2) && Near(unchanged.z, 3) && Near(unchanged.w, 1),
              "零旋转单位缩放保持恒等变换");
        const auto rx = Transform::RotationX(0) * Vec4{1, 2, 3, 1}, ry = Transform::RotationY(0) * Vec4{1, 2, 3, 1},
                   rz = Transform::RotationZ(0) * Vec4{1, 2, 3, 1};
        Check(Near(rx.x, 1) && Near(ry.y, 2) && Near(rz.z, 3), "各轴旋转保留未参与旋转的单位对角线");
        Camera camera;
        const auto origin = camera.ViewMatrix() * Vec4{0, 0, 0, 1};
        Check(Near(origin.z, -3) && Near(origin.w, 1), "LookAt 保留最后一行，世界原点位于相机前方");
        Vec3 nearNdc{}, farNdc{};
        Check((camera.ProjectionMatrix() * Vec4{0, 0, -camera.nearPlane, 1}).TryPerspectiveDivide(nearNdc) &&
                  (camera.ProjectionMatrix() * Vec4{0, 0, -camera.farPlane, 1}).TryPerspectiveDivide(farNdc) &&
                  Near(nearNdc.z, -1) && Near(farNdc.z, 1),
              "透视投影近远面映射正确");
        ClipVertex up;
        up.position = {0, .5f, 0, 1};
        ScreenVertex screen;
        Check(Transform::ToScreen(up, 1000, 1000, screen) && Near(screen.x, 500) && Near(screen.y, 750),
              "视口 Y 向上，没有双重翻转");
        bool invalid = false;
        try
        {
            Transform::Orthographic(0, 0, -1, 1, .1f, 10);
        }
        catch (const std::invalid_argument &)
        {
            invalid = true;
        }
        Check(invalid, "正交投影拒绝零宽度范围");

        const auto triangle = OBJLoader::Load("assets/models/triangle.obj");
        Check(triangle.vertices.size() == 3 && triangle.indices.size() == 3, "三角形 OBJ 加载");
        const auto cube = OBJLoader::Load("assets/models/cube.obj");
        Check(cube.indices.size() == 36 && cube.vertices.size() == 24, "Cube 的独立法线与缺失 UV 正确加载");
        Check(OBJLoader::Load("assets/models/low_poly_icosahedron.obj").indices.size() == 54, "多面模型完整加载");
        Check(OBJLoader::Load("assets/models/不存在.obj").indices.empty(), "缺失文件安全失败");
        const auto fixture = output / "fixture.obj";
        const auto write = [&](const std::string &s) {
            std::ofstream f(fixture, std::ios::binary);
            f << s;
            if (!f)
                throw std::runtime_error("测试输入写入失败");
        };
        write("v 0 0 0\nv 1 0 0\nv 1 1 0\nv 0 1 0\nf -4 -3 -2 -1\n");
        Check(OBJLoader::Load(fixture.string()).indices.size() == 6, "负索引四边形三角化");
        write("v 0 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 99\n");
        Check(OBJLoader::Load(fixture.string()).vertices.empty(), "非法位置索引安全拒绝");
        write("v 0 0 0\nv 1 0 0\nv 0 1 0\nvt 0 0\nf 1/99 2/1 3/1\n");
        Check(OBJLoader::Load(fixture.string()).vertices.empty(), "非法 UV 索引安全拒绝");

        Raster::Clear(0);
        Raster::ClearDepth();
        RasterizeTriangle(V(16, 16, .6f, {0, 0, 1}), V(96, 16, .6f, {0, 0, 1}), V(16, 96, .6f, {0, 0, 1}));
        RasterizeTriangle(V(16, 16, -.4f, {1, 0, 0}), V(96, 16, -.4f, {1, 0, 0}), V(16, 96, -.4f, {1, 0, 0}));
        RasterizeTriangle(V(16, 16, .6f, {0, 0, 1}), V(96, 16, .6f, {0, 0, 1}), V(16, 96, .6f, {0, 0, 1}));
        Check(Raster::GetPixel(32, 32) == Raster::MakeColor(255, 0, 0) && Near(Raster::GetDepth(32, 32), .3f),
              "填充内部像素，近面遮挡远面且绘制顺序不影响深度");
        Check(Visible(0) > 3000, "三角形生成面积，而非只有三个点");
        const auto before = Hash();
        RasterizeTriangle(V(20, 20, 0, {1, 0, 0}), V(20, 20, 0, {1, 0, 0}), V(20, 20, 0, {1, 0, 0}));
        auto nan = V(0, 0, 0, {1, 0, 0});
        nan.x = std::numeric_limits<float>::quiet_NaN();
        RasterizeTriangle(nan, V(100, 0, 0, {1, 0, 0}), V(0, 100, 0, {1, 0, 0}));
        Check(Hash() == before, "退化与非有限三角形不写入缓冲");
        Raster::Clear(0);
        Raster::ClearDepth();
        RasterizeTriangle(V(0, 0, 0, {1, 1, 1}), V(64, 0, 0, {1, 1, 1}), V(64, 64, 0, {1, 1, 1}));
        std::vector<bool> first(4096);
        for (int y = 0; y < 64; ++y)
            for (int x = 0; x < 64; ++x)
                first[y * 64 + x] = Raster::GetPixel(x, y) != 0;
        Raster::Clear(0);
        Raster::ClearDepth();
        RasterizeTriangle(V(0, 0, 0, {1, 1, 1}), V(64, 64, 0, {1, 1, 1}), V(0, 64, 0, {1, 1, 1}));
        int overlap = 0, covered = 0;
        for (int y = 0; y < 64; ++y)
            for (int x = 0; x < 64; ++x)
            {
                const bool second = Raster::GetPixel(x, y) != 0;
                overlap += second && first[y * 64 + x];
                covered += second || first[y * 64 + x];
            }
        Check(overlap == 0 && covered == 4096, "共享边既不重复覆盖也不漏像素");

        ClipVertex a, b, c;
        a.position = {-.6f, -.6f, -2, 1};
        b.position = {.7f, -.6f, 0, 1};
        c.position = {0, .7f, 0, 1};
        const auto polygon = ClipTriangleToFrustum(a, b, c);
        bool inside = polygon.size() == 4;
        for (const auto &v : polygon)
            for (int plane = 0; plane < 6; ++plane)
                inside &= EvaluateClipPlane(v.position, plane) >= -1e-5f;
        Check(inside, "近面相交三角形被真正裁成四边形");
        Mesh bad = triangle;
        bad.indices[2] = 999;
        invalid = false;
        try
        {
            DrawMeshBasic(bad, Mat4::Identity(), camera);
        }
        catch (const std::invalid_argument &)
        {
            invalid = true;
        }
        Check(invalid, "绘制接口检查非法 Mesh 索引");
        Mesh crossing;
        crossing.vertices = {{{-.08f, -.06f, -.05f}, {}, {}}, {{.7f, -.6f, -1}, {}, {}}, {{0, .7f, -1}, {}, {}}};
        crossing.indices = {0, 1, 2};
        Camera nearCamera;
        nearCamera.eye = {0, 0, 0};
        nearCamera.target = {0, 0, -1};
        Raster::Clear(0);
        Raster::ClearDepth();
        DrawMeshBasic(crossing, Mat4::Identity(), nearCamera, false);
        Check(Visible(0) > 1000, "近面相交几何在完整 Mesh 管线中仍可见");
        Check(Raster::SavePPM((output / "near-clipping.ppm").string().c_str()), "裁剪结果导出");
        for (const char *name : {"cube", "triangle", "low_poly_icosahedron"})
        {
            RenderScene(std::string("assets/models/") + name + ".obj");
            Check(Visible(Raster::MakeColor(18, 22, 30)) > 10000, "真实应用管线渲染 OBJ 主体");
            Check(Raster::SavePPM((output / (std::string(name) + ".ppm")).string().c_str()), "导出真实 1000×1000 画面");
        }
        write("v -.5 -.5 0\nv .5 -.5 0\nv 0 .5 0\nf 1 2 3\n");
        RenderScene(fixture.string());
        const auto cached = Hash();
        write("");
        RenderScene(fixture.string());
        Check(Hash() == cached, "重复绘制使用缓存，即使磁盘测试副本改变也不会重新解析");
        std::cout << "全部基础管线验收通过\n";
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "验收失败：" << e.what() << '\n';
        return 1;
    }
}
