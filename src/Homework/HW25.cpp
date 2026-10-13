/*
【HW25 学习说明：正交投影】
运行选择：Config.h 中 kActiveHomework=25，或启动参数 --hw 25。
当前状态：已有实现。以下调用说明对应当前代码，不自动实现缺失作业。
测试调度：InitializeApplication -> RunMathHomeworkTest(25) -> TestHW25。
输出方式：只有数学测试回调；后续 Runner 清空帧缓冲，没有作业图形可画，黑窗口属于正常行为。

【调用流程、数学原理与计算过程】
实际调用：TestHW25 -> MakeOrthographic -> IdentityMat4；再由 Multiply 与 near4/check 验证。
数学原理：用平移和缩放把相机空间长方体线性映射到 NDC；没有随距离缩小的透视效果，w=1。
计算公式：x'=2*x/(r-l)-(r+l)/(r-l)，y'=2*y/(t-b)-(t+b)/(t-b)，
z'=-2*z/(far-near)-(far+near)/(far-near)；near/far 是沿 -Z 测量的正距离。
默认边界 -1～1、near=1、far=11：z=-1 -> -1，z=-11 -> 1，右上近点 (1,1,-1) -> (1,1,-1)。
本章共 4 项检查，包含 left=right 非法边界异常；不绘图，也未调用 PerspectiveDivide。
在 Camera/Transforms.cpp 查看缩放对角项、平移列和深度符号。

【阅读与调试】
F11 进入本章下述调用的公共实现；在本文件入口设置断点，可检查本章数据与变量。
公共算法不会被复制到每个 HW；本次只补说明，运算语句保持不变。
*/

#include "Homework/Entries.h"
#include "Geometry/Primitives.h"
#include "Core/Math.h"
#include "Camera/Transforms.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <stdexcept>
#include <limits>

// 数学入口：按下列顺序调用本章算法；near/nearN 输出实际与预期值，check 汇总 PASS/FAIL。
// 返回 TestResult；本章具体公式与逐步数值见文件顶部学习说明。
TestResult TestHW25()
{
    const int hw = 25;

    constexpr float halfPi = 3.14159265358979323846f * 0.5f;
    MathTestContext checks(hw);

    {
        // 建立线性投影盒，再逐个检查近面、远面和右上近点的坐标映射。
        const Mat4 ortho = MakeOrthographic(-1, 1, -1, 1, 1, 11);
        checks.check(checks.near4(Multiply(ortho, Vec4{0, 0, -1, 1}), Vec4{0, 0, -1, 1}),
                     "Ortho near z -> -1");
        checks.check(checks.near4(Multiply(ortho, Vec4{0, 0, -11, 1}), Vec4{0, 0, 1, 1}),
                     "Ortho far z -> +1");
        checks.check(checks.near4(Multiply(ortho, Vec4{1, 1, -1, 1}), Vec4{1, 1, -1, 1}),
                     "Ortho top-right bound");
        bool rejected = false;
        try
        {
            (void)MakeOrthographic(0, 0, -1, 1, 1, 10);
        }
        catch (const std::invalid_argument &)
        {
            rejected = true;
        }
        checks.check(rejected, "Ortho rejects invalid bounds");
    }
    std::printf("[HW%02d] 结果：%d PASS, %d FAIL\n\n", hw, checks.result.passed, checks.result.failed);
    return {checks.result.passed, checks.result.failed};
}
