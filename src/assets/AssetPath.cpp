#include "assets/AssetPath.h"
#include <fstream>
#include <stdexcept>
#include <windows.h>

std::filesystem::path ResolveAssetPath(const std::string &name)
{
    const auto requested = std::filesystem::u8path(name);
    if (requested.is_absolute())
        return requested;
    std::wstring executable(32768, L'\0');
    const auto length = GetModuleFileNameW(nullptr, executable.data(), static_cast<DWORD>(executable.size()));
    if (length && length < executable.size())
    {
        executable.resize(length);
        const auto candidate = std::filesystem::path(executable).parent_path() / requested;
        if (std::filesystem::is_regular_file(candidate))
            return candidate;
    }
    auto relative = requested;
    auto first = relative.begin();
    if (first != relative.end() && (*first == "assets" || *first == "Assets"))
        relative = relative.lexically_relative(*first);
    const auto candidate = std::filesystem::path(JGRAPHICSLAB_ASSET_DIR) / relative;
    if (std::filesystem::is_regular_file(candidate))
        return candidate;
    return requested;
}
std::string AssetPathUtf8(const std::filesystem::path &path)
{
    const auto text = path.u8string();
    return {text.begin(), text.end()};
}
std::string ReadAssetText(const std::filesystem::path &path, std::size_t maxBytes)
{
    std::error_code error;
    const auto size = std::filesystem::file_size(path, error);
    if (error || size > maxBytes)
        throw std::runtime_error("资源不存在、不可读取或超过大小预算：" + AssetPathUtf8(path));
    std::ifstream file(path, std::ios::binary);
    std::string text(static_cast<std::size_t>(size), '\0');
    if (!file || (size && !file.read(text.data(), static_cast<std::streamsize>(size))))
        throw std::runtime_error("资源读取失败：" + AssetPathUtf8(path));
    return text;
}
