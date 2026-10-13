#include "Homework/AssetHomework.h"
#include "Assets/ResourcePaths.h"
#include "Renderer/Framebuffer.h"
#include <chrono>
#include <cmath>
#include <cstdio>
#include <stdexcept>

namespace
{
struct CachedPreview
{
    AssetPreviewFrame frame;
    Assets::Texture texture;
    Assets::HdrImage hdr;
    bool ready = false;
    std::size_t attempts = 0;
    float angle = 0;
    std::string error;
};
std::array<CachedPreview, 10> cache;
ModelDemoCache models;
std::size_t IndexOf(int id)
{
    if (!FindAssetHomework(id))
        throw std::invalid_argument("无效的 HW50 资源展示编号");
    return static_cast<std::size_t>(id - HW50_1);
}
void Caption(CachedPreview &slot, const AssetHomework &entry, const std::wstring &footer)
{
    AssetPreview::Caption(slot.frame,
                          std::wstring(entry.key, entry.key + std::char_traits<char>::length(entry.key)) +
                              L"  " + entry.title,
                          entry.description, L"Assets/" + std::wstring(entry.path), footer);
}
void LogLoaded(const AssetHomework &entry, unsigned width, unsigned height, double milliseconds)
{
    std::printf("%s：%s；图像 %u×%u；加载 %.3f 毫秒（缓存复用）\n", entry.key,
                Assets::PathUtf8(std::filesystem::path(entry.path)).c_str(), width, height, milliseconds);
}
} // namespace
const std::array<AssetHomework, 10> &AssetHomeworkRegistry()
{
    // 独立扩展表，不改原来的 50 条作业记录或未实现状态。
    static const std::array<AssetHomework, 10> registry{{
        {HW50_1, "HW50_1", L"立方体 OBJ", L"三维几何：位置、法线、UV 与三角形索引", L"Models/cube.obj",
         L"彩色表示模型空间面法线；OBJ 只加载一次。", RenderHW50_1},
        {HW50_2, "HW50_2", L"茶壶 OBJ", L"曲面测试模型；使用原 MVP、裁剪、深度与光栅化管线",
         L"Models/teapot.obj", L"彩色表示模型空间面法线；OBJ 只加载一次。", RenderHW50_2},
        {HW50_3, "HW50_3", L"兔子 OBJ", L"完整几何：69,451 个三角形，没有降采样", L"Models/bunny.obj",
         L"大模型建议 Release；可用 --animate 查看旋转。", RenderHW50_3},
        {HW50_4, "HW50_4", L"龙 OBJ", L"完整几何：249,882 个三角形，没有降采样", L"Models/dragon.obj",
         L"大模型建议 Release；可用 --animate 查看旋转。", RenderHW50_4},
        {HW50_5, "HW50_5", L"棋盘格颜色贴图", L"检查 UV 排布、重复和纹理采样时使用的规则图案",
         L"Textures/checker.png", L"颜色贴图：每个像素存储表面颜色；这里直接展示原图。", RenderHW50_5},
        {HW50_6, "HW50_6", L"木材颜色贴图", L"木纹的基础颜色，用作材质的颜色输入", L"Textures/wood.jpg",
         L"它不保存三维几何或光照；这里直接展示颜色纹理。", RenderHW50_6},
        {HW50_7, "HW50_7", L"砖墙颜色贴图", L"砖块与砖缝的颜色，和后两张图属于同一砖墙材质",
         L"Textures/brick_color.jpg", L"与 HW50_8 法线、HW50_9 高度图配套使用。", RenderHW50_7},
        {HW50_8, "HW50_8", L"砖墙法线贴图", L"RGB 编码切线空间 X/Y/Z 方向，用于表面光照细节",
         L"NormalMaps/brick_normal.jpg", L"蓝紫色是方向数据；解码方向为 2×RGB/255－1。", RenderHW50_8},
        {HW50_9, "HW50_9", L"砖墙高度图", L"灰度编码相对高度：较亮处较高，较暗处较低",
         L"HeightMaps/brick_height.jpg", L"可用于凹凸或位移；本例直接显示图像，不移动模型顶点。",
         RenderHW50_9},
        {HW50_10, "HW50_10", L"摄影棚 HDR 环境图", L"上：360° 全景；下：同一环境在镜面球上的方向采样",
         L"Environment/studio_small_09.hdr", L"浮点亮度经曝光映射显示；用于环境反射与环境照明。",
         RenderHW50_10},
    }};
    return registry;
}
const AssetHomework *FindAssetHomework(int id)
{
    if (id < HW50_1 || id > HW50_10)
        return nullptr;
    return &AssetHomeworkRegistry()[static_cast<std::size_t>(id - HW50_1)];
}
int ParseAssetHomeworkId(const std::string &name)
{
    for (const auto &entry : AssetHomeworkRegistry())
        if (name == entry.key || "HW" + name == entry.key)
            return entry.id;
    return 0;
}
void ListAssetHomework()
{
    for (const auto &entry : AssetHomeworkRegistry())
        std::printf("%s：%s；Assets/%s\n", entry.key,
                    Assets::PathUtf8(std::filesystem::path(entry.title)).c_str(),
                    Assets::PathUtf8(std::filesystem::path(entry.path)).c_str());
}
bool IsAnimatedAssetHomework(int id)
{
    return id >= HW50_1 && id <= HW50_4;
}
std::size_t AssetHomeworkLoadCount(int id)
{
    const auto index = IndexOf(id);
    if (index < 4)
        return models.LoadAttempts(static_cast<ModelDemo>(index + 1));
    return cache[index].attempts;
}
const AssetPreviewFrame &ShowAssetModel(int id, ModelDemo model, float angle)
{
    auto &slot = cache[IndexOf(id)];
    const auto &entry = *FindAssetHomework(id);
    if (!std::isfinite(angle))
        throw std::invalid_argument("模型展示角度无效");
    if (slot.ready && slot.angle == angle)
        return slot.frame;
    const auto &loaded = models.Get(model);
    if (!loaded)
        throw std::runtime_error(loaded.error);
    Raster::Clear(0);
    // 仅改变这个资源展示的模型尺寸，不更改旧作业或旧模型演示的参数。
    const Mat4 modelMatrix =
        Multiply(MakeRotationY3D(angle), Multiply(MakeScale3D(1.4f, 1.4f, 1.4f), models.FittedModel(model)));
    const auto stats = DrawMesh(loaded.mesh, modelMatrix, Camera{});
    AssetPreview::Begin(slot.frame);
    AssetPreview::Image(slot.frame, Raster::Pixels(), Raster::WIDTH, Raster::HEIGHT, 30, 102, 540, 448);
    Caption(slot, entry,
            L"顶点 " + std::to_wstring(loaded.mesh.vertices.size()) + L"；三角形 " +
                std::to_wstring(stats.inputTriangles) + L"；渲染 " + std::to_wstring(stats.milliseconds) +
                L" ms；面法线颜色");
    if (!slot.ready)
        std::printf("%s：提交全部 %zu 个三角形；渲染 %.3f 毫秒\n", entry.key, stats.inputTriangles,
                    stats.milliseconds);
    slot.ready = true;
    slot.angle = angle;
    return slot.frame;
}
const AssetPreviewFrame &ShowAssetImage(int id)
{
    auto &slot = cache[IndexOf(id)];
    const auto &entry = *FindAssetHomework(id);
    if (slot.ready)
        return slot.frame;
    if (!slot.error.empty())
        throw std::runtime_error(slot.error);
    const auto start = std::chrono::steady_clock::now();
    ++slot.attempts;
    try
    {
        if (!Assets::LoadTexture(Assets::Resolve(entry.path), slot.texture, slot.error))
            throw std::runtime_error(slot.error);
        AssetPreview::Begin(slot.frame);
        AssetPreview::Image(slot.frame, slot.texture.pixels.data(), static_cast<int>(slot.texture.width),
                            static_cast<int>(slot.texture.height), 30, 102, 540, 448);
        Caption(slot, entry,
                std::to_wstring(slot.texture.width) + L"×" + std::to_wstring(slot.texture.height) + L"；" +
                    entry.footer);
        slot.ready = true;
        LogLoaded(
            entry, slot.texture.width, slot.texture.height,
            std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count());
    }
    catch (const std::exception &e)
    {
        slot.error = e.what();
        throw;
    }
    return slot.frame;
}
const AssetPreviewFrame &ShowAssetEnvironment(int id)
{
    auto &slot = cache[IndexOf(id)];
    const auto &entry = *FindAssetHomework(id);
    if (slot.ready)
        return slot.frame;
    if (!slot.error.empty())
        throw std::runtime_error(slot.error);
    const auto start = std::chrono::steady_clock::now();
    ++slot.attempts;
    try
    {
        if (!Assets::LoadHdr(Assets::Resolve(entry.path), slot.hdr, slot.error))
            throw std::runtime_error(slot.error);
        AssetPreview::Begin(slot.frame);
        AssetPreview::Environment(slot.frame, slot.hdr);
        Caption(slot, entry,
                std::to_wstring(slot.hdr.width) + L"×" + std::to_wstring(slot.hdr.height) +
                    L"；HDR 显示映射 c/(1+c)，gamma=2.2");
        slot.ready = true;
        LogLoaded(
            entry, slot.hdr.width, slot.hdr.height,
            std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count());
    }
    catch (const std::exception &e)
    {
        slot.error = e.what();
        throw;
    }
    return slot.frame;
}
