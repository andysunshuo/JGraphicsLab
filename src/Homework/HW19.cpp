/*
【HW19 学习说明：三阶矩阵】
运行选择：Config.h 中 kActiveHomework=19，或启动参数 --hw 19。
当前状态：已有实现。以下调用说明对应当前代码，不自动实现缺失作业。
测试调度：InitializeApplication -> RunMathHomeworkTest(19) -> TestHW19。
输出方式：只有数学测试回调；后续 Runner 清空帧缓冲，没有作业图形可画，黑窗口属于正常行为。

【调用流程、数学原理与计算过程】
实际调用：TestHW19 -> IdentityMat3、MakeTranslation2D、MakeScale2D；
再调用 Mat3*Mat3 与 Mat3*Vec3 的 Multiply，交给 MathTestContext::near3/nearMat3/check。
数学原理：C[row][col]=sum_k A[row][k]*B[k][col]；矩阵乘列向量时每一行与向量做点积。
计算过程：单位矩阵对角为 1，保持 (2,3,1) 不变；分别验证 I*M=M 与 M*I=M。
T(2,3)*S(2,3)*(1,2,1)：先得到 (2,6,1)，再平移成 (4,9,1)。
本章共 3 项检查；m[row][column] 是存储下标约定，不能据此把列向量变为行向量。
在 Core/Math.cpp 查看三层 row/col/k 循环、result 初始零值及各项累加。

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
TestResult TestHW19()
{
    const int hw = 19;

    constexpr float halfPi = 3.14159265358979323846f * 0.5f;
    MathTestContext checks(hw);

    {
        // 建立单位矩阵与二维齐次平移矩阵，后续检查单位性质和先缩放后平移。
        const Mat3 i = IdentityMat3();
        const Mat3 m = MakeTranslation2D(2, 3);
        checks.check(checks.near3(Multiply(i, Vec3{2, 3, 1}), {2, 3, 1}), "Mat3 identity * vector");
        checks.check(checks.nearMat3(Multiply(i, m), m) && checks.nearMat3(Multiply(m, i), m),
                     "Mat3 identity multiplication");
        checks.check(checks.near3(Multiply(Multiply(m, MakeScale2D(2, 3)), Vec3{1, 2, 1}), {4, 9, 1}),
                     "Mat3 matrix product * vector");
    }
    std::printf("[HW%02d] 结果：%d PASS, %d FAIL\n\n", hw, checks.result.passed, checks.result.failed);
    return {checks.result.passed, checks.result.failed};
}
