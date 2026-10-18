#include "material/Texture.h"
#include "RasterWindow.h"
#include "assets/AssetPath.h"
#include "assets/WicSupport.h"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <stdexcept>

namespace
{
constexpr std::size_t kPixelBudget = 64 * 1024 * 1024 / sizeof(Vec3);
void ValidateSize(int width, int height)
{
    if (width < 1 || height < 1 || width > 2048 || height > 2048 ||
        static_cast<std::size_t>(width) * height > kPixelBudget)
        throw std::invalid_argument("纹理尺寸超出预算：单边应为 1～2048");
}
Vec2 AddressUV(const Vec2 &uv, TextureWrap wrap)
{
    if (!uv.IsFinite())
        throw std::invalid_argument("纹理 UV 包含非有限值");
    if (wrap == TextureWrap::Repeat)
        return {uv.x - std::floor(uv.x), uv.y - std::floor(uv.y)};
    if (wrap != TextureWrap::Clamp)
        throw std::invalid_argument("纹理地址模式无效");
    return {std::clamp(uv.x, 0.f, 1.f), std::clamp(uv.y, 0.f, 1.f)};
}
const Vec3 &Texel(const TextureLevel &level, int x, int y, TextureWrap wrap)
{
    if (wrap == TextureWrap::Repeat)
    {
        x = (x % level.width + level.width) % level.width;
        y = (y % level.height + level.height) % level.height;
    }
    else
    {
        x = std::clamp(x, 0, level.width - 1);
        y = std::clamp(y, 0, level.height - 1);
    }
    return level.pixels[static_cast<std::size_t>(y) * level.width + x];
}
} // namespace

