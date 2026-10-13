#pragma once
#include "Core/Color.h"
#include "Assets/TextureLoader.h"
#include "Assets/HdrLoader.h"
#include <filesystem>
#include <string>
#include <vector>

// 资源展示专用画布，不更改原作业的固定分辨率帧缓冲。
struct AssetPreviewFrame
{
    int width = 600, height = 600;
    std::vector<Raster::Color> pixels;
};
namespace AssetPreview
{
void Begin(AssetPreviewFrame &frame);
void Caption(AssetPreviewFrame &frame, const std::wstring &title, const std::wstring &description,
             const std::wstring &path, const std::wstring &footer);
void Image(AssetPreviewFrame &frame, const Raster::Color *pixels, int width, int height, int x, int y,
           int boxWidth, int boxHeight);
void Environment(AssetPreviewFrame &frame, const Assets::HdrImage &image);
bool SavePPM(const AssetPreviewFrame &frame, const std::filesystem::path &path);
} // namespace AssetPreview
