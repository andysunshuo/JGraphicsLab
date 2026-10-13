#pragma once
#include "Core/Math.h"
#include <vector>

// ============================================================
// 十四、HW30：Frustum Clipping（齐次裁剪空间）
// ============================================================
// 六个平面：
// LEFT   : x + w >= 0     RIGHT : w - x >= 0
// BOTTOM : y + w >= 0     TOP   : w - y >= 0
// NEAR   : z + w >= 0     FAR   : w - z >= 0
// 注意：此处 OpenGL 风格 z_clip ∈ [-w,+w]。

struct ClipVertex
{
    Vec4 position;   // 裁剪空间的四维坐标
    Color3 color;    // 顶点颜色（裁剪新交点也需要插值）
    // 新作业的可插值属性；默认零值保持原 HW30/HW31 聚合初始化兼容。
    Vec3 worldPosition{};
    Vec3 normal{};
    Vec2 uv{};
};

float EvaluateClipPlane(const Vec4& p, int planeIndex);

std::vector<ClipVertex> ClipPolygonAgainstPlane(
    const std::vector<ClipVertex>& polygon,
    int planeIndex);

std::vector<ClipVertex> ClipTriangleToFrustum(
    const ClipVertex& a,
    const ClipVertex& b,
    const ClipVertex& c);
