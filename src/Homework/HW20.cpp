/*
【HW20 学习说明：四阶矩阵】
运行选择：Config.h 中 kActiveHomework=20，或启动参数 --hw 20。
当前状态：已有实现。以下调用说明对应当前代码，不自动实现缺失作业。
测试调度：InitializeApplication -> RunMathHomeworkTest(20) -> TestHW20。
输出方式：只有数学测试回调；后续 Runner 清空帧缓冲，没有作业图形可画，黑窗口属于正常行为。

【调用流程、数学原理与计算过程】
实际调用：TestHW20 -> IdentityMat4、MakeTranslation3D、MakeScale3D；
再调用 Mat4*Mat4 与 Mat4*Vec4 的 Multiply，交给 MathTestContext::near4/nearMat4/check。
数学原理：四阶矩阵沿用逐行乘列的求和公式，四维齐次分量允许将平移纳入矩阵乘法。
计算过程：I 保持 (2,3,4,1)；验证 I*T=T 和 T*I=T。
T(2,3,4)*S(2,3,4)*(1,2,3,1)：先缩放成 (2,6,12,1)，再平移成 (4,9,16,1)。
本章共 3 项检查，不绘图；结果中的 w 是真实计算分量，投影后可能不等于 1。
在 Core/Math.cpp 观察四阶 Multiply 的 row/col/k 和返回 Vec4 的四个点积。

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
TestResult TestHW20()
{
    const int hw = 20;

    constexpr float halfPi = 3.14159265358979323846f * 0.5f;
    MathTestContext checks(hw);

    {
        // 建立 I、T、S；后续矩阵乘列向量验证 T*S 的应用顺序。
        const Mat4 i = IdentityMat4();
        const Mat4 t = MakeTranslation3D(2, 3, 4);
        const Mat4 scale = MakeScale3D(2, 3, 4);
        checks.check(checks.near4(Multiply(i, Vec4{2, 3, 4, 1}), Vec4{2, 3, 4, 1}), "Mat4 identity * vector");
        checks.check(checks.nearMat4(Multiply(i, t), t) && checks.nearMat4(Multiply(t, i), t),
                     "Mat4 identity multiplication");
        checks.check(checks.near4(Multiply(Multiply(t, scale), Vec4{1, 2, 3, 1}), Vec4{4, 9, 16, 1}),
                     "Mat4 matrix product * vector");
    }
    std::printf("[HW%02d] 结果：%d PASS, %d FAIL\n\n", hw, checks.result.passed, checks.result.failed);
    return {checks.result.passed, checks.result.failed};
}
