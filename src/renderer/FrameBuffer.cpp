#include "RasterWindow.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <stdexcept>

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
    if (Contains(x, y))
        colors[DisplayIndex(x, y)] = color;
}
bool DepthTest(int x, int y, float depth)
{
    if (!Contains(x, y) || !std::isfinite(depth) || depth < 0 || depth > 1)
        return false;
    auto &saved = depths[static_cast<std::size_t>(y) * WIDTH + x];
    if (depth < saved)
    {
        saved = depth;
        return true;
    }
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
Color EncodeLinearColor(const Vec3 &linear)
{
    if (!linear.IsFinite())
        throw std::invalid_argument("颜色包含非有限值");
    const auto encode = [](float c) {
        c = std::clamp(c, 0.0f, 1.0f);
        return static_cast<int>(
            std::lround(255 * (c <= 0.0031308f ? 12.92f * c : 1.055f * std::pow(c, 1 / 2.4f) - .055f)));
    };
    return MakeColor(encode(linear.x), encode(linear.y), encode(linear.z));
}
Vec3 DecodeSrgbColor(Color color)
{
    const auto decode = [](unsigned c) {
        const float v = c / 255.f;
        return v <= .04045f ? v / 12.92f : std::pow((v + .055f) / 1.055f, 2.4f);
    };
    return {decode((color >> 16) & 255), decode((color >> 8) & 255), decode(color & 255)};
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
} // namespace Raster
