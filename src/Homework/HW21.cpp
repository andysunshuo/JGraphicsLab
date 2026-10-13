/*
【HW21 学习说明：二维变换】
运行选择：Config.h 中 kActiveHomework=21，或启动参数 --hw 21。
当前状态：已有实现。以下调用说明对应当前代码，不自动实现缺失作业。
测试调度：InitializeApplication -> RunMathHomeworkTest(21) -> TestHW21。
绘制调度：InitializeApplication -> RenderApplicationFrame -> RunHomework(21,false) -> RenderHW21。
顺序：先运行数学检查，再清空检查可能写入的颜色/深度，最后调用本章绘制入口。

【调用流程、数学原理与计算过程】
测试调用：TestHW21 -> 六类 Make...2D 变换、Multiply、TransformPoint2D -> MathTestContext 检查。
渲染调用：RenderHW21 -> DrawTransform2DDemo -> MakeTranslation2D/MakeRotation2D/MakeScale2D；
随后组合 T*R*S，对每条边的两点调用 TransformPoint2D，再由 DrawLine -> PutPixel。
数学原理：二维位置补为 (x,y,1)；列向量使右侧先作用。旋转 x'=cosθ*x-sinθ*y，y'=sinθ*x+cosθ*y。
反射 X 轴时翻转 y，反射 Y 轴时翻转 x；错切 x'=x+shearX*y，y'=y+shearY*x。
测试组合：T(10,20)*R(90°)*S(2,3)*(1,0)：(2,0)->(0,2)->(10,22)，共 8 项检查。
演示组合为平移 (50,50)、旋转 0.5 弧度、缩放 (1.15,0.8)；灰线只平移，白线应用完整组合。
在本文件观察 local、matrix 与每条边端点；在 Camera/Transforms.cpp 观察齐次变换结果。

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

// HW21：比较未变换（暗灰色）的四边形和完成变换（白色）的四边形。
// 可调节平移/旋转/反射/错切，观察差异。
// 绘制辅助入口：建立二维 T/R/S，把局部四边形变到屏幕，再逐边绘制灰色对照和白色结果。
// 可在函数内观察每一步的矩阵、顶点、裁剪结果或屏幕位置；详细计算见文件顶部。
void DrawTransform2DDemo()
{
    const Point2 local[4] = {{-15.0f, -12.0f}, {15.0f, -12.0f}, {15.0f, 12.0f}, {-15.0f, 12.0f}};
    const Mat3 translation = MakeTranslation2D(50.0f, 50.0f);
    const Mat3 rotation = MakeRotation2D(0.5f);
    const Mat3 scaling = MakeScale2D(1.15f, 0.8f);
    // 先组合 T*R*S；实际作用顺序是缩放 -> 旋转 -> 平移。
    const Mat3 matrix = Multiply(translation, Multiply(rotation, scaling));

    // 遍历四条边：灰色只平移，白色两个端点先经完整矩阵变换再画线。
    for (int i = 0; i < 4; ++i)
    {
        const int next = (i + 1) % 4;
        // 对照图：仅将局部四边形移至画面中央。
        DrawLine({local[i].x + 50.0f, local[i].y + 50.0f}, {local[next].x + 50.0f, local[next].y + 50.0f},
                 Raster::MakeColor(65, 65, 65));
        DrawLine(TransformPoint2D(matrix, local[i]), TransformPoint2D(matrix, local[next]), white);
    }
}

// 数学入口：按下列顺序调用本章算法；near/nearN 输出实际与预期值，check 汇总 PASS/FAIL。
// 返回 TestResult；本章具体公式与逐步数值见文件顶部学习说明。
TestResult TestHW21()
{
    const int hw = 21;

    constexpr float halfPi = 3.14159265358979323846f * 0.5f;
    MathTestContext checks(hw);

    {
        const Point2 p = {2, 3};
        const auto pointNear = [&](const Point2 &actual, Point2 expected) {
            return checks.near(actual.x, expected.x) && checks.near(actual.y, expected.y);
        };
        checks.check(pointNear(TransformPoint2D(MakeTranslation2D(10, 20), p), {12, 23}), "2D Translation");
        checks.check(pointNear(TransformPoint2D(MakeRotation2D(halfPi), {1, 0}), {0, 1}),
                     "2D Rotation +90deg");
        checks.check(pointNear(TransformPoint2D(MakeScale2D(2, 3), {4, 5}), {8, 15}), "2D Scale");
        checks.check(pointNear(TransformPoint2D(MakeReflectionX2D(), p), {2, -3}), "2D Reflection about X");
        checks.check(pointNear(TransformPoint2D(MakeReflectionY2D(), p), {-2, 3}), "2D Reflection about Y");
        checks.check(pointNear(TransformPoint2D(MakeShear2D(1, 0), p), {5, 3}), "2D Shear X");
        checks.check(pointNear(TransformPoint2D(MakeShear2D(0, 1), p), {2, 5}), "2D Shear Y");
        const Mat3 trs =
            Multiply(MakeTranslation2D(10, 20), Multiply(MakeRotation2D(halfPi), MakeScale2D(2, 3)));
        checks.check(pointNear(TransformPoint2D(trs, {1, 0}), {10, 22}), "2D T*R*S order");
    }
    std::printf("[HW%02d] 结果：%d PASS, %d FAIL\n\n", hw, checks.result.passed, checks.result.failed);
    return {checks.result.passed, checks.result.failed};
}

// 作业绘制入口：Runner 已清背景和深度，随后执行下列调用。
// 具体调用链、采样/变换过程和观察结果见文件顶部学习说明。
void RenderHW21()
{

    // 数学检查之后运行可视化；F11 进入本文件上面的矩阵与端点流程。
    DrawTransform2DDemo();
}
