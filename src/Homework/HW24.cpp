/*
【HW24 学习说明：观察矩阵】
运行选择：Config.h 中 kActiveHomework=24，或启动参数 --hw 24。
当前状态：已有实现。以下调用说明对应当前代码，不自动实现缺失作业。
测试调度：InitializeApplication -> RunMathHomeworkTest(24) -> TestHW24。
输出方式：只有数学测试回调；后续 Runner 清空帧缓冲，没有作业图形可画，黑窗口属于正常行为。

【调用流程、数学原理与计算过程】
实际调用：TestHW24 -> LookAt -> Subtract、Length、Normalize、Cross、Dot、IdentityMat4；
然后 Multiply(Mat4,Vec4) 验证变换结果，near4/check 统计 4 项检查。
数学原理：把世界点减去 eye，并投影到相机正交基；本工程相机看向局部 -Z。
计算过程：f=normalize(target-eye)，r=normalize(f×up)，u=r×f；矩阵前三行是 r、u、-f。
平移列为 -r·eye、-u·eye、f·eye；eye=(0,0,3) 时世界原点 -> (0,0,-3)，eye 自己 -> 原点。
测试还验证朝 +X 的相机将世界 (1,0,0) 变到相机 (0,0,-1)，以及 eye=target 时抛出参数异常。
LookAt 也检查 up 与视线共线；当前测试没有单独覆盖该异常。断点观察 fVector、f、r、u。

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
TestResult TestHW24()
{
    const int hw = 24;

    constexpr float halfPi = 3.14159265358979323846f * 0.5f;
    MathTestContext checks(hw);

    {
        // 进入 LookAt：从 target-eye 建立方向与正交基，再通过点积求平移列。
        const Mat4 view = LookAt({0, 0, 3}, {0, 0, 0}, {0, 1, 0});
        checks.check(checks.near4(Multiply(view, Vec4{0, 0, 0, 1}), Vec4{0, 0, -3, 1}),
                     "LookAt world origin -> view -Z");
        checks.check(checks.near4(Multiply(view, Vec4{0, 0, 3, 1}), Vec4{0, 0, 0, 1}),
                     "LookAt eye -> view origin");
        checks.check(checks.near4(Multiply(LookAt({0, 0, 0}, {1, 0, 0}, {0, 1, 0}), Vec4{1, 0, 0, 1}),
                                  Vec4{0, 0, -1, 1}),
                     "LookAt arbitrary +X view direction");
        // 异常分支：相机 eye 与 target 重合时没有有效视线，用抛异常结果验证拒绝。
        bool rejected = false;
        try
        {
            (void)LookAt({0, 0, 0}, {0, 0, 0}, {0, 1, 0});
        }
        catch (const std::invalid_argument &)
        {
            rejected = true;
        }
        checks.check(rejected, "LookAt rejects coincident eye/target");
    }
    std::printf("[HW%02d] 结果：%d PASS, %d FAIL\n\n", hw, checks.result.passed, checks.result.failed);
    return {checks.result.passed, checks.result.failed};
}
