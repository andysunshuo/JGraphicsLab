#pragma once

#include <string>

// OBJ 缓存一次 -> 自动居中/等比缩放 -> Model/View/Projection -> 裁剪 -> 填充与深度。
inline constexpr const char *kDefaultModelPath = "assets/models/triangle.obj";
void RenderScene();
void RenderScene(const std::string &modelPath);
