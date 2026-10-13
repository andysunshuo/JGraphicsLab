#include "Application/ModelDemos.h"
#include "Assets/ResourcePaths.h"
#include "Assets/TextureLoader.h"
#include "Homework/Runner.h"
#include "Renderer/DepthBuffer.h"
#include "Renderer/Framebuffer.h"
#include "GoldenFrames.h"
#include <windows.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <limits>
#include <set>
#include <stdexcept>

namespace
{
int failures = 0;
void Check(bool passed, const char *description)
{
    std::printf("%s：%s\n", passed ? "PASS" : "FAIL", description);
    if (!passed)
        ++failures;
}
std::uint64_t FrameHash()
{
    std::uint64_t value = 14695981039346656037ull;
    const auto append = [&](const void *data, std::size_t size) {
        const auto *bytes = static_cast<const unsigned char *>(data);
        for (std::size_t i = 0; i < size; ++i)
        {
            value ^= bytes[i];
            value *= 1099511628211ull;
        }
    };
    append(Raster::Pixels(), Raster::WIDTH * Raster::HEIGHT * sizeof(Raster::Color));
    append(depthBuffer, sizeof(depthBuffer));
    return value;
}
std::size_t VisiblePixels()
{
    std::size_t count = 0;
    for (int i = 0; i < Raster::WIDTH * Raster::HEIGHT; ++i)
        if (Raster::Pixels()[i])
            ++count;
    return count;
}
void Write(const std::filesystem::path &path, const std::string &text)
{
    std::ofstream file(path, std::ios::binary);
    file << text;
    if (!file)
        throw std::runtime_error("无法保存测试输入");
}
void CoreTests(const std::filesystem::path &output)
{
    const auto math = RunAllMathTests();
    Check(math.failed == 0 && math.passed > 0, "所有原有数学测试通过");
    std::set<int> ids;
    for (const auto &hw : HomeworkRegistry())
    {
        ids.insert(hw.number);
        Check(FindHomework(hw.number) == &hw, "注册表与 switch 对应唯一条目");
        if (hw.status == HomeworkStatus::NotImplemented)
            Check(!hw.test && hw.render, "未实现作业有独立入口但没有伪造测试");
        else
            Check(hw.test || hw.render, "已有作业拥有可运行入口");
    }
    Check(ids.size() == 50 && *ids.begin() == 1 && *ids.rbegin() == 50, "HW01～HW50 编号无冲突");
    Check(!FindHomework(-1) && !FindHomework(0) && !FindHomework(51), "越界编号不映射到作业");
    for (int hw = 2; hw <= 31; ++hw)
    {
        Check(RunHomework(hw), "逐编号运行已有作业");
        const auto hash = FrameHash();
        std::printf("HW%02d 帧缓冲与深度哈希：%llu，原始基准：%llu\n", hw,
                    static_cast<unsigned long long>(hash), static_cast<unsigned long long>(golden[hw - 2]));
        Check(hash == golden[hw - 2], "与重构前像素及深度逐字节基准一致");
        std::ofstream snapshot(output / ("hw" + std::to_string(hw) + ".bin"),std::ios::binary);
        snapshot.write(reinterpret_cast<const char*>(Raster::Pixels()),Raster::WIDTH*Raster::HEIGHT*sizeof(Raster::Color));
        snapshot.write(reinterpret_cast<const char*>(depthBuffer),sizeof(depthBuffer));
        if (hw >= 29)
            Check(Raster::SavePPM(output / ("hw" + std::to_string(hw) + ".ppm")), "保存教学回归图像");
    }
    Check(!RunHomework(1), "唯一缺失的 HW01 明确返回未实现");
    const auto fixture = output / L"独立索引.obj";
    Write(fixture, "v 0 0 0\nv 1 0 0\nv 0 1 0\nvt 0 0\nvt 1 0\nvt 0 1\nvn 0 0 1\nvn 0 0 -1\nf 1/1/1 2/2/1 "
                   "3/3/1\nf -3/2/2 -1/3/2 -2/1/2\n");
    const auto seam = Assets::LoadObj(fixture);
    Check(static_cast<bool>(seam) && seam.mesh.vertices.size() == 6 && seam.mesh.indices.size() == 6,
          "独立位置、UV、法线组合与负索引映射正确");
    if (seam)
    {
        const auto &a = seam.mesh.vertices[0];
        const auto &b = seam.mesh.vertices[3];
        Check(a.position.x == b.position.x && a.uv.x != b.uv.x && a.normal.z == -b.normal.z,
              "共享位置仍保留 UV 接缝与法线差异");
    }
    Write(fixture, "v 0 0 0\nv 1 0 0\nv 1 1 0\nv 0 1 0\nf 1 2 3 4\n");
    const auto quad = Assets::LoadObj(fixture);
    Check(static_cast<bool>(quad) && quad.mesh.indices.size() == 6,
          "缺失 UV 和法线的四边形由 tinyobjloader 三角化");
    for (const std::string &face : {"f 1 2 99\n", "f 1/99 2/99 3/99\n", "f 1//99 2//99 3//99\n"})
    {
        Write(fixture, "v 0 0 0\nv 1 0 0\nv 0 1 0\n" + face);
        const auto bad = Assets::LoadObj(fixture);
        Check(!bad && !bad.error.empty() && bad.mesh.vertices.empty(),
              "非法 OBJ 索引返回明确错误并清空部分网格");
    }
    Check(!Assets::LoadObj(output / L"不存在.obj"), "模型文件不存在时不崩溃");
    Assets::LoadLimits limits;
    limits.maxFileBytes = 8;
    Check(!Assets::LoadObj(fixture, limits), "模型文件超过预算时拒绝加载");
    Write(fixture, "v 0 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 3\n");
    limits = {};
    limits.maxTriangles = 0;
    Check(!Assets::LoadObj(fixture, limits), "解析前检查三角形数量预算");
    limits = {};
    limits.maxVertices = 2;
    Check(!Assets::LoadObj(fixture, limits), "统一顶点数量受到预算限制");
    Write(fixture, "v 1e40 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 3\n");
    Check(!Assets::LoadObj(fixture), "非有限顶点拒绝进入网格");
    Mesh mesh;
    mesh.vertices = {{{0, 0, 0}, {}, {}}, {{1, 0, 0}, {}, {}}, {{0, 1, 0}, {}, {}}};
    mesh.indices = {0, 1, 99, 0, 0, 0};
    Check(DrawMesh(mesh, IdentityMat4(), Camera{}).rejectedTriangles == 2,
          "通用渲染拒绝非法索引和退化三角形");
    mesh.indices = {0, 1, 2};
    mesh.vertices[0].position.x = std::numeric_limits<float>::quiet_NaN();
    Check(DrawMesh(mesh, IdentityMat4(), Camera{}).rejectedTriangles == 1, "通用渲染拒绝非有限坐标");
    mesh.vertices[0].position = {0, 0, 0};
    const Mat4 giant = MakeTranslation3D(1e30f, 1e30f, 1e30f);
    Check(DrawMesh(mesh, giant, Camera{}).rejectedTriangles == 1, "巨大模型变换不会导致屏幕整数转换越界");
    Assets::Texture texture;
    std::string error;
    Check(Assets::LoadTexture(Assets::Resolve("Textures/checker.png"), texture, error) && texture.width &&
              texture.height &&
              texture.pixels.size() == static_cast<std::size_t>(texture.width) * texture.height,
          "PNG 纹理加载与尺寸检查");
    Check(Assets::LoadTexture(Assets::Resolve("Textures/wood.jpg"), texture, error), "JPEG 纹理加载");
    Check(!Assets::LoadTexture(output / L"不存在.png", texture, error) && !error.empty(),
          "纹理加载失败返回中文错误");
}
void ModelTests(bool large, const std::filesystem::path &output)
{
    ModelDemoCache cache;
    std::ofstream timings(output / (large ? L"大模型.csv" : L"小模型.csv"));
    timings << "模型,顶点数,三角形数,加载毫秒,首帧毫秒,后三帧平均毫秒,可见像素,加载次数\n";
    const std::array<ModelDemo, 2> models = large ? std::array{ModelDemo::Bunny, ModelDemo::Dragon}
                                                  : std::array{ModelDemo::Cube, ModelDemo::Teapot};
    for (const auto demo : models)
    {
        const auto &loaded = cache.Get(demo);
        Check(static_cast<bool>(loaded), "OBJ 模型成功加载");
        if (!loaded)
            continue;
        Raster::Clear(0);
        const auto first = RenderModelDemo(cache, demo);
        const auto pixels = VisiblePixels();
        Check(pixels > 100, "OBJ 经过完整裁剪、光栅化和深度管线生成可见图像");
        Check(first.inputTriangles == loaded.mesh.indices.size() / 3 && first.rasterizedTriangles > 0,
              "全部模型三角形均进入渲染管线");
        Check(Raster::SavePPM(output / (std::string(ModelDemoName(demo)) + ".ppm")), "保存模型验收图像");
        bool validDepth = true;
        for (int y = 0; y < Raster::HEIGHT; ++y)
            for (int x = 0; x < Raster::WIDTH; ++x)
                validDepth &=
                    std::isfinite(depthBuffer[y][x]) && depthBuffer[y][x] >= 0 && depthBuffer[y][x] <= 1;
        Check(validDepth, "模型渲染后的深度值有效且位于 [0,1]");
        double elapsed = 0;
        for (int frame = 0; frame < 3; ++frame)
        {
            Raster::Clear(0);
            elapsed += RenderModelDemo(cache, demo, 0.4f + frame * 0.1f).milliseconds;
        }
        Check(&loaded == &cache.Get(demo) && cache.LoadAttempts(demo) == 1,
              "重复渲染复用同一网格且只加载一次");
        const auto expected = demo == ModelDemo::Bunny    ? 69451u
                              : demo == ModelDemo::Dragon ? 249882u
                              : demo == ModelDemo::Cube   ? 12u
                                                          : 6320u;
        Check(loaded.mesh.indices.size() / 3 == expected, "模型三角形数量与原资源清单一致，没有降采样");
        timings << ModelDemoName(demo) << ',' << loaded.mesh.vertices.size() << ','
                << loaded.mesh.indices.size() / 3 << ',' << loaded.milliseconds << ',' << first.milliseconds
                << ',' << elapsed / 3 << ',' << pixels << ',' << cache.LoadAttempts(demo) << '\n';
        std::printf("验收 %s：加载 %.3f 毫秒，首帧 %.3f 毫秒，后续平均 %.3f 毫秒，%zu 像素\n",
                    ModelDemoName(demo), loaded.milliseconds, first.milliseconds, elapsed / 3, pixels);
    }
}
} // namespace
int main(int argc, char **argv)
{
    SetConsoleOutputCP(CP_UTF8);
    try
    {
        const auto output = std::filesystem::current_path() / "validation" / JRENDERLAB_TEST_CONFIG;
        std::filesystem::create_directories(output);
        const std::string mode = argc > 1 ? argv[1] : "--core";
        if (mode == "--core")
            CoreTests(output);
        else if (mode == "--models-small")
            ModelTests(false, output);
        else if (mode == "--models-large")
            ModelTests(true, output);
        else
            throw std::runtime_error("无效验收模式");
        std::printf("验收失败数：%d\n", failures);
        return failures ? 1 : 0;
    }
    catch (const std::exception &e)
    {
        std::fprintf(stderr, "验收异常：%s\n", e.what());
        return 1;
    }
}
