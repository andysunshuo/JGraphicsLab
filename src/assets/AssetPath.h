#pragma once
#include <filesystem>
#include <string>

// 绝对路径直接使用；相对路径依次搜索程序旁、编译时资产目录、调用者路径。
// OBJ、独立 MTL 和独立贴图共用此规则；MTL 内贴图相对 MTL 所在目录解析。
std::filesystem::path ResolveAssetPath(const std::string &utf8Path);
std::string ReadAssetText(const std::filesystem::path &path, std::size_t maxBytes);
std::string AssetPathUtf8(const std::filesystem::path &path);
