#pragma once
#include "math/Vec2.h"
#include "math/Vec3.h"
#include "math/Vec4.h"

// 本文件只定义数据，无需额外 .cpp；三个阶段的顶点类型不能混用。
// 保留已有 Vertex 字段顺序 position、uv、normal，兼容 OBJLoader.cpp。
struct Vertex
{
    Vec3 position{};     // 模型空间位置，不是已经除过 w 的 NDC。
    Vec2 uv{};           // U 向右、V 向上，接缝处需拆分统一顶点。
    Vec3 normal{};       // 模型空间法线，OBJ 缺失时为零，之后可生成。
    Vec3 color{1, 1, 1}; // 归一化线性 RGB；最终显示时才编码为 sRGB。
};

struct ClipVertex
{
    Vec4 position{}; // P*V*M*Position；保留 w，先裁剪再除 w。
    Vec3 worldPosition{};
    Vec3 normal{}; // 世界空间法线，裁剪时线性插值，着色时归一化。
    Vec2 uv{};
    Vec3 color{1, 1, 1};
};

// 保留 x、y 两个首字段，现有 RasterizeTriangle.cpp 仍然直接使用它们。
struct ScreenVertex
{
    float x = 0, y = 0;    // 左下原点、Y 向上，尚未取整的屏幕位置。
    float ndcZ = 0;        // [-1,1]，深度映射为 ndcZ*0.5+0.5。
    float reciprocalW = 1; // 保存 1/clip.w，供透视正确属性插值。
    Vec3 worldPosition{};
    Vec3 normal{};
    Vec2 uv{};
    Vec3 color{1, 1, 1};
};
