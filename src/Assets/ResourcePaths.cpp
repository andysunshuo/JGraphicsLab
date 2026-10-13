#include "Assets/ResourcePaths.h"
#include <windows.h>
#include <array>
#include <stdexcept>

namespace Assets
{
std::string PathUtf8(const std::filesystem::path &path)
{
    const auto bytes = path.u8string();
    return std::string(bytes.begin(), bytes.end());
}
std::filesystem::path ExecutableDirectory()
{
    std::array<wchar_t, 32768> buffer{};
    const DWORD size = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    if (!size || size >= buffer.size())
        throw std::runtime_error("无法获取可执行文件路径");
    return std::filesystem::path(std::wstring(buffer.data(), size)).parent_path();
}
std::filesystem::path Resolve(const std::filesystem::path &relativePath)
{
    if (relativePath.is_absolute())
        return relativePath;
    const auto local = ExecutableDirectory() / "Assets" / relativePath;
    if (std::filesystem::exists(local))
        return local;
    const auto source = std::filesystem::u8path(JRENDERLAB_ASSET_DIR) / relativePath;
    if (std::filesystem::exists(source))
        return source;
    throw std::runtime_error("找不到资源：" + PathUtf8(relativePath) +
                             "；已检查程序目录 Assets 与工程资源目录");
}
} // namespace Assets
