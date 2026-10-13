#include "Renderer/AssetPreview.h"
#include <windows.h>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <stdexcept>

namespace
{
Raster::Color ToneMap(const Vec3 &radiance)
{
    const auto channel = [](float value) {
        // 曝光固定为 1，Reinhard 压缩后做 gamma 显示转换，保留浮点原数据。
        return static_cast<int>(std::lround(std::pow(value / (1.0f + value), 1.0f / 2.2f) * 255.0f));
    };
    return Raster::MakeColor(channel(radiance.x), channel(radiance.y), channel(radiance.z));
}
Vec3 SampleEnvironment(const Assets::HdrImage &image, const Vec3 &direction)
{
    constexpr float pi = 3.14159265358979323846f;
    const float u = 0.5f + std::atan2(direction.z, direction.x) / (2 * pi);
    const float v = std::acos(std::clamp(direction.y, -1.0f, 1.0f)) / pi;
    const int x = std::clamp(static_cast<int>(u * image.width), 0, static_cast<int>(image.width) - 1);
    const int y = std::clamp(static_cast<int>(v * image.height), 0, static_cast<int>(image.height) - 1);
    return image.pixels[static_cast<std::size_t>(y) * image.width + x];
}
} // namespace
namespace AssetPreview
{
void Begin(AssetPreviewFrame &frame)
{
    if (frame.width <= 0 || frame.height <= 0 || frame.width > 1200 || frame.height > 1200)
        throw std::invalid_argument("资源预览画布尺寸无效");
    frame.pixels.assign(static_cast<std::size_t>(frame.width) * frame.height, Raster::MakeColor(18, 22, 30));
}
void Image(AssetPreviewFrame &frame, const Raster::Color *pixels, int width, int height, int x, int y,
           int boxWidth, int boxHeight)
{
    if (!pixels || width <= 0 || height <= 0 || boxWidth <= 0 || boxHeight <= 0)
        throw std::invalid_argument("资源预览图像或矩形尺寸无效");
    const double scale =
        std::min(static_cast<double>(boxWidth) / width, static_cast<double>(boxHeight) / height);
    const int w = std::max(1, static_cast<int>(width * scale));
    const int h = std::max(1, static_cast<int>(height * scale));
    const int left = x + (boxWidth - w) / 2, top = y + (boxHeight - h) / 2;
    for (int dy = 0; dy < h; ++dy)
        for (int dx = 0; dx < w; ++dx)
        {
            const int px = left + dx, py = top + dy;
            if (px < 0 || py < 0 || px >= frame.width || py >= frame.height)
                continue;
            // 展示原图颜色，使用最近邻，不擅自把法线图当颜色图做 gamma 或滤波。
            const int sx = std::min(width - 1, dx * width / w), sy = std::min(height - 1, dy * height / h);
            frame.pixels[static_cast<std::size_t>(py) * frame.width + px] =
                pixels[static_cast<std::size_t>(sy) * width + sx];
        }
}
void Caption(AssetPreviewFrame &frame, const std::wstring &title, const std::wstring &description,
             const std::wstring &path, const std::wstring &footer)
{
    // 在内存 DIB 上绘制中文，不创建额外窗口；最后复制到资源专用画布。
    HDC dc = CreateCompatibleDC(nullptr);
    if (!dc)
        throw std::runtime_error("无法创建资源预览文字画布");
    BITMAPINFO info{};
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = frame.width;
    info.bmiHeader.biHeight = -frame.height;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    void *bits = nullptr;
    HBITMAP bitmap = CreateDIBSection(dc, &info, DIB_RGB_COLORS, &bits, nullptr, 0);
    if (!bitmap)
    {
        DeleteDC(dc);
        throw std::runtime_error("无法创建资源预览位图");
    }
    HGDIOBJ previousBitmap = SelectObject(dc, bitmap);
    std::copy(frame.pixels.begin(), frame.pixels.end(), static_cast<Raster::Color *>(bits));
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, RGB(225, 230, 240));
    HFONT titleFont =
        CreateFontW(-24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                    CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, L"Microsoft YaHei");
    HFONT bodyFont =
        CreateFontW(-16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                    CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, L"Microsoft YaHei");
    HGDIOBJ previousFont = SelectObject(dc, titleFont ? titleFont : GetStockObject(DEFAULT_GUI_FONT));
    RECT titleRect{20, 10, frame.width - 20, 42};
    DrawTextW(dc, title.c_str(), -1, &titleRect, DT_SINGLELINE | DT_END_ELLIPSIS);
    SelectObject(dc, bodyFont ? bodyFont : GetStockObject(DEFAULT_GUI_FONT));
    RECT descriptionRect{20, 45, frame.width - 20, 69};
    DrawTextW(dc, description.c_str(), -1, &descriptionRect, DT_SINGLELINE | DT_END_ELLIPSIS);
    RECT pathRect{20, 72, frame.width - 20, 94};
    DrawTextW(dc, path.c_str(), -1, &pathRect, DT_SINGLELINE | DT_END_ELLIPSIS);
    RECT footerRect{20, frame.height - 43, frame.width - 20, frame.height - 3};
    DrawTextW(dc, footer.c_str(), -1, &footerRect, DT_WORDBREAK);
    GdiFlush();
    std::copy_n(static_cast<Raster::Color *>(bits), frame.pixels.size(), frame.pixels.begin());
    SelectObject(dc, previousFont);
    SelectObject(dc, previousBitmap);
    if (titleFont)
        DeleteObject(titleFont);
    if (bodyFont)
        DeleteObject(bodyFont);
    DeleteObject(bitmap);
    DeleteDC(dc);
}
void Environment(AssetPreviewFrame &frame, const Assets::HdrImage &image)
{
    if (!image.width || !image.height ||
        image.pixels.size() != static_cast<std::size_t>(image.width) * image.height)
        throw std::invalid_argument("环境预览图像尺寸无效");
    // 预览色只计算一次，HDR 原数据仍保留用于球面方向采样。
    std::vector<Raster::Color> mapped(image.pixels.size());
    std::transform(image.pixels.begin(), image.pixels.end(), mapped.begin(), ToneMap);
    Image(frame, mapped.data(), static_cast<int>(image.width), static_cast<int>(image.height), 30, 102, 540,
          286);
    // 解析球面的反射方向 r=2(n·v)n-v；它只是展示环境采样，不改写原 Mesh 着色管线。
    constexpr int radius = 78, centerX = 300, centerY = 472;
    for (int y = centerY - radius; y <= centerY + radius; ++y)
        for (int x = centerX - radius; x <= centerX + radius; ++x)
        {
            const float nx = static_cast<float>(x - centerX) / radius;
            const float ny = -static_cast<float>(y - centerY) / radius;
            const float squared = nx * nx + ny * ny;
            if (squared > 1 || x < 0 || y < 0 || x >= frame.width || y >= frame.height)
                continue;
            const float nz = std::sqrt(1 - squared);
            const Vec3 reflected{2 * nx * nz, 2 * ny * nz, 2 * nz * nz - 1};
            frame.pixels[static_cast<std::size_t>(y) * frame.width + x] =
                ToneMap(SampleEnvironment(image, reflected));
        }
}
bool SavePPM(const AssetPreviewFrame &frame, const std::filesystem::path &path)
{
    std::ofstream file(path, std::ios::binary);
    if (!file)
        return false;
    file << "P6\n" << frame.width << ' ' << frame.height << "\n255\n";
    for (Raster::Color c : frame.pixels)
    {
        const char rgb[3]{static_cast<char>(Raster::GetR(c)), static_cast<char>(Raster::GetG(c)),
                          static_cast<char>(Raster::GetB(c))};
        file.write(rgb, 3);
    }
    return static_cast<bool>(file);
}
} // namespace AssetPreview
