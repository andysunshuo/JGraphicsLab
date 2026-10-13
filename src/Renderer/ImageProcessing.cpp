#include "Renderer/ImageProcessing.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace RenderLearning
{
namespace
{
void Validate(const Image &image)
{
    if (image.width <= 0 || image.height <= 0 || image.width > 2048 || image.height > 2048 ||
        image.pixels.size() != static_cast<std::size_t>(image.width) * image.height)
        throw std::invalid_argument("学习图像尺寸或像素数量无效");
}
int Address(int coordinate, int size, AddressMode mode)
{
    if (mode == AddressMode::Clamp)
        return std::clamp(coordinate, 0, size - 1);
    return (coordinate % size + size) % size;
}
Vec2 TexelPosition(const Image &image, Vec2 uv, AddressMode mode)
{
    if (!std::isfinite(uv.x) || !std::isfinite(uv.y))
        throw std::invalid_argument("纹理 UV 非有限");
    if (mode == AddressMode::Repeat)
    {
        uv.x -= std::floor(uv.x);
        uv.y -= std::floor(uv.y);
    }
    else
    {
        uv.x = std::clamp(uv.x, 0.0f, 1.0f);
        uv.y = std::clamp(uv.y, 0.0f, 1.0f);
    }
    // UV 的 V 向上，图像的行号向下；整数坐标指 texel 中心。
    return {uv.x * image.width - 0.5f, (1 - uv.y) * image.height - 0.5f};
}
Vec3 Mix(Vec3 a, Vec3 b, float t)
{
    return Add(Multiply(a, 1 - t), Multiply(b, t));
}
} // namespace
Image::Image(int w, int h, Vec3 fill) : width(w), height(h)
{
    if (w <= 0 || h <= 0 || w > 2048 || h > 2048)
        throw std::invalid_argument("学习图像单边应为 1～2048");
    pixels.assign(static_cast<std::size_t>(w) * h, fill);
}
const Vec3 &Image::At(int x, int y) const
{
    Validate(*this);
    if (x < 0 || y < 0 || x >= width || y >= height)
        throw std::out_of_range("学习图像像素坐标越界");
    return pixels[static_cast<std::size_t>(y) * width + x];
}
Vec3 &Image::At(int x, int y)
{
    return const_cast<Vec3 &>(static_cast<const Image &>(*this).At(x, y));
}
float SrgbToLinear(float c)
{
    c = std::clamp(c, 0.0f, 1.0f);
    return c <= 0.04045f ? c / 12.92f : std::pow((c + 0.055f) / 1.055f, 2.4f);
}
Raster::Color DisplayColor(Vec3 c)
{
    const auto encode = [](float value) {
        if (!std::isfinite(value))
            value = 0;
        value = std::clamp(value, 0.0f, 1.0f);
        value = value <= 0.0031308f ? value * 12.92f : 1.055f * std::pow(value, 1 / 2.4f) - 0.055f;
        return static_cast<int>(std::lround(value * 255));
    };
    return Raster::MakeColor(encode(c.x), encode(c.y), encode(c.z));
}
Image FromTexture(const Assets::Texture &texture, bool srgb)
{
    if (texture.pixels.size() != static_cast<std::size_t>(texture.width) * texture.height)
        throw std::invalid_argument("纹理像素数量无效");
    Image result(static_cast<int>(texture.width), static_cast<int>(texture.height));
    for (std::size_t i = 0; i < result.pixels.size(); ++i)
    {
        const auto c = texture.pixels[i];
        Vec3 rgb{Raster::GetR(c) / 255.0f, Raster::GetG(c) / 255.0f, Raster::GetB(c) / 255.0f};
        if (srgb)
            rgb = {SrgbToLinear(rgb.x), SrgbToLinear(rgb.y), SrgbToLinear(rgb.z)};
        result.pixels[i] = rgb;
    }
    return result;
}
Image Convolve(const Image &source, const std::vector<float> &kernel, int side)
{
    Validate(source);
    if (side <= 0 || side > 15 || side % 2 == 0 || kernel.size() != static_cast<std::size_t>(side) * side)
        throw std::invalid_argument("卷积核必须为不超过 15 的奇数阶方阵");
    for (float weight : kernel)
        if (!std::isfinite(weight))
            throw std::invalid_argument("卷积核非有限");
    Image result(source.width, source.height);
    const int radius = side / 2;
    for (int y = 0; y < source.height; ++y)
        for (int x = 0; x < source.width; ++x)
        {
            Vec3 sum{};
            for (int ky = 0; ky < side; ++ky)
                for (int kx = 0; kx < side; ++kx)
                    sum = Add(sum, Multiply(source.At(std::clamp(x - (kx - radius), 0, source.width - 1),
                                                      std::clamp(y - (ky - radius), 0, source.height - 1)),
                                            kernel[ky * side + kx]));
            result.At(x, y) = sum;
        }
    return result;
}
Image Downsample(const Image &source, int factor)
{
    Validate(source);
    if (factor < 1 || factor > 16)
        throw std::invalid_argument("降采样倍数应为 1～16");
    Image result((source.width + factor - 1) / factor, (source.height + factor - 1) / factor);
    for (int y = 0; y < result.height; ++y)
        for (int x = 0; x < result.width; ++x)
        {
            Vec3 sum{};
            int count = 0;
            for (int sy = y * factor; sy < std::min(source.height, (y + 1) * factor); ++sy)
                for (int sx = x * factor; sx < std::min(source.width, (x + 1) * factor); ++sx)
                {
                    sum = Add(sum, source.At(sx, sy));
                    ++count;
                }
            result.At(x, y) = Multiply(sum, 1.0f / count);
        }
    return result;
}
std::vector<Image> BuildMipmaps(const Image &source)
{
    Validate(source);
    std::vector<Image> levels;
    levels.reserve(12);
    levels.push_back(source);
    while (levels.back().width > 1 || levels.back().height > 1)
        levels.push_back(Downsample(levels.back()));
    return levels;
}
Vec3 SampleNearest(const Image &image, Vec2 uv, AddressMode mode)
{
    Validate(image);
    const auto p = TexelPosition(image, uv, mode);
    return image.At(Address(static_cast<int>(std::floor(p.x + 0.5f)), image.width, mode),
                    Address(static_cast<int>(std::floor(p.y + 0.5f)), image.height, mode));
}
Vec3 SampleBilinear(const Image &image, Vec2 uv, AddressMode mode)
{
    Validate(image);
    const auto p = TexelPosition(image, uv, mode);
    const int x = static_cast<int>(std::floor(p.x)), y = static_cast<int>(std::floor(p.y));
    const float a = p.x - x, b = p.y - y;
    const auto read = [&](int ix, int iy) {
        return image.At(Address(ix, image.width, mode), Address(iy, image.height, mode));
    };
    return Mix(Mix(read(x, y), read(x + 1, y), a), Mix(read(x, y + 1), read(x + 1, y + 1), a), b);
}
Vec3 SampleTrilinear(const std::vector<Image> &levels, Vec2 uv, float lod, AddressMode mode)
{
    if (levels.empty() || !std::isfinite(lod))
        throw std::invalid_argument("Mipmap 或 LOD 无效");
    lod = std::clamp(lod, 0.0f, static_cast<float>(levels.size() - 1));
    const auto lo = static_cast<std::size_t>(std::floor(lod)), hi = std::min(lo + 1, levels.size() - 1);
    return Mix(SampleBilinear(levels[lo], uv, mode), SampleBilinear(levels[hi], uv, mode), lod - lo);
}
float EstimateLod(const Image &base, Vec2 dx, Vec2 dy)
{
    Validate(base);
    if (!std::isfinite(dx.x) || !std::isfinite(dx.y) || !std::isfinite(dy.x) || !std::isfinite(dy.y))
        throw std::invalid_argument("纹理导数非有限");
    const float rho = std::max(std::hypot(dx.x * base.width, dx.y * base.height),
                               std::hypot(dy.x * base.width, dy.y * base.height));
    return std::log2(std::max(rho, 1e-8f));
}
Image Checker(int w, int h, int cell)
{
    if (cell < 1)
        throw std::invalid_argument("棋盘格间距必须为正");
    Image result(w, h);
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
        {
            const float c = ((x / cell + y / cell) & 1) ? 0.9f : 0.025f;
            result.At(x, y) = {c, c, c};
        }
    return result;
}
} // namespace RenderLearning
