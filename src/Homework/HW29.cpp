/*
【HW29 学习说明：线框立方体 MVP】
运行选择：Config.h 中 kActiveHomework=29，或启动参数 --hw 29。
当前状态：已有实现。以下调用说明对应当前代码，不自动实现缺失作业。
测试调度：InitializeApplication -> RunMathHomeworkTest(29) -> TestHW29。
绘制调度：InitializeApplication -> RenderApplicationFrame -> RunHomework(29,false) -> RenderHW29。
顺序：先运行数学检查，再清空检查可能写入的颜色/深度，最后调用本章绘制入口。

【调用流程、数学原理与计算过程】
测试调用：TestHW29 -> MakeModelMatrix、LookAt、MakePerspective、Multiply、MakePosition4、PerspectiveDivide；
统计 8 个顶点均位于 NDC [-1,1]，再检查 24 个边索引组成 12 条边，共 2 项检查。
渲染调用：RenderHW29 -> DrawWireframeCubeMVP(1.2) -> 同样建立 M/V/P 与 MVP=P*V*M；
逐顶点 MakePosition4 -> Multiply -> PerspectiveDivide -> ViewportTransform，最后按 hwCubeEdges 调 DrawLine -> PutPixel。
数学原理：模型坐标先到世界，再到相机、裁剪空间，除 w 后映射屏幕；组合矩阵仍按列向量右侧先作用。
默认 8 点立方体中心为原点、边长 1，模型绕 X 0.25 弧度、绕 Y 1.2 弧度；相机在 (1.7,1.3,3)。
投影 FOV=60°、aspect=1、near=0.1、far=20；不填面、不做深度测试，隐藏边也可见。
此线框演示不执行边裁剪，依赖示例处于视锥内；默认是固定旋转姿态，不是自动动画。
在本文件观察 model/view/projection、clip/ndc、screen[8]、valid[8] 与当前边 a/b。

【阅读与调试】
F11 进入本章下述调用的公共实现；在本文件入口设置断点，可检查本章数据与变量。
公共算法不会被复制到每个 HW；本次只补说明，运算语句保持不变。
*/

#include "Homework/Entries.h"
#include "Geometry/Primitives.h"
#include "Homework/DemoData.h"
#include "Renderer/Rasterizer2D.h"
#include "Core/Math.h"
#include "Camera/Transforms.h"
#include "Renderer/Framebuffer.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <stdexcept>
#include <limits>

// 调度函数：通过 HW17~HW28 中完整实现的数学函数进行变换。
// 随着各模块完成，立方体将自动能够显示。
// angleRadians：控制模型绕 Y 轴旋转的角度。
// 绘制辅助入口：建立 MVP，变换 8 个角点，再按 12 条边索引调用直线光栅化；角度单位为弧度。
// 可在函数内观察每一步的矩阵、顶点、裁剪结果或屏幕位置；详细计算见文件顶部。
void DrawWireframeCubeMVP(float angleRadians)
{
    const Vec3 position = {0.0f, 0.0f, 0.0f};
    const Vec3 rotation = {0.25f, angleRadians, 0.0f};
    const Vec3 scale = {1.0f, 1.0f, 1.0f};

    const Mat4 model = MakeModelMatrix(position, rotation, scale);
    const Mat4 view = LookAt({1.7f, 1.3f, 3.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f});
    const float aspect = static_cast<float>(Raster::WIDTH) / static_cast<float>(Raster::HEIGHT);
    const Mat4 projection = MakePerspective(1.04719755f, aspect, 0.1f, 20.0f);

    // 注意：MVP=P*V*M。行向量约定下，顺序才会不同。
    // 第 1 阶段：把 Model 与 View 组合，再左乘 Projection 得到 MVP。
    const Mat4 modelView = Multiply(view, model);
    const Mat4 mvp = Multiply(projection, modelView);

    Point2 screen[8] = {};
    bool valid[8] = {};

    // 第 2 阶段：8 个位置补 w=1 -> 乘 MVP -> 除 w -> 映射到屏幕；暂不做边裁剪。
    for (int i = 0; i < 8; ++i)
    {
        const Vec4 local = MakePosition4(hwCubeVertices[i]);
        const Vec4 clip = Multiply(mvp, local);
        Vec3 ndc = {};
        if (!PerspectiveDivide(clip, ndc))
            continue;

        // HW29 的示例物体完全位于视锥内；这里暂不实现边裁剪。
        screen[i] = ViewportTransform(ndc, Raster::WIDTH, Raster::HEIGHT);
        valid[i] = true;
    }

    // 第 3 阶段：每两个边索引取屏幕端点，调用 Bresenham 画线。
    for (int i = 0; i < 24; i += 2)
    {
        const int a = hwCubeEdges[i];
        const int b = hwCubeEdges[i + 1];
        if (valid[a] && valid[b])
            DrawLine(screen[a], screen[b], white);
    }
}

// 数学入口：按下列顺序调用本章算法；near/nearN 输出实际与预期值，check 汇总 PASS/FAIL。
// 返回 TestResult；本章具体公式与逐步数值见文件顶部学习说明。
TestResult TestHW29()
{
    const int hw = 29;

    constexpr float halfPi = 3.14159265358979323846f * 0.5f;
    MathTestContext checks(hw);

    {
        const Mat4 model = MakeModelMatrix({0, 0, 0}, {0.25f, 1.2f, 0}, {1, 1, 1});
        const Mat4 view = LookAt({1.7f, 1.3f, 3}, {0, 0, 0}, {0, 1, 0});
        const Mat4 proj = MakePerspective(1.04719755f, 1.0f, 0.1f, 20.0f);
        const Mat4 mvp = Multiply(proj, Multiply(view, model));
        int count = 0;
        for (const Vec3 &vertex : hwCubeVertices)
        {
            const Vec4 clip = Multiply(mvp, MakePosition4(vertex));
            Vec3 ndc = {};
            if (PerspectiveDivide(clip, ndc) && ndc.x >= -1.0f && ndc.x <= 1.0f && ndc.y >= -1.0f &&
                ndc.y <= 1.0f && ndc.z >= -1.0f && ndc.z <= 1.0f)
                ++count;
        }
        checks.check(count == 8, "MVP: all 8 cube corners are in the frustum");
        checks.check(sizeof(hwCubeEdges) / sizeof(hwCubeEdges[0]) == 24, "Wireframe: 12 edges");
    }
    std::printf("[HW%02d] 结果：%d PASS, %d FAIL\n\n", hw, checks.result.passed, checks.result.failed);
    return {checks.result.passed, checks.result.failed};
}

// 作业绘制入口：Runner 已清背景和深度，随后执行下列调用。
// 具体调用链、采样/变换过程和观察结果见文件顶部学习说明。
void RenderHW29()
{

    DrawWireframeCubeMVP(1.2f);
}
