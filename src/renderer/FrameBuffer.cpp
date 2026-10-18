#include "RasterWindow.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include "assets/WicSupport.h"

namespace
{
std::array<Raster::Color, Raster::WIDTH * Raster::HEIGHT> colors{};
std::array<float, Raster::WIDTH * Raster::HEIGHT> depths{};
bool Contains(int x, int y)
{
    return x >= 0 && y >= 0 && x < Raster::WIDTH && y < Raster::HEIGHT;
}
std::size_t DisplayIndex(int x, int y)
{
    return static_cast<std::size_t>(Raster::HEIGHT - 1 - y) * Raster::WIDTH + x;
}
} // namespace
namespace Raster
{
Color MakeColor(int r, int g, int b)
{
    return ((r & 255u) << 16) | ((g & 255u) << 8) | (b & 255u);
}
void Clear(Color color)
{
    std::fill(colors.begin(), colors.end(), color);
}
void ClearDepth(float depth)
{
    if (!std::isfinite(depth) || depth < 0 || depth > 1)
        throw std::invalid_argument("清屏深度应为有限的 0～1");
    std::fill(depths.begin(), depths.end(), depth);
}
void PutPixel(int x, int y, Color color)
{
    if (Contains(x,y))
    {
        colors[DisplayIndex(x, y)] = color;
    }
}
bool DepthTest(int x, int y, float depth)
{
    // 1. 检查坐标和深度是否合法
    if (!Contains(x, y) ||
        !std::isfinite(depth) ||
        depth < 0 ||
        depth > 1)
    {
        return false;
    }

    // 2. 取得这个像素已保存的深度
    // 会直接修改 depths[index]
    auto& saved = depths[
        static_cast<std::size_t>(y) * WIDTH + x
    ];

    // 3. 当前深度更小，代表距离相机更近
    if (depth < saved)
    {
        // 4. 更新深度
        saved = depth;
        return true;
    }

    // 5. 被遮挡，拒绝绘制
    return false;
}
float GetDepth(int x, int y)
{
    if (!Contains(x, y))
        throw std::out_of_range("深度坐标越界");
    return depths[static_cast<std::size_t>(y) * WIDTH + x];
}
Color GetPixel(int x, int y)
{
    if (!Contains(x, y))
        throw std::out_of_range("像素坐标越界");
    return colors[DisplayIndex(x, y)];
}
const Color *Pixels()
{
    return colors.data();
}
Color EncodeLinearColor(const Vec3& linear)
{
    // 1. 检查 RGB 是否包含 NaN 或 Infinity
    if (!linear.IsFinite())
    {
        throw std::invalid_argument("颜色包含非有限值");
    }

    // 2. 定义单通道编码函数
    const auto encode = [](float c) -> int
        {
        // 将颜色限制在 [0,1]
        c = std::clamp(c, 0.0f, 1.0f);

        // Linear RGB 转换到 sRGB
        float srgb = (c <= 0.0031308f)
            ? 12.92f * c
            : 1.055f * std::pow(c, 1.0f / 2.4f) - 0.055f;

        // sRGB 浮点数转为 8-bit 整数
        return static_cast<int>(std::lround(srgb * 255.0f));
        };

    // 3. 分别转换三个通道，再打包颜色
    return MakeColor(
        encode(linear.x),
        encode(linear.y),
        encode(linear.z)
    );
}

Vec3 DecodeSrgbColor(Color color)
{
    const auto decode = [](unsigned c) {
        const float v = c / 255.f;
        return v <= .04045f ? v / 12.92f : std::pow((v + .055f) / 1.055f, 2.4f);
        };
    return { decode((color >> 16) & 255), decode((color >> 8) & 255), decode(color & 255) };
}
bool SavePPM(const char *utf8Path)
{
    std::ofstream file(std::filesystem::u8path(utf8Path), std::ios::binary);
    file << "P6\n" << WIDTH << ' ' << HEIGHT << "\n255\n";
    for (auto color : colors)
    {
        const char rgb[3] = {static_cast<char>(color >> 16), static_cast<char>(color >> 8), static_cast<char>(color)};
        file.write(rgb, 3);
    }
    return static_cast<bool>(file);
}
bool SavePNG(const char *utf8Path)
{
    try
    {
        ImageIO::Apartment apartment;
        using Microsoft::WRL::ComPtr;
        ComPtr<IWICImagingFactory> factory;
        ImageIO::Check(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                                       IID_PPV_ARGS(&factory)), "无法创建 PNG 工厂");
        ComPtr<IWICStream> stream;
        ImageIO::Check(factory->CreateStream(&stream), "无法创建 PNG 输出流");
        const auto path = std::filesystem::u8path(utf8Path);
        ImageIO::Check(stream->InitializeFromFilename(path.c_str(), GENERIC_WRITE), "无法打开 PNG 输出文件");
        ComPtr<IWICBitmapEncoder> encoder;
        ImageIO::Check(factory->CreateEncoder(GUID_ContainerFormatPng, nullptr, &encoder), "无法创建 PNG 编码器");
        ImageIO::Check(encoder->Initialize(stream.Get(), WICBitmapEncoderNoCache), "无法初始化 PNG 编码器");
        ComPtr<IWICBitmapFrameEncode> frame;
        ImageIO::Check(encoder->CreateNewFrame(&frame, nullptr), "无法创建 PNG 帧");
        ImageIO::Check(frame->Initialize(nullptr), "无法初始化 PNG 帧");
        ImageIO::Check(frame->SetSize(WIDTH, HEIGHT), "无法设置 PNG 尺寸");
        auto format = GUID_WICPixelFormat24bppBGR;
        ImageIO::Check(frame->SetPixelFormat(&format), "无法设置 PNG 格式");
        if (!IsEqualGUID(format, GUID_WICPixelFormat24bppBGR))
            throw std::runtime_error("PNG 编码器不接受 BGR 格式");
        std::vector<BYTE> bgr(static_cast<std::size_t>(WIDTH) * HEIGHT * 3);
        for (std::size_t i = 0; i < colors.size(); ++i)
        {
            bgr[3 * i] = static_cast<BYTE>(colors[i]);
            bgr[3 * i + 1] = static_cast<BYTE>(colors[i] >> 8);
            bgr[3 * i + 2] = static_cast<BYTE>(colors[i] >> 16);
        }
        ImageIO::Check(frame->WritePixels(HEIGHT, WIDTH * 3, static_cast<UINT>(bgr.size()), bgr.data()), "无法写 PNG 像素");
        ImageIO::Check(frame->Commit(), "PNG 帧提交失败");
        ImageIO::Check(encoder->Commit(), "PNG 文件提交失败");
        return true;
    }
    catch (const std::exception &)
    {
        return false;
    }
}
} // namespace Raster
