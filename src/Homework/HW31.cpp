/*
【HW31 学习说明：三维光栅化与深度】
运行选择：Config.h 中 kActiveHomework=31，或启动参数 --hw 31。
当前状态：已有实现。以下调用说明对应当前代码，不自动实现缺失作业。
测试调度：InitializeApplication -> RunMathHomeworkTest(31) -> TestHW31。
绘制调度：InitializeApplication -> RenderApplicationFrame -> RunHomework(31,false) -> RenderHW31。
顺序：先运行数学检查，再清空检查可能写入的颜色/深度，最后调用本章绘制入口。

【调用流程、数学原理与计算过程】
测试调用：TestHW31 -> MakeScreenVertex3D -> PerspectiveDivide/ViewportTransform；
随后 ClearDepthBuffer、三次 RasterizeTriangle3D 并检查 depthBuffer，共 5 项检查。
渲染调用：RenderHW31 -> DrawSolidCubeMVP(1.2) -> 首次构造静态 Mesh -> MakeModelMatrix -> DrawMesh；
DrawMesh 内部 Camera::View/Projection -> LookAt/MakePerspective -> Multiply -> ClipTriangleToFrustum；
每个裁剪多边形拆为三角扇，MakeScreenVertex3D -> RasterizeTriangle3D -> EdgeFunction -> DepthTest；
深度通过后 InterpolateColor -> Clamp/MakeColor -> Raster::PutPixel，结果写入公共帧缓冲。
数学原理：将绕序统一为 CCW，以 Top-left 规则决定共享边覆盖，再用边值/总面积得到重心权重。
深度 ndcZ=alpha*z0+beta*z1+gamma*z2，depth=ndcZ*0.5+0.5；NDC 深度对屏幕重心权重可线性插值。
测试先绘蓝色远三角形 zNDC=0.6 -> depth=0.8，再绘红色近三角形 -0.4 -> 0.3，最后远面不能覆盖近面。
Mesh 使用原 8 顶点、36 索引/12 三角形、6 面色；每相邻两个三角形共用一面色，只构造一次，不加载 cube.obj。
本章保留原矩阵/相机/固定姿态，没有默认背面剔除或三维 MSAA；屏幕包围盒裁至 0～99。
在本文件观察 mesh/model/options，在 MeshRenderer.cpp 观察 polygon，在 Rasterizer3D.cpp 观察 area、weights、depth。

【阅读与调试】
F11 进入本章下述调用的公共实现；在本文件入口设置断点，可检查本章数据与变量。
公共算法不会被复制到每个 HW；本次只补说明，运算语句保持不变。
*/

#include "Homework/Entries.h"
#include "Geometry/Primitives.h"
#include "Homework/DemoData.h"
#include "Renderer/DepthBuffer.h"
#include "Camera/Transforms.h"
#include "Renderer/Rasterizer3D.h"
#include "Renderer/MeshRenderer.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <stdexcept>
#include <limits>

// HW31.3 — 完整绘制调度：12 三角形 -> 齐次裁剪 -> 深度光栅化。
// 全部调度和光栅化均已实现。
// 绘制辅助入口：首次创建原教学 Cube 的静态 Mesh，建立 Model 与面色选项后交给通用 DrawMesh。
// 可在函数内观察每一步的矩阵、顶点、裁剪结果或屏幕位置；详细计算见文件顶部。
void DrawSolidCubeMVP(float angleRadians)
{
    // 原顶点、三角形顺序、面色及相机参数保持一致；网格只构造一次。
    // 第 1 阶段：仅首次调用初始化 Mesh；每两个三角形分配原立方体同一面色。
    static const Mesh mesh = [] {
        Mesh m;
        for (const Vec3 &position : hwCubeVertices)
            m.vertices.push_back({position, {}, {}});
        for (int index : hwCubeTriangleIndices)
            m.indices.push_back(static_cast<Index>(index));
        for (int tri = 0; tri < 12; ++tri)
            m.triangleColors.push_back(hwCubeFaceColors[tri / 2]);
        return m;
    }();
    const Mat4 model = MakeModelMatrix({0, 0, 0}, {0.25f, angleRadians, 0}, {1, 1, 1});
    RenderOptions options;
    options.colorMode = MeshColorMode::FaceColor;
    // 第 2 阶段：进入通用管线，按 MVP -> 六平面裁剪 -> 屏幕光栅化 -> 深度与颜色写入执行。
    DrawMesh(mesh, model, Camera{}, options);
}

// 数学入口：按下列顺序调用本章算法；near/nearN 输出实际与预期值，check 汇总 PASS/FAIL。
// 返回 TestResult；本章具体公式与逐步数值见文件顶部学习说明。
TestResult TestHW31()
{
    const int hw = 31;

    constexpr float halfPi = 3.14159265358979323846f * 0.5f;
    MathTestContext checks(hw);

    {
        const ClipVertex cv = {{0, 0, 0, 1}, {255, 0, 0}};
        ScreenVertex3D sv = {};
        checks.check(MakeScreenVertex3D(cv, sv) && checks.near(sv.position.x, 50) &&
                         checks.near(sv.position.y, 50) && checks.near(sv.ndcZ, 0),
                     "ClipVertex -> ScreenVertex3D at (50,50)");
        ClearDepthBuffer();
        const ScreenVertex3D far0 = {{10, 10}, 0.6f, {0, 0, 255}};
        const ScreenVertex3D far1 = {{90, 10}, 0.6f, {0, 0, 255}};
        const ScreenVertex3D far2 = {{50, 90}, 0.6f, {0, 0, 255}};
        // 绘制远面：NDC 深度 0.6 映射为 0.8；之后再用近面与重复远面验证遮挡。
        RasterizeTriangle3D(far0, far1, far2);
        checks.check(checks.near(depthBuffer[30][50], 0.8f), "Rasterize: far triangle writes depth=0.8");
        const ScreenVertex3D near0 = {{10, 10}, -0.4f, {255, 0, 0}};
        const ScreenVertex3D near1 = {{90, 10}, -0.4f, {255, 0, 0}};
        const ScreenVertex3D near2 = {{50, 90}, -0.4f, {255, 0, 0}};
        RasterizeTriangle3D(near0, near1, near2);
        checks.check(checks.near(depthBuffer[30][50], 0.3f),
                     "Rasterize: nearer triangle overwrites depth=0.3");
        RasterizeTriangle3D(far0, far1, far2);
        checks.check(checks.near(depthBuffer[30][50], 0.3f), "Z-Buffer: farther triangle cannot overwrite");
        checks.check(checks.near(depthBuffer[0][0], 1.0f), "Rasterize: outside triangle depth unchanged");
    }
    std::printf("[HW%02d] 结果：%d PASS, %d FAIL\n\n", hw, checks.result.passed, checks.result.failed);
    return {checks.result.passed, checks.result.failed};
}

// 作业绘制入口：Runner 已清背景和深度，随后执行下列调用。
// 具体调用链、采样/变换过程和观察结果见文件顶部学习说明。
void RenderHW31()
{

    DrawSolidCubeMVP(1.2f);
}
