/*
【HW22 学习说明：三维变换】
运行选择：Config.h 中 kActiveHomework=22，或启动参数 --hw 22。
当前状态：已有实现。以下调用说明对应当前代码，不自动实现缺失作业。
测试调度：InitializeApplication -> RunMathHomeworkTest(22) -> TestHW22。
输出方式：只有数学测试回调；后续 Runner 清空帧缓冲，没有作业图形可画，黑窗口属于正常行为。

【调用流程、数学原理与计算过程】
实际调用：TestHW22 -> MakeTranslation3D、MakeScale3D、MakeRotationX3D/Y3D/Z3D；
每个构造函数建立 Mat4 后，用 Multiply(Mat4,Vec4) 变换，最后 near4/check 验证。
数学原理：平移写第四列，缩放写对角，旋转在对应二维坐标平面使用 sin/cos，方向遵循右手法则。
计算过程：(1,2,3,1) 平移 (10,20,30) -> (11,22,33,1)，缩放 (2,3,4) -> (2,6,12,1)。
正 90° 时绕 X：(0,1,0)->(0,0,1)；绕 Y：(0,0,1)->(1,0,0)；绕 Z：(1,0,0)->(0,1,0)。
本章共 5 项检查；halfPi 是 pi/2 弧度，不能把 90 直接当作函数参数。
在 Camera/Transforms.cpp 查看旋转矩阵正负 sin 的位置，在 Core/Math.cpp 查看乘法。

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
TestResult TestHW22()
{
    const int hw = 22;

    constexpr float halfPi = 3.14159265358979323846f * 0.5f;
    MathTestContext checks(hw);

    checks.check(checks.near4(Multiply(MakeTranslation3D(10, 20, 30), Vec4{1, 2, 3, 1}), Vec4{11, 22, 33, 1}),
                 "3D Translation");
    checks.check(checks.near4(Multiply(MakeScale3D(2, 3, 4), Vec4{1, 2, 3, 1}), Vec4{2, 6, 12, 1}),
                 "3D Scale");
    // 正 90° 在对应二维平面改变坐标；分别检查 X、Y、Z 三个右手旋转。
    checks.check(checks.near4(Multiply(MakeRotationX3D(halfPi), Vec4{0, 1, 0, 1}), Vec4{0, 0, 1, 1}),
                 "Rotation about X +90deg");
    checks.check(checks.near4(Multiply(MakeRotationY3D(halfPi), Vec4{0, 0, 1, 1}), Vec4{1, 0, 0, 1}),
                 "Rotation about Y +90deg");
    checks.check(checks.near4(Multiply(MakeRotationZ3D(halfPi), Vec4{1, 0, 0, 1}), Vec4{0, 1, 0, 1}),
                 "Rotation about Z +90deg");

    std::printf("[HW%02d] 结果：%d PASS, %d FAIL\n\n", hw, checks.result.passed, checks.result.failed);
    return {checks.result.passed, checks.result.failed};
}
