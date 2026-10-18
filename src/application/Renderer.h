#pragma once

#include <string>
#include "material/Material.h"
#include "renderer/TriangleRasterizer.h"

// OBJ 缓存一次 -> 自动居中/等比缩放 -> Model/View/Projection -> 裁剪 -> 填充与深度。
inline constexpr const char *kDefaultModelPath = "assets/models/viking_room/viking_room.obj";
inline constexpr LightingModel kDefaultLightingModel = LightingModel::BlinnPhong;
inline constexpr const char *kDefaultMaterial = "auto";

struct SceneSettings
{
    std::string modelPath = kDefaultModelPath;
    std::string material = kDefaultMaterial; // auto 使用 OBJ 原材质；也可给独立 MTL 的 UTF-8 路径。
    std::string texture; // 可覆盖颜色贴图；相对路径按统一资产规则解析。
    LightingModel lighting = kDefaultLightingModel;
    ShadingMode shading = ShadingMode::Phong;
    bool basic = false; // 保留原面色管线，用于重构回归对照。
};
void RenderScene();
void RenderScene(const std::string &modelPath);
void RenderScene(const SceneSettings &settings);
std::string ModelPathFromName(const std::string &name);
bool SaveRenderedImage(const std::string &utf8Path); // 支持 .png 与 .ppm。

// 已接通 Light/Material/OBJ/MTL/Texture；字符串重载保留基础面色，供原测试对照。
// SceneSettings 入口缓存当前 OBJ 和当前替换材质；切换光照只重绘，不重新读盘。
// 函数补全顺序见 docs/从模型到渲染_函数补全顺序.md；模型姿态、相机与光源在 Renderer.cpp 调整。
