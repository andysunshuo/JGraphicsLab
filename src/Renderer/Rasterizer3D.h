#pragma once
#include "Geometry/Primitives.h"
#include "Renderer/Clipping.h"
#include <vector>

// ============================================================
// 十五、HW31：3D Triangle Rasterization + Z Buffer
// ============================================================
// 屏幕顶点包含：二维位置、NDC 深度、RGB 顶点颜色。
struct ScreenVertex3D
{
    Point2 position;
    float ndcZ;  // 透视除法之后的 z，范围 [-1,1]
    Color3 color;
};

bool MakeScreenVertex3D(const ClipVertex& clipVertex,
    ScreenVertex3D& output);

void RasterizeTriangle3D(const ScreenVertex3D& a,
    const ScreenVertex3D& b,
    const ScreenVertex3D& c);
