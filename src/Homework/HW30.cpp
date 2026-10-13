/*
【HW30 学习说明：视锥裁剪】
运行选择：Config.h 中 kActiveHomework=30，或启动参数 --hw 30。
当前状态：已有实现。以下调用说明对应当前代码，不自动实现缺失作业。
测试调度：InitializeApplication -> RunMathHomeworkTest(30) -> TestHW30。
绘制调度：InitializeApplication -> RenderApplicationFrame -> RunHomework(30,false) -> RenderHW30。
顺序：先运行数学检查，再清空检查可能写入的颜色/深度，最后调用本章绘制入口。

【调用流程、数学原理与计算过程】
测试调用：TestHW30 -> EvaluateClipPlane、ClipTriangleToFrustum、InterpolateClipVertex，再 near4/near/check。
渲染调用：RenderHW30 -> DrawNearClippedTriangleDemo -> MakePerspective -> Multiply -> ClipTriangleToFrustum；
内部逐六平面 ClipPolygonAgainstPlane -> EvaluateClipPlane -> InterpolateClipVertex；
裁剪后 PerspectiveDivide -> ViewportTransform -> 三角扇 FillTriangleColorInterpolation，外轮廓 DrawLine。
数学原理：保持齐次坐标，用 x+w、w-x、y+w、w-y、z+w、w-z 的非负侧定义六个内半空间。
穿过平面的边以 dA、dB 求交点 t=dA/(dA-dB)，新位置和颜色为 (1-t)*A+t*B；先裁剪才能安全除 w。
测试 near 例：A=(0,0,-2,1) 的 dA=-1，B=(0.6,-0.4,0,1) 的 dB=1，t=0.5，交点=(0.3,-0.2,-1,1)。
一个顶点在 near 外的测试三角形裁成四边形；三角扇以 (0,1,2)、(0,2,3) 绘制。
图形演示 near=0.5，其中相机顶点 z=-0.2 在近面前方，另外两点 z=-2；不做深度测试或透视正确属性插值。
本章共 6 项检查。断点观察 plane、previousDistance/currentDistance、t、polygon 与 screen。

【阅读与调试】
F11 进入本章下述调用的公共实现；在本文件入口设置断点，可检查本章数据与变量。
公共算法不会被复制到每个 HW；本次只补说明，运算语句保持不变。
*/

#include "Homework/Entries.h"
#include "Geometry/Primitives.h"
#include "Homework/DemoData.h"
#include "Renderer/Rasterizer2D.h"
#include "Core/Math.h"
#include "Renderer/Interpolation.h"
#include "Camera/Transforms.h"
#include "Renderer/Clipping.h"
#include "Renderer/Framebuffer.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <stdexcept>
#include <limits>

// HW30.5 — 显示近裁剪面测试三角形
// 三个顶点直接放在相机空间：其中第一个位于 near 前方，应被裁掉一部分。
// 与 HW29 不同，这里不能先做 PerspectiveDivide 再执行裁剪。
// 绘制辅助入口：投影相机空间三角形，先六平面裁剪，再做透视除法、视口映射和三角扇绘制。
// 可在函数内观察每一步的矩阵、顶点、裁剪结果或屏幕位置；详细计算见文件顶部。
void DrawNearClippedTriangleDemo()
{
    const Mat4 projection =
        MakePerspective(1.04719755f, static_cast<float>(Raster::WIDTH) / Raster::HEIGHT, 0.5f, 10.0f);

    const ClipVertex a = {Multiply(projection, Vec4{0.0f, 0.0f, -0.2f, 1.0f}), {255.0f, 60.0f, 60.0f}};
    const ClipVertex b = {Multiply(projection, Vec4{0.65f, -0.45f, -2.0f, 1.0f}), {60.0f, 255.0f, 60.0f}};
    const ClipVertex c = {Multiply(projection, Vec4{0.0f, 0.65f, -2.0f, 1.0f}), {60.0f, 60.0f, 255.0f}};

    // 第 1 阶段：投影后的三角形逐六平面裁剪，保留交点和插值颜色，不提前除 w。
    const std::vector<ClipVertex> polygon = ClipTriangleToFrustum(a, b, c);
    if (polygon.size() < 3)
        return;

    // 先把裁剪后的每个顶点变换到屏幕空间，再把多边形分割为三角扇。
    // HW11 的 RGB 插值可作为本章的颜色可视化（并非透视校正属性插值）。
    std::vector<Point2> screen;
    screen.reserve(polygon.size());
    // 第 2 阶段：裁剪完成后才除 w，并把 NDC 映射到连续屏幕坐标。
    for (const ClipVertex &vertex : polygon)
    {
        Vec3 ndc = {};
        if (!PerspectiveDivide(vertex.position, ndc))
            return;
        screen.push_back(ViewportTransform(ndc, Raster::WIDTH, Raster::HEIGHT));
    }
    // 第 3 阶段：用固定首点与相邻两点拆成三角扇，调用 HW11 的 RGB 插值填充。
    for (std::size_t j = 1; j + 1 < screen.size(); ++j)
    {
        const Triangle triangle = {screen[0], screen[j], screen[j + 1]};
        FillTriangleColorInterpolation(triangle, polygon[0].color, polygon[j].color, polygon[j + 1].color);
    }
    // 最后只画外轮廓；不把三角扇内部对角线画成白线。
    for (std::size_t i = 0; i < screen.size(); ++i)
        DrawLine(screen[i], screen[(i + 1) % screen.size()], white);
}

