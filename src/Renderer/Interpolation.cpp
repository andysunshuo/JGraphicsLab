#include "Renderer/Interpolation.h"
#include "Core/Math.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <stdexcept>
#include <limits>

// ============================================================
// HW11 — Color Interpolation
// 三角形颜色插值
// ============================================================

// 功能：
// 根据重心坐标，计算三角形内部点的 RGB 颜色。
//
// 参数：
// bary：重心坐标 alpha、beta、gamma。
// a   ：顶点 A 的颜色。
// b   ：顶点 B 的颜色。
// c   ：顶点 C 的颜色。
//
// 返回值：
// 插值计算得到的 Raster::Color。
//
// 调用关系：
// 先执行 ComputeBarycentric()。
// 再将结果交给 InterpolateColor()。
//
// 数学公式：
// C = alpha*Ca + beta*Cb + gamma*Cc
//
// R、G、B 三个通道分别插值。
//
// 示例：
// A = 红色
// B = 绿色
// C = 蓝色
//
// 目标：
// 三角形内部形成 RGB 平滑渐变。
//
// 注意：
// 当前 Color3 使用 0~255 的 float。
// 返回 Raster::Color 前需要处理通道范围和整数转换。
Raster::Color InterpolateColor(
    const Barycentric& bary,
    const Color3& a,
    const Color3& b,
    const Color3& c)
{
    // RGB 插值

    float r =
        bary.alpha * a.r +
        bary.beta * b.r +
        bary.gamma * c.r;


    float g =
        bary.alpha * a.g +
        bary.beta * b.g +
        bary.gamma * c.g;


    float bValue =
        bary.alpha * a.b +
        bary.beta * b.b +
        bary.gamma * c.b;


    // 限制颜色范围
    r = Clamp(r, 0.0f, 255.0f);
    g = Clamp(g, 0.0f, 255.0f);
    bValue = Clamp(bValue, 0.0f, 255.0f);


    // 转换成最终 framebuffer 使用的颜色
    return Raster::MakeColor(
        static_cast<int>(r),
        static_cast<int>(g),
        static_cast<int>(bValue)
    );
}

// HW30.2 — 沿边插值生成交点
// t 在 0~1 之间时，返回 a+(b-a)*t 的位置和颜色。
// 新顶点不仅插值 x/y/z，也必须插值 w！
// 当三角形被裁掉一部分时，计算新的顶点。 
ClipVertex InterpolateClipVertex(const ClipVertex& a,
    const ClipVertex& b,
    float t)
{
    // 在四维齐次空间中做边交点插值，必须保留 w。
    const float invT = 1.0f - t;
    return {
        {
            a.position.x * invT + b.position.x * t,
            a.position.y * invT + b.position.y * t,
            a.position.z * invT + b.position.z * t,
            a.position.w * invT + b.position.w * t
        },
        {
            a.color.r * invT + b.color.r * t,
            a.color.g * invT + b.color.g * t,
            a.color.b * invT + b.color.b * t
        },
        Add(Multiply(a.worldPosition,invT),Multiply(b.worldPosition,t)),
        Add(Multiply(a.normal,invT),Multiply(b.normal,t)),
        Add(Multiply(a.uv,invT),Multiply(b.uv,t))
    };
}
