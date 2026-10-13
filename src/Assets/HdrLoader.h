#pragma once
#include "Core/Math.h"
#include <filesystem>
#include <string>
#include <vector>

namespace Assets
{
struct HdrImage
{
    unsigned width = 0, height = 0;
    // 线性 RGB 辐射亮度；允许分量大于 1，不能当作普通 8 位图片直接写入。
    std::vector<Vec3> pixels;
};
bool LoadHdr(const std::filesystem::path &path, HdrImage &image, std::string &error);
} // namespace Assets
