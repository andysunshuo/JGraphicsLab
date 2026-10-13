#pragma once
#include "Core/Math.h"
#include <cstdint>
#include <vector>

using Index = std::uint32_t;
struct Vertex
{
    Vec3 position{};
    Vec3 normal{};
    Vec2 uv{};
};
struct Mesh
{
    std::vector<Vertex> vertices;
    std::vector<Index> indices;
    // 可选的逐三角形颜色；教学立方体通过此字段保留原面色。
    std::vector<Color3> triangleColors;
};
struct Bounds
{
    Vec3 min{}, max{};
};
bool IsFinite(const Vec3 &value);
bool GetBounds(const Mesh &mesh, Bounds &bounds);
// 返回模型矩阵，不改写模型顶点；最长边映射为指定长度。
Mat4 MakeAutoFitModel(const Mesh &mesh, float targetExtent = 1.5f);
