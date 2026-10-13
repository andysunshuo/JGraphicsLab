/*
【HW28 学习说明：视口变换】
运行选择：Config.h 中 kActiveHomework=28，或启动参数 --hw 28。
当前状态：已有实现。以下调用说明对应当前代码，不自动实现缺失作业。
测试调度：InitializeApplication -> RunMathHomeworkTest(28) -> TestHW28。
输出方式：只有数学测试回调；后续 Runner 清空帧缓冲，没有作业图形可画，黑窗口属于正常行为。

【调用流程、数学原理与计算过程】
实际调用：TestHW28 -> ViewportTransform；用 MathTestContext::near/check 验证三个屏幕位置。
数学原理：NDC 的 [-1,1] 通过线性映射变到连续视口边界 [0,width] 和 [0,height]。
计算公式：xScreen=(xNdc+1)*width/2，yScreen=(yNdc+1)*height/2，Y 仍向上。
100*100 时 (-1,-1)->(0,0)、(0,0)->(50,50)、(1,1)->(100,100)，共 3 项检查。
(100,100) 是右上边界，不能作为合法像素索引；实际索引范围是 0～99，中心采样是 x+0.5/y+0.5。
该函数只映射 x/y，不做整数取整、覆盖、深度映射或颜色写入。
在 Camera/Transforms.cpp 观察 ndc、width/height、返回的连续 Point2。

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
TestResult TestHW28()
{
    const int hw = 28;

    constexpr float halfPi = 3.14159265358979323846f * 0.5f;
    MathTestContext checks(hw);

    {
        // 分别映射左下边界、中心、右上边界；返回的是连续坐标，而不是可直接使用的整数索引。
        const Point2 lo = ViewportTransform({-1, -1, 0}, 100, 100);
        const Point2 center = ViewportTransform({0, 0, 0}, 100, 100);
        const Point2 hi = ViewportTransform({1, 1, 0}, 100, 100);
        checks.check(checks.near(lo.x, 0) && checks.near(lo.y, 0), "Viewport bottom-left (0,0)");
        checks.check(checks.near(center.x, 50) && checks.near(center.y, 50), "Viewport center (50,50)");
        checks.check(checks.near(hi.x, 100) && checks.near(hi.y, 100),
                     "Viewport right/top boundary (100,100)");
    }
    std::printf("[HW%02d] 结果：%d PASS, %d FAIL\n\n", hw, checks.result.passed, checks.result.failed);
    return {checks.result.passed, checks.result.failed};
}
