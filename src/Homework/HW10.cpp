/*
【HW10 学习说明：重心坐标】
运行选择：Config.h 中 kActiveHomework=10，或启动参数 --hw 10。
当前状态：已有实现。以下调用说明对应当前代码，不自动实现缺失作业。
测试调度：InitializeApplication -> RunMathHomeworkTest(10) -> TestHW10。
绘制调度：InitializeApplication -> RenderApplicationFrame -> RunHomework(10,false) -> RenderHW10。
顺序：先运行数学检查，再清空检查可能写入的颜色/深度，最后调用本章绘制入口。

【调用流程、数学原理与计算过程】
测试调用：TestHW10 -> ComputeBarycentric -> EdgeFunction；随后 MathTestContext::near/check 统计结果。
渲染调用：RenderHW10 -> DrawTriangleWireframe -> DrawLine -> Raster::PutPixel，仅画白色轮廓。
数学原理：重心坐标是三个有向子面积占总有向面积的比例，P=alpha*A+beta*B+gamma*C。
计算过程：area=E(A,B,C)=5825；alpha=E(B,C,P)/area，beta=E(C,A,P)/area，gamma=E(A,B,P)/area。
默认 P=(45,40) 时权重为 2000/5825、1750/5825、2075/5825，三者之和为 1。
ComputeBarycentric 对 area=0 返回 false；测试目前检查计算成功与权重和，不额外绘制插值色。
在 Geometry/Geometry.cpp 观察 result 的三个分量；在本文件看实际和与预期 1 的 PASS/FAIL。

【阅读与调试】
F11 进入本章下述调用的公共实现；在本文件入口设置断点，可检查本章数据与变量。
公共算法不会被复制到每个 HW；本次只补说明，运算语句保持不变。
*/

#include "Homework/Entries.h"
#include "Geometry/Primitives.h"
#include "Homework/DemoData.h"
#include "Renderer/Rasterizer2D.h"
#include "Geometry/Geometry.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <stdexcept>
#include <limits>

// 数学入口：按下列顺序调用本章算法；near/nearN 输出实际与预期值，check 汇总 PASS/FAIL。
// 返回 TestResult；本章具体公式与逐步数值见文件顶部学习说明。
TestResult TestHW10()
{
    const int hw = 10;

    constexpr float halfPi = 3.14159265358979323846f * 0.5f;
    MathTestContext checks(hw);

    {
        Barycentric bary = {};
        // 第 1 步：以三个有向子面积除以总面积，得到 bary 的三个权重。
        const bool ok = ComputeBarycentric(testTriangle, insidePoint, bary);
        checks.check(ok && checks.near(bary.alpha + bary.beta + bary.gamma, 1.0f),
                     "Barycentric: alpha+beta+gamma = 1");
    }
    std::printf("[HW%02d] 结果：%d PASS, %d FAIL\n\n", hw, checks.result.passed, checks.result.failed);
    return {checks.result.passed, checks.result.failed};
}

// 作业绘制入口：Runner 已清背景和深度，随后执行下列调用。
// 具体调用链、采样/变换过程和观察结果见文件顶部学习说明。
void RenderHW10()
{

    // 数学检查结束后只画参考轮廓；RGB 插值演示是下一章 HW11。
    DrawTriangleWireframe(testTriangle, white);
}
