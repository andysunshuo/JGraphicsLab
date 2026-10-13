#pragma once
#include "Core/Color.h"
#include <filesystem>
#include <vector>
#include <string>
namespace Assets
{
struct Texture
{
    unsigned width = 0, height = 0;
    // 按图像文件从上到下存储；当前 Mesh 管线尚未采样纹理。
    std::vector<Raster::Color> pixels;
};
bool LoadTexture(const std::filesystem::path &path, Texture &texture, std::string &error);
} // namespace Assets
