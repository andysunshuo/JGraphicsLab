/*
【HW16 学习说明：二维向量】
运行选择：Config.h 中 kActiveHomework=16，或启动参数 --hw 16。
当前状态：已有实现。以下调用说明对应当前代码，不自动实现缺失作业。
测试调度：InitializeApplication -> RunMathHomeworkTest(16) -> TestHW16。
输出方式：只有数学测试回调；后续 Runner 清空帧缓冲，没有作业图形可画，黑窗口属于正常行为。

【调用流程、数学原理与计算过程】
实际调用：TestHW16 -> Vec2 的 Add、Subtract、Multiply、Dot、Length、Normalize；
Length 内部调用 Dot 再 sqrt，Normalize 内部先调用 Length；结果交给 MathTestContext::near2/near/check。
数学原理：加减与数乘逐分量执行；点积 a·b=ax*bx+ay*by；长度 |v|=sqrt(v·v)；单位向量 v/|v|。
计算过程：(1,2)+(3,4)=(4,6)，(5,7)-(2,3)=(3,4)，2*(3,4)=(6,8)。
点积 (1,2)·(3,4)=11，(3,4) 的长度 sqrt(9+16)=5，归一化得到 (0.6,0.8)。
长度<=1e-7 时 Normalize 返回零向量，避免除以零；本章共 7 项检查，不绘制图形、不加载 OBJ。
在 Core/Math.cpp 的 Vec2 重载中断点，注意不要误进同名 Vec3/Mat4 重载。

【阅读与调试】
F11 进入本章下述调用的公共实现；在本文件入口设置断点，可检查本章数据与变量。
公共算法不会被复制到每个 HW；本次只补说明，运算语句保持不变。
*/

#include "Homework/Entries.h"
#include "Geometry/Primitives.h"
#include "Core/Math.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <stdexcept>
#include <limits>

// 数学入口：按下列顺序调用本章算法；near/nearN 输出实际与预期值，check 汇总 PASS/FAIL。
// 返回 TestResult；本章具体公式与逐步数值见文件顶部学习说明。
TestResult TestHW16()
{
    const int hw = 16;

    constexpr float halfPi = 3.14159265358979323846f * 0.5f;
    MathTestContext checks(hw);

    checks.check(checks.near2(Add(Vec2{1, 2}, Vec2{3, 4}), {4, 6}), "Vec2 Add (1,2)+(3,4)=(4,6)");
    checks.check(checks.near2(Subtract(Vec2{5, 7}, Vec2{2, 3}), {3, 4}), "Vec2 Subtract");
    checks.check(checks.near2(Multiply(Vec2{3, 4}, 2.0f), {6, 8}), "Vec2 Multiply scalar");
    // 点积逐分量相乘相加：1*3+2*4=11；near 验证与预期的误差。
    checks.check(checks.near(Dot(Vec2{1, 2}, Vec2{3, 4}), 11.0f), "Vec2 Dot=11");
    checks.check(checks.near(Length(Vec2{3, 4}), 5.0f), "Vec2 Length=5");
    // 归一化先求长度 5，再各分量除以 5；下一条检查零向量分支。
    checks.check(checks.near2(Normalize(Vec2{3, 4}), {0.6f, 0.8f}), "Vec2 Normalize");
    checks.check(checks.near2(Normalize(Vec2{0, 0}), {0, 0}), "Vec2 Normalize zero vector");

    std::printf("[HW%02d] 结果：%d PASS, %d FAIL\n\n", hw, checks.result.passed, checks.result.failed);
    return {checks.result.passed, checks.result.failed};
}
