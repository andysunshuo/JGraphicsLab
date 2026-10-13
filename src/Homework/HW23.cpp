/*
【HW23 学习说明：模型矩阵】
运行选择：Config.h 中 kActiveHomework=23，或启动参数 --hw 23。
当前状态：已有实现。以下调用说明对应当前代码，不自动实现缺失作业。
测试调度：InitializeApplication -> RunMathHomeworkTest(23) -> TestHW23。
输出方式：只有数学测试回调；后续 Runner 清空帧缓冲，没有作业图形可画，黑窗口属于正常行为。

【调用流程、数学原理与计算过程】
实际调用：TestHW23 -> MakeModelMatrix；内部调用三维 T、Rz、Ry、Rx、S 构造函数与 Multiply。
本文件再用 Multiply(Mat4,Vec4) 变换位置，并通过 nearMat4/near4/check 验证，共 3 项。
数学原理：M=T*Rz*Ry*Rx*S；最右侧先执行，因此依次缩放、绕 X/Y/Z 旋转、最后平移。
计算过程：零位置/零角度/单位缩放得到 I；(1,2,3) 经 S(2,3,4) 与 T(10,20,30) -> (12,26,42)。
另一例 (1,0,0) 经统一缩放 2、绕 Z 90°、平移 (10,20,30)：(2,0,0)->(0,2,0)->(10,22,30)。
本章只验证模型矩阵，不自动显示立方体；图形使用相同算法的是 HW29/HW31。
在 Camera/Transforms.cpp 的 MakeModelMatrix 观察 t、rz、ry、rx、s 与最终矩阵。

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
TestResult TestHW23()
{
    const int hw = 23;

    constexpr float halfPi = 3.14159265358979323846f * 0.5f;
    MathTestContext checks(hw);

    // 先验证恒等参数生成 I，再验证 T*S 和 T*Rz*S 对具体位置的逐步效果。
    checks.check(checks.nearMat4(MakeModelMatrix({0, 0, 0}, {0, 0, 0}, {1, 1, 1}), IdentityMat4()),
                 "identity Model Matrix");
    checks.check(checks.near4(Multiply(MakeModelMatrix({10, 20, 30}, {0, 0, 0}, {2, 3, 4}), Vec4{1, 2, 3, 1}),
                              Vec4{12, 26, 42, 1}),
                 "Model T*S transforms position");
    checks.check(
        checks.near4(Multiply(MakeModelMatrix({10, 20, 30}, {0, 0, halfPi}, {2, 2, 2}), Vec4{1, 0, 0, 1}),
                     Vec4{10, 22, 30, 1}),
        "Model T*Rz*S order");

    std::printf("[HW%02d] 结果：%d PASS, %d FAIL\n\n", hw, checks.result.passed, checks.result.failed);
    return {checks.result.passed, checks.result.failed};
}
