/*
【HW17 学习说明：三维向量】
运行选择：Config.h 中 kActiveHomework=17，或启动参数 --hw 17。
当前状态：已有实现。以下调用说明对应当前代码，不自动实现缺失作业。
测试调度：InitializeApplication -> RunMathHomeworkTest(17) -> TestHW17。
输出方式：只有数学测试回调；后续 Runner 清空帧缓冲，没有作业图形可画，黑窗口属于正常行为。

【调用流程、数学原理与计算过程】
实际调用：TestHW17 -> Vec3 的 Add、Subtract、Multiply、Dot、Cross、Length、Normalize；
Length -> Dot -> sqrt；Normalize -> Length -> Multiply；结果交给 MathTestContext::near3/near/check。
数学原理：点积 a·b=ax*bx+ay*by+az*bz，也等于 |a||b|cos(theta)；叉积方向服从右手法则。
叉积公式：(ay*bz-az*by, az*bx-ax*bz, ax*by-ay*bx)，交换两向量会使结果取反。
计算过程：(1,2,3)·(4,5,6)=4+10+18=32；X×Y=(0,0,1)，Y×X=(0,0,-1)。
Length(2,3,6)=sqrt(4+9+36)=7；Normalize(0,0,5)=(0,0,1)，零向量返回零。
本章共 9 项检查，控制台打印实际值、预期值和 PASS/FAIL；没有 RenderHW17 图形回调。
在 Core/Math.cpp 的 Vec3 重载单步，观察点积累加、叉积符号及归一化长度。

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
TestResult TestHW17()
{
    const int hw = 17;

    constexpr float halfPi = 3.14159265358979323846f * 0.5f;
    MathTestContext checks(hw);

    checks.check(checks.near3(Add(Vec3{1, 2, 3}, Vec3{4, 5, 6}), {5, 7, 9}), "Vec3 Add");
    checks.check(checks.near3(Subtract(Vec3{5, 7, 9}, Vec3{1, 2, 3}), {4, 5, 6}), "Vec3 Subtract");
    checks.check(checks.near3(Multiply(Vec3{1, 2, 3}, 2.0f), {2, 4, 6}), "Vec3 Multiply scalar");
    // 点积计算：1*4+2*5+3*6=32，可进入 Vec3 版本 Dot 查看逐项累加。
    checks.check(checks.near(Dot(Vec3{1, 2, 3}, Vec3{4, 5, 6}), 32.0f), "Vec3 Dot=32");
    // 叉积先检查右手方向 +Z，再交换输入顺序检查 -Z，不能把叉积写成对称运算。
    checks.check(checks.near3(Cross({1, 0, 0}, {0, 1, 0}), {0, 0, 1}), "Cross(X,Y)=+Z");
    checks.check(checks.near3(Cross({0, 1, 0}, {1, 0, 0}), {0, 0, -1}), "Cross(Y,X)=-Z");
    // 长度内部 Dot(v,v)=49，再 sqrt 得 7；后两条验证非零和零向量归一化。
    checks.check(checks.near(Length(Vec3{2, 3, 6}), 7.0f), "Vec3 Length=7");
    checks.check(checks.near3(Normalize(Vec3{0, 0, 5}), {0, 0, 1}), "Vec3 Normalize");
    checks.check(checks.near3(Normalize(Vec3{0, 0, 0}), {0, 0, 0}), "Vec3 Normalize zero vector");

    std::printf("[HW%02d] 结果：%d PASS, %d FAIL\n\n", hw, checks.result.passed, checks.result.failed);
    return {checks.result.passed, checks.result.failed};
}