// 数学入口：按下列顺序调用本章算法；near/nearN 输出实际与预期值，check 汇总 PASS/FAIL。
// 返回 TestResult；本章具体公式与逐步数值见文件顶部学习说明。
TestResult TestHW30()
{
    const int hw = 30;

    constexpr float halfPi = 3.14159265358979323846f * 0.5f;
    MathTestContext checks(hw);

    {
        const Vec4 inside = {0, 0, 0, 1};
        bool sixPositive = true;
        for (int plane = 0; plane < 6; ++plane)
            sixPositive &= (EvaluateClipPlane(inside, plane) >= 0.0f);
        checks.check(sixPositive, "Clip: center inside all six planes");
        checks.check(checks.near(EvaluateClipPlane({0, 0, -2, 1}, 4), -1.0f), "Clip: near plane z+w");
        const ClipVertex va = {{0, 0, -2, 1}, {255, 0, 0}};
        const ClipVertex vb = {{0.6f, -0.4f, 0, 1}, {0, 255, 0}};
        const ClipVertex vc = {{-0.6f, 0.4f, 0, 1}, {0, 0, 255}};
        const auto polygon = ClipTriangleToFrustum(va, vb, vc);
        checks.check(polygon.size() == 4, "Near clipping triangle -> quad (4 vertices)");
        bool allInside = true;
        for (const ClipVertex &v : polygon)
            for (int plane = 0; plane < 6; ++plane)
                if (EvaluateClipPlane(v.position, plane) < -1e-4f)
                    allInside = false;
        checks.check(allInside, "Clipped polygon entirely inside all six planes");
        const ClipVertex midpoint = InterpolateClipVertex(va, vb, 0.5f);
        checks.check(checks.near4(midpoint.position, Vec4{0.3f, -0.2f, -1, 1}) &&
                         checks.near(midpoint.color.r, 127.5f) && checks.near(midpoint.color.g, 127.5f),
                     "Clip edge interpolates Vec4 and RGB");
        checks.check(ClipTriangleToFrustum({{5, 0, 0, 1}, {255, 0, 0}}, {{6, 0, 0, 1}, {0, 255, 0}},
                                           {{5, 1, 0, 1}, {0, 0, 255}})
                         .empty(),
                     "Clip triangle completely outside right plane");
    }
    std::printf("[HW%02d] 结果：%d PASS, %d FAIL\n\n", hw, checks.result.passed, checks.result.failed);
    return {checks.result.passed, checks.result.failed};
}

// 作业绘制入口：Runner 已清背景和深度，随后执行下列调用。
// 具体调用链、采样/变换过程和观察结果见文件顶部学习说明。
void RenderHW30()
{

    DrawNearClippedTriangleDemo();
}