bool Texture::Load(const std::string &utf8Path, TextureEncoding encoding, std::string *error)
{
    Clear();
    try
    {
        const auto path = ResolveAssetPath(utf8Path);
        if (std::filesystem::file_size(path) > 64 * 1024 * 1024)
            throw std::runtime_error("贴图文件超过 64 MiB 预算");
        if (encoding != TextureEncoding::SrgbColor && encoding != TextureEncoding::LinearData)
            throw std::invalid_argument("贴图颜色编码无效");
        ImageIO::Apartment apartment;
        using Microsoft::WRL::ComPtr;
        ComPtr<IWICImagingFactory> factory;
        ImageIO::Check(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                                       IID_PPV_ARGS(&factory)), "无法创建 WIC 图像工厂");
        ComPtr<IWICBitmapDecoder> decoder;
        ImageIO::Check(factory->CreateDecoderFromFilename(path.c_str(), nullptr, GENERIC_READ,
                        WICDecodeMetadataCacheOnDemand, &decoder), "无法打开贴图，支持 PNG/JPEG/BMP 等 WIC 格式");
        ComPtr<IWICBitmapFrameDecode> frame;
        ImageIO::Check(decoder->GetFrame(0, &frame), "无法读取贴图第一帧");
        UINT width = 0, height = 0;
        ImageIO::Check(frame->GetSize(&width, &height), "无法读取贴图尺寸");
        if (width > 2048 || height > 2048)
            throw std::runtime_error("贴图单边超过 2048；请显式准备较小的颜色贴图");
        ValidateSize(static_cast<int>(width), static_cast<int>(height));
        ComPtr<IWICFormatConverter> converter;
        ImageIO::Check(factory->CreateFormatConverter(&converter), "无法创建贴图颜色转换器");
        ImageIO::Check(converter->Initialize(frame.Get(), GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone,
                                            nullptr, 0, WICBitmapPaletteTypeCustom), "无法转换贴图像素格式");
        std::vector<unsigned char> rgba(static_cast<std::size_t>(width) * height * 4);
        ImageIO::Check(converter->CopyPixels(nullptr, width * 4, static_cast<UINT>(rgba.size()), rgba.data()),
                       "读取贴图像素失败");
        TextureLevel level{static_cast<int>(width), static_cast<int>(height), {}};
        level.pixels.resize(static_cast<std::size_t>(width) * height);
        for (std::size_t i = 0; i < level.pixels.size(); ++i)
        {
            const auto *p = rgba.data() + i * 4;
            level.pixels[i] = encoding == TextureEncoding::SrgbColor
                ? Raster::DecodeSrgbColor(Raster::MakeColor(p[0], p[1], p[2]))
                : Vec3{p[0] / 255.f, p[1] / 255.f, p[2] / 255.f};
        }
        levels_.push_back(std::move(level));
        if (error)
            error->clear();
        return true;
    }
    catch (const std::exception &e)
    {
        Clear();
        if (error)
            *error = "贴图加载失败（" + utf8Path + "）：" + e.what();
        return false;
    }
}
void Texture::Create(int width, int height, const std::vector<Vec3> &pixels)
{
    ValidateSize(width, height);
    if (pixels.size() != static_cast<std::size_t>(width) * height ||
        !std::all_of(pixels.begin(), pixels.end(), [](const auto &p) { return p.IsFinite(); }))
        throw std::invalid_argument("程序纹理像素数量错误或包含非有限值");
    std::vector<TextureLevel> candidate{{width, height, pixels}};
    levels_.swap(candidate);
}
void Texture::Clear() { levels_.clear(); }
bool Texture::Empty() const { return levels_.empty(); }
int Texture::Width() const { return Empty() ? 0 : levels_[0].width; }
int Texture::Height() const { return Empty() ? 0 : levels_[0].height; }
std::size_t Texture::LevelCount() const { return levels_.size(); }
const TextureLevel &Texture::Level(std::size_t index) const
{
    if (index >= levels_.size())
        throw std::out_of_range("纹理层级越界或纹理为空");
    return levels_[index];
}
Vec3 Texture::SampleNearest(const Vec2 &uv, std::size_t index, TextureWrap wrap) const
{
    const auto& level = Level(index);
    const auto p = AddressUV(uv, wrap);
    return Texel(level, static_cast<int>(std::floor(p.x * level.width)),
        static_cast<int>(std::floor((1 - p.y) * level.height)), wrap);
}   
Vec3 Texture::SampleBilinear(const Vec2 &uv, std::size_t index, TextureWrap wrap) const
{
    const auto& level = Level(index);
    const auto p = AddressUV(uv, wrap);
    const float s = p.x * level.width - .5f, t = (1 - p.y) * level.height - .5f;
    const int x = static_cast<int>(std::floor(s)), y = static_cast<int>(std::floor(t));
    return Vec3::Lerp(Vec3::Lerp(Texel(level, x, y, wrap), Texel(level, x + 1, y, wrap), s - x),
        Vec3::Lerp(Texel(level, x, y + 1, wrap), Texel(level, x + 1, y + 1, wrap), s - x), t - y);
}
Vec3 Texture::SampleTrilinear(const Vec2 &uv, float lod, TextureWrap wrap) const
{
    if (Empty() || !std::isfinite(lod))
        throw std::invalid_argument("三线性采样要求有效纹理和有限 LOD");
    lod = std::clamp(lod, 0.f, static_cast<float>(LevelCount() - 1));
    const auto a = static_cast<std::size_t>(std::floor(lod)), b = std::min(a + 1, LevelCount() - 1);
    return Vec3::Lerp(SampleBilinear(uv, a, wrap), SampleBilinear(uv, b, wrap), lod - static_cast<float>(a));
}
Vec3 Texture::Sample(const Vec2 &uv, TextureFilter filter, TextureWrap wrap, float lod) const
{
    if (!std::isfinite(lod))
        throw std::invalid_argument("纹理 LOD 必须有限");
    switch (filter)
    {
    case TextureFilter::Nearest: return SampleNearest(uv, 0, wrap);
    case TextureFilter::Bilinear: return SampleBilinear(uv, 0, wrap);
    case TextureFilter::Trilinear: return SampleTrilinear(uv, lod, wrap);
    default: throw std::invalid_argument("纹理过滤模式无效");
    }
}
void Texture::GenerateMipmaps()
{
    if (Empty())
        throw std::invalid_argument("空纹理不能生成 Mipmap");
    levels_.resize(1);
    std::size_t total = levels_[0].pixels.size();
    while (levels_.back().width > 1 || levels_.back().height > 1)
    {
        const auto &previous = levels_.back();
        TextureLevel next{(previous.width + 1) / 2, (previous.height + 1) / 2, {}};
        total += static_cast<std::size_t>(next.width) * next.height;
        if (total > kPixelBudget)
            throw std::runtime_error("Mipmap 总像素超过 64 MiB 预算");
        next.pixels.resize(static_cast<std::size_t>(next.width) * next.height);
        for (int y = 0; y < next.height; ++y)
            for (int x = 0; x < next.width; ++x)
            {
                Vec3 sum{};
                int count = 0;
                for (int j = 0; j < 2; ++j)
                    for (int i = 0; i < 2; ++i)
                        if (2 * x + i < previous.width && 2 * y + j < previous.height)
                        {
                            sum += previous.pixels[static_cast<std::size_t>(2 * y + j) * previous.width + 2 * x + i];
                            ++count;
                        }
                next.pixels[static_cast<std::size_t>(y) * next.width + x] = sum / static_cast<float>(count);
            }
        levels_.push_back(std::move(next));
    }
}
float Texture::EstimateLod(const Vec2 &dx, const Vec2 &dy) const
{
    if (Empty() || !dx.IsFinite() || !dy.IsFinite())
        throw std::invalid_argument("LOD 估计要求有效纹理和有限导数");
    const float rho = std::max(std::hypot(dx.x * Width(), dx.y * Height()),
                               std::hypot(dy.x * Width(), dy.y * Height()));
    if (!std::isfinite(rho))
        throw std::invalid_argument("LOD 导数过大");
    return std::log2(std::max(rho, 1e-8f));
}
