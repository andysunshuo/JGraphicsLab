#pragma once
#include "math/Vec2.h"
#include "math/Vec3.h"
#include <cstddef>
#include <string>
#include <vector>

enum class TextureFilter
{
    Nearest,
    Bilinear,
    Trilinear
};
enum class TextureWrap
{
    Clamp,
    Repeat
};
// 颜色纹理解码为线性值；法线/高度等数据不能应用 sRGB 幂律。
enum class TextureEncoding
{
    SrgbColor,
    LinearData
};
struct TextureLevel
{
    int width = 0, height = 0;
    std::vector<Vec3> pixels; // 行号从上到下，三通道浮点值。
};

// Texture.cpp 已实现 WIC 读图、最近邻、双线性、Mipmap 和三线性；演示默认双线性基础级。
class Texture
{
  public:
    // 使用 Windows WIC 读取 PNG/JPEG/BMP，CMake 已链接 windowscodecs 和 ole32。
    // 校验文件/尺寸预算：64 MiB、单边 <=2048；失败清空并返回中文 error。
    // 颜色图调用 Raster::DecodeSrgbColor，数据图只把通道除以 255。
    bool Load(const std::string &utf8Path, TextureEncoding encoding = TextureEncoding::SrgbColor,
              std::string *error = nullptr);
    // 用程序生成的线性 RGB 创建基础级；检查 width*height==pixels.size() 和预算。
    void Create(int width, int height, const std::vector<Vec3> &linearPixels);
    void Clear();
    bool Empty() const;
    int Width() const;
    int Height() const;
    std::size_t LevelCount() const;
    const TextureLevel &Level(std::size_t index = 0) const;

    // 共用 UV 地址规则：V 向上，texel 中心 s=u*W-0.5、t=(1-v)*H-0.5。
    // Repeat 用 u-floor(u)，支持负 UV；Clamp 限制 [0,1]，再限制邻域坐标。
    Vec3 SampleNearest(const Vec2 &uv, std::size_t level = 0, TextureWrap wrap = TextureWrap::Repeat) const;
    // 取四 texel：先沿 X 再沿 Y 插值；在色彩线性空间完成，不能直接平均 sRGB 字节。
    Vec3 SampleBilinear(const Vec2 &uv, std::size_t level = 0, TextureWrap wrap = TextureWrap::Repeat) const;
    // 两相邻级分别调用 SampleBilinear，再按 LOD 小数混合；级别钳制到合法范围。
    Vec3 SampleTrilinear(const Vec2 &uv, float lod, TextureWrap wrap = TextureWrap::Repeat) const;
    // 仅按 filter 分派上述函数，不复制采样公式。
    Vec3 Sample(const Vec2 &uv, TextureFilter filter = TextureFilter::Bilinear, TextureWrap wrap = TextureWrap::Repeat,
                float lod = 0) const;
    // 每级 2×2 平均，奇数尺寸向上取整且只平均有效像素，直到 1×1；只生成一次。
    void GenerateMipmaps();
    // rho=max(length(duvDx*(W,H)),length(duvDy*(W,H)))；返回 log2(max(rho,epsilon))。
    float EstimateLod(const Vec2 &duvDx, const Vec2 &duvDy) const;

  private:
    std::vector<TextureLevel> levels_;
};
