/*
【HW27 学习说明：透视除法】
运行选择：Config.h 中 kActiveHomework=27，或启动参数 --hw 27。
当前状态：已有实现。以下调用说明对应当前代码，不自动实现缺失作业。
测试调度：InitializeApplication -> RunMathHomeworkTest(27) -> TestHW27。
输出方式：只有数学测试回调；后续 Runner 清空帧缓冲，没有作业图形可画，黑窗口属于正常行为。

【调用流程、数学原理与计算过程】
实际调用：TestHW27 -> PerspectiveDivide -> isfinite/abs 检查及逐分量除法；near3/check 验证结果。
数学原理：齐次坐标中 (x,y,z,w) 与缩放后的同类坐标表示同一位置，除 w 得到三维 NDC。
计算过程：(2,4,6,2) -> (1,2,3)，w=0 或 abs(w)<=1e-7 时返回 false，避免无效除法。
函数也检查除法后的 x/y/z 是否有限；返回 false 时调用者不应使用输出当作成功结果。
本章共 3 项检查；它没有检查点是否在六平面之内，也不能替代 HW30 裁剪。
在 Camera/Transforms.cpp 观察 clip.w、ndc 和返回值；本章没有图形回调。

【阅读与调试】
F11 进入本章下述调用的公共实现；在本文件入口设置断点，可检查本章数据与变量。
公共算法不会被复制到每个 HW；本次只补说明，运算语句保持不变。
*/

#include "Homework/Entries.h"
#include "Geometry/Primitives.h"
#include "Camera/Transforms.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <stdexcept>
#include <limits>

// 数学入口：按下列顺序调用本章算法；near/nearN 输出实际与预期值，check 汇总 PASS/FAIL。
// 返回 TestResult；本章具体公式与逐步数值见文件顶部学习说明。
TestResult TestHW27()
{
    const int hw = 27;

    constexpr float halfPi = 3.14159265358979323846f * 0.5f;
    MathTestContext checks(hw);

    {
        // 先测试有效 w 的除法；再检查 w=0 和极小 w 会返回 false。
        Vec3 ndc = {};
        checks.check(PerspectiveDivide({2, 4, 6, 2}, ndc) && checks.near3(ndc, {1, 2, 3}), "Clip/w => NDC");
        checks.check(!PerspectiveDivide({1, 2, 3, 0}, ndc), "Reject w=0");
        checks.check(!PerspectiveDivide({1, 2, 3, 1e-9f}, ndc), "Reject tiny w");
    }
    std::printf("[HW%02d] 结果：%d PASS, %d FAIL\n\n", hw, checks.result.passed, checks.result.failed);
    return {checks.result.passed, checks.result.failed};
}
