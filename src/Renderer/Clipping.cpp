#include "Renderer/Clipping.h"
#include "Renderer/Interpolation.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <stdexcept>
#include <limits>

// HW30.1 — 点相对裁剪平面的有符号距离值
// 参数 planeIndex：0左 1右 2下 3上 4近 5远。
// >=0：在半空间内部；<0：需要裁掉。
// 例如对 near 面应计算 z+w。
float EvaluateClipPlane(const Vec4& p, int planeIndex)
{
    // OpenGL 右手系，六个裁剪平面的内侧距离均要求 >=0。
    switch (planeIndex)
    {
    case 0: return p.x + p.w;  // Left
    case 1: return p.w - p.x;  // Right
    case 2: return p.y + p.w;  // Bottom
    case 3: return p.w - p.y;  // Top
    case 4: return p.z + p.w;  // Near
    case 5: return p.w - p.z;  // Far
    default: throw std::invalid_argument("EvaluateClipPlane: invalid planeIndex");
    }
}

// ------------------------------------------------------------
// HW30.3 — Sutherland-Hodgman 单平面多边形裁剪
// polygon：按边界顺序排列的一组 ClipVertex。
// planeIndex：选择六个裁剪面中的一个。
// 思路：遍历多边形边 (previous,current)：
// ① inside->inside ：保留 current；
// ② inside->outside：只添加交点；
// ③ outside->inside：先添加交点，再添加 current；
// ④ outside->outside：两点都不添加。
// 用 da=EvaluateClipPlane(a),db=EvaluateClipPlane(b)；
// 交点参数 t = da/(da-db)，注意分母为 0 的情形。
// 一条边可能生成 0、1 或 2 个输出顶点。
// ------------------------------------------------------------
// Sutherland-Hodgman：
// 将一个多边形针对一个裁剪平面进行裁剪，
// 输出一个新的多边形。
// 三角形只是输入的一种特殊情况。
std::vector<ClipVertex> ClipPolygonAgainstPlane(
    const std::vector<ClipVertex>& polygon,
    int planeIndex)
{
    // Sutherland-Hodgman：沿多边形边顺序逐条裁剪。
    std::vector<ClipVertex> output;
    if (polygon.empty()) return output;
    output.reserve(polygon.size() + 1);

    ClipVertex previous = polygon.back();
    float previousDistance = EvaluateClipPlane(previous.position, planeIndex);
    bool previousInside = (previousDistance >= 0.0f);
    for (const ClipVertex& current : polygon)
    {
        const float currentDistance = EvaluateClipPlane(current.position, planeIndex);
        const bool currentInside = (currentDistance >= 0.0f);
        if (previousInside != currentInside)
        {
            // 边跨过平面：f(t)=(1-t)*dPrev+t*dCur=0。
            const float denominator = previousDistance - currentDistance;
            if (std::abs(denominator) > 1e-12f)
            {
                const float t = previousDistance / denominator;
                output.push_back(InterpolateClipVertex(previous, current, t));
            }
        }
        if (currentInside) output.push_back(current);
        previous = current;
        previousDistance = currentDistance;
        previousInside = currentInside;
    }
    return output;
}

// HW30.4 — 依次经过全部六个裁剪面（调度代码已提供）
std::vector<ClipVertex> ClipTriangleToFrustum(
    const ClipVertex& a,
    const ClipVertex& b,
    const ClipVertex& c)
{
    std::vector<ClipVertex> polygon = { a, b, c };
    for (int plane = 0; plane < 6; ++plane)
    {
        polygon = ClipPolygonAgainstPlane(polygon, plane);
        if (polygon.empty())
            break;
    }
    return polygon; // 裁剪后可能有 0、3、4 或更多顶点
}
