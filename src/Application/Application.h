#pragma once
#include "Application/Config.h"
#include <filesystem>
#include <string>
#include <vector>
struct ApplicationOptions
{
    int homework = kActiveHomework;
    ModelDemo demo = kActiveModelDemo;
    bool allTests = false, list = false, headless = false, animate = false;
    std::filesystem::path output;
};
ApplicationOptions ParseArguments(const std::vector<std::string> &arguments);
bool InitializeApplication(const ApplicationOptions &options);
bool RenderApplicationFrame(const ApplicationOptions &options, float angle = 0.4f);
void RenderScene();

struct ApplicationDisplaySurface
{
    const Raster::Color *pixels;
    int width, height;
};
// 旧作业返回原帧缓冲；资源扩展返回独立的带中文说明画布。
ApplicationDisplaySurface GetApplicationDisplaySurface();
std::wstring ApplicationWindowTitle(const ApplicationOptions &options);
