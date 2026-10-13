#pragma once
#include "Core/Math.h"
#include "Assets/TextureLoader.h"
#include <vector>

namespace RenderLearning
{
// 图像内存从上到下排列，RGB 为线性浮点值；显示时才编码为 sRGB。
struct Image
{
    int width = 0, height = 0;
    std::vector<Vec3> pixels;
    Image() = default;
    Image(int width, int height, Vec3 fill = {});
    const Vec3 &At(int x, int y) const;
    Vec3 &At(int x, int y);
};
enum class AddressMode
{
    Clamp,
    Repeat
};
Image FromTexture(const Assets::Texture &texture, bool srgb = true);
// 真卷积翻转核方向，边缘钳制；奇数核边长不超过 15。
Image Convolve(const Image &source, const std::vector<float> &kernel, int side);
// 在线性空间平均实际存在的块像素，奇数尺寸向上取整。
Image Downsample(const Image &source, int factor = 2);
std::vector<Image> BuildMipmaps(const Image &source);
Vec3 SampleNearest(const Image &source, Vec2 uv, AddressMode address = AddressMode::Repeat);
Vec3 SampleBilinear(const Image &source, Vec2 uv, AddressMode address = AddressMode::Repeat);
Vec3 SampleTrilinear(const std::vector<Image> &levels, Vec2 uv, float lod,
                     AddressMode address = AddressMode::Repeat);
float EstimateLod(const Image &base, Vec2 duvDx, Vec2 duvDy);
float SrgbToLinear(float value);
Raster::Color DisplayColor(Vec3 linear);
Image Checker(int width, int height, int cell = 8);
} // namespace RenderLearning
