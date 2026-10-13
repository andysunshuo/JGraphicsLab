#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_HDR
#define STBI_NO_STDIO
#include "stb_image.h"
#include "Assets/HdrLoader.h"
#include "Assets/ResourcePaths.h"
#include <cmath>
#include <algorithm>
#include <fstream>
#include <memory>
#include <stdexcept>

namespace Assets
{
namespace
{
struct StrictMemoryReader
{
    const std::vector<unsigned char> &bytes;
    std::size_t position = 0;
    bool readPastEnd = false;
};
int ReadBytes(void *user, char *output, int requested)
{
    auto &reader = *static_cast<StrictMemoryReader *>(user);
    const auto count = std::min(static_cast<std::size_t>(requested), reader.bytes.size() - reader.position);
    if (!count && requested > 0)
        reader.readPastEnd = true;
    std::copy_n(reader.bytes.data() + reader.position, count, output);
    reader.position += count;
    return static_cast<int>(count);
}
void SkipBytes(void *user, int offset)
{
    auto &reader = *static_cast<StrictMemoryReader *>(user);
    const auto target = static_cast<std::int64_t>(reader.position) + offset;
    if (target < 0 || static_cast<std::uint64_t>(target) > reader.bytes.size())
    {
        reader.readPastEnd = true;
        return;
    }
    reader.position = static_cast<std::size_t>(target);
}
int AtEnd(void *user)
{
    const auto &reader = *static_cast<StrictMemoryReader *>(user);
    return reader.position == reader.bytes.size();
}
} // namespace
bool LoadHdr(const std::filesystem::path &path, HdrImage &image, std::string &error)
{
    image = {};
    error.clear();
    try
    {
        const auto length = std::filesystem::file_size(path);
        if (!length || length > 64 * 1024 * 1024)
            throw std::runtime_error("HDR 文件为空或超过 64 MiB 上限");
        std::ifstream file(path, std::ios::binary);
        std::vector<unsigned char> bytes(static_cast<std::size_t>(length));
        if (!file.read(reinterpret_cast<char *>(bytes.data()), static_cast<std::streamsize>(bytes.size())) ||
            file.peek() != std::char_traits<char>::eof())
            throw std::runtime_error("HDR 文件读取失败或读取期间大小发生变化");
        int width = 0, height = 0, channels = 0;
        const int size = static_cast<int>(bytes.size());
        if (!stbi_is_hdr_from_memory(bytes.data(), size) ||
            !stbi_info_from_memory(bytes.data(), size, &width, &height, &channels))
            throw std::runtime_error("文件不是可读取的 Radiance HDR 图像");
        if (width <= 0 || height <= 0 || width > 8192 || height > 8192 ||
            static_cast<std::uint64_t>(width) * height > 16 * 1024 * 1024)
            throw std::runtime_error("HDR 尺寸超过单边 8192 或总像素预算");
        const int expectedWidth = width, expectedHeight = height;
        // 保留 stb 解码算法；回调记录真正请求了文件末尾以后的数据，拒绝其补零的截断图像。
        StrictMemoryReader reader{bytes};
        const stbi_io_callbacks callbacks{ReadBytes, SkipBytes, AtEnd};
        std::unique_ptr<float, decltype(&stbi_image_free)> decoded(
            stbi_loadf_from_callbacks(&callbacks, &reader, &width, &height, &channels, 3), stbi_image_free);
        if (!decoded || reader.readPastEnd || width != expectedWidth || height != expectedHeight)
            throw std::runtime_error("HDR RGBE 解码失败");
        const auto count = static_cast<std::size_t>(width) * height;
        image.pixels.resize(count);
        for (std::size_t i = 0; i < count; ++i)
        {
            const Vec3 rgb{decoded.get()[3 * i], decoded.get()[3 * i + 1], decoded.get()[3 * i + 2]};
            if (!std::isfinite(rgb.x) || !std::isfinite(rgb.y) || !std::isfinite(rgb.z) || rgb.x < 0 ||
                rgb.y < 0 || rgb.z < 0)
                throw std::runtime_error("HDR 包含无效的线性亮度");
            image.pixels[i] = rgb;
        }
        image.width = static_cast<unsigned>(width);
        image.height = static_cast<unsigned>(height);
    }
    catch (const std::exception &e)
    {
        image = {};
        error = "加载 HDR 环境图失败：" + PathUtf8(path) + "；" + e.what();
    }
    return error.empty();
}
} // namespace Assets
