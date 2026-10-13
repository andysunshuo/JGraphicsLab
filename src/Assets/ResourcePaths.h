#pragma once
#include <filesystem>

namespace Assets
{
std::string PathUtf8(const std::filesystem::path &path);
std::filesystem::path ExecutableDirectory();
// 优先可执行文件旁 Assets，其次编译时的源目录；不使用当前工作目录。
std::filesystem::path Resolve(const std::filesystem::path &relativePath);
} // namespace Assets
