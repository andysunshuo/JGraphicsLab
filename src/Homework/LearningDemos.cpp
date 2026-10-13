#include "Homework/LearningDemos.h"
#include "Application/ModelDemos.h"
#include "Assets/ResourcePaths.h"
#include <windows.h>
#include <algorithm>
#include <cstdio>
#include <stdexcept>

namespace HomeworkLearning
{
namespace
{
AssetPreviewFrame lesson;
int lastNumber = 0;
void Label(const std::wstring &text, int x, int y)
{
    HDC dc = CreateCompatibleDC(nullptr);
    if (!dc)
        throw std::runtime_error("无法创建学习画布文字上下文");
    BITMAPINFO info{};
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = lesson.width;
    info.bmiHeader.biHeight = -lesson.height;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    void *bits = nullptr;
    HBITMAP bitmap = CreateDIBSection(dc, &info, DIB_RGB_COLORS, &bits, nullptr, 0);
    if (!bitmap)
    {
        DeleteDC(dc);
        throw std::runtime_error("无法创建学习画布位图");
    }
    const auto oldBitmap = SelectObject(dc, bitmap);
    std::copy(lesson.pixels.begin(), lesson.pixels.end(), static_cast<Raster::Color *>(bits));
    HFONT font =
        CreateFontW(-16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                    CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, L"Microsoft YaHei");
    const auto oldFont = SelectObject(dc, font ? font : GetStockObject(DEFAULT_GUI_FONT));
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, RGB(220, 228, 238));
    RECT rect{x, y, lesson.width - 8, y + 22};
    DrawTextW(dc, text.c_str(), -1, &rect, DT_SINGLELINE);
    GdiFlush();
    std::copy_n(static_cast<Raster::Color *>(bits), lesson.pixels.size(), lesson.pixels.begin());
    SelectObject(dc, oldFont);
    SelectObject(dc, oldBitmap);
    if (font)
        DeleteObject(font);
    DeleteObject(bitmap);
    DeleteDC(dc);
}
} // namespace
void Publish(int number, const std::wstring &name, const std::wstring &principle, std::vector<Panel> panels)
{
    if (panels.empty() || panels.size() > 12)
        throw std::invalid_argument("学习对照面板数量应为 1～12");
    AssetPreview::Begin(lesson);
    const int columns = panels.size() <= 3 ? static_cast<int>(panels.size()) : 4;
    const int rows = (static_cast<int>(panels.size()) + columns - 1) / columns;
    const int cellW = 570 / columns, cellH = 450 / rows;
    for (std::size_t i = 0; i < panels.size(); ++i)
    {
        const int x = 15 + static_cast<int>(i % columns) * cellW,
                  y = 100 + static_cast<int>(i / columns) * cellH;
        std::vector<Raster::Color> colors(panels[i].image.pixels.size());
        std::transform(panels[i].image.pixels.begin(), panels[i].image.pixels.end(), colors.begin(),
                       DisplayColor);
        AssetPreview::Image(lesson, colors.data(), panels[i].image.width, panels[i].image.height, x, y + 25,
                            cellW - 8, cellH - 32);
        Label(panels[i].label, x + 3, y);
    }
    AssetPreview::Caption(lesson, L"HW" + std::to_wstring(number) + L"  " + name, principle,
                          L"源码：src/Homework/HW" + std::to_wstring(number) + L".cpp",
                          L"← / → 切换 HW32～HW50；控制台看实际值/预期值与 PASS/FAIL。");
    lastNumber = number;
}
const AssetPreviewFrame *DisplayFor(int number)
{
    return lastNumber == number ? &lesson : nullptr;
}
const Mesh &Sphere()
{
    static const Mesh mesh = MakeUVSphere(28, 14);
    return mesh;
}
const Mesh &CoarseSphere()
{
    static const Mesh mesh = MakeUVSphere(10, 6);
    return mesh;
}
const Mesh &Cube()
{
    static const Mesh mesh = MakeUVCube();
    return mesh;
}
const Mesh &Ground()
{
    static const Mesh mesh = [] {
        Mesh m;
        m.vertices = {{{-3, -0.8f, -12}, {0, 1, 0}, {0, 8}},
                      {{3, -0.8f, -12}, {0, 1, 0}, {8, 8}},
                      {{3, -0.8f, 1}, {0, 1, 0}, {8, 0}},
                      {{-3, -0.8f, 1}, {0, 1, 0}, {0, 0}}};
        m.indices = {0, 2, 1, 0, 3, 2};
        return m;
    }();
    return mesh;
}
const Image &ColorTexture()
{
    static const Image image = [] {
        Assets::Texture texture;
        std::string error;
        if (!Assets::LoadTexture(Assets::Resolve("Textures/wood.jpg"), texture, error))
            throw std::runtime_error(error);
        return FromTexture(texture);
    }();
    return image;
}
const Image &CheckerTexture()
{
    static const Image image = Checker(256, 256, 16);
    return image;
}
const std::vector<Image> &CheckerMipmaps()
{
    static const auto levels = BuildMipmaps(CheckerTexture());
    return levels;
}
Image Render(const Mesh &mesh, PipelineOptions options, Mat4 model, int resolution, int samples)
{
    RenderTarget target(resolution, resolution, samples);
    Camera camera;
    // 学习画布使用较窄的视角，让面法线、光照和纹理细节更容易观察。
    camera.fovRadians = 0.87266463f;
    options.light.eye = camera.eye;
    const auto stats = RenderLearning::DrawMesh(mesh, model, camera, options, target);
    std::printf("学习渲染：%zu 输入三角形，%zu 栅格三角形，%zu 深度写入；%d×%d、%d 样本\n",
                stats.inputTriangles, stats.rasterTriangles, stats.depthWrites, resolution, resolution,
                samples);
    return target.Resolve();
}
Image RenderGround(PipelineOptions options)
{
    RenderTarget target(256, 256);
    Camera camera;
    camera.eye = {0, 1.5f, 3};
    camera.target = {0, -0.8f, -3};
    options.light.eye = camera.eye;
    RenderLearning::DrawMesh(Ground(), IdentityMat4(), camera, options, target);
    return target.Resolve();
}
Image Magnify(const Image &source, bool bilinear, int size)
{
    Image result(size, size);
    for (int y = 0; y < size; ++y)
        for (int x = 0; x < size; ++x)
        {
            Vec2 uv{(x + 0.5f) / size, 1 - (y + 0.5f) / size};
            result.At(x, y) = bilinear ? SampleBilinear(source, uv, AddressMode::Clamp)
                                       : SampleNearest(source, uv, AddressMode::Clamp);
        }
    return result;
}
Image Decimate(const Image &source, int factor)
{
    if (factor < 1 || factor > 16)
        throw std::invalid_argument("直接抽样倍数无效");
    Image result((source.width + factor - 1) / factor, (source.height + factor - 1) / factor);
    for (int y = 0; y < result.height; ++y)
        for (int x = 0; x < result.width; ++x)
            result.At(x, y) = source.At(x * factor, y * factor);
    return result;
}
Image Occlusion(int samples, int size)
{
    RenderTarget target(size, size, samples);
    PipelineOptions options;
    options.shading = Shading::Flat;
    PipelineStats stats;
    const auto vertex = [](float x, float y, float z, Vec3 color) {
        return PipelineVertex{{x, y}, z, 1, {{}, {0, 0, 1}, color, {}}};
    };
    RasterizeAttributedTriangle(vertex(size * 0.1f, size * 0.08f, 0.6f, {0.02f, 0.2f, 0.9f}),
                                vertex(size * 0.95f, size * 0.22f, 0.6f, {0.02f, 0.2f, 0.9f}),
                                vertex(size * 0.44f, size * 0.96f, 0.6f, {0.02f, 0.2f, 0.9f}), target,
                                options, stats);
    RasterizeAttributedTriangle(vertex(size * 0.13f, size * 0.17f, -0.4f, {0.9f, 0.03f, 0.04f}),
                                vertex(size * 0.87f, size * 0.36f, -0.4f, {0.9f, 0.03f, 0.04f}),
                                vertex(size * 0.29f, size * 0.8f, -0.4f, {0.9f, 0.03f, 0.04f}), target,
                                options, stats);
    return target.Resolve();
}
} // namespace HomeworkLearning
