/*
【HW18 学习说明：齐次向量】
运行选择：Config.h 中 kActiveHomework=18，或启动参数 --hw 18。
当前状态：已有实现。以下调用说明对应当前代码，不自动实现缺失作业。
测试调度：InitializeApplication -> RunMathHomeworkTest(18) -> TestHW18。
输出方式：只有数学测试回调；后续 Runner 清空帧缓冲，没有作业图形可画，黑窗口属于正常行为。

【调用流程、数学原理与计算过程】
实际调用：TestHW18 -> MakePosition4/MakeDirection4、Vec4 Add/Multiply；
然后 MakeTranslation3D -> 矩阵 Multiply；结果交给 MathTestContext::near4/check。
数学原理：位置补 w=1，方向补 w=0；平移矩阵的第四列乘上 w，因此只平移位置。
计算过程：(2,3,4) 变成位置 (2,3,4,1) 或方向 (2,3,4,0)，Vec4 加法/数乘包含 w 分量。
平移 (10,20,30) 后，位置 (1,2,3,1) -> (11,22,33,1)，方向 (1,2,3,0) 保持不变。
本章共 6 项数学检查、不绘制图形；不要把四维向量中的 w 随意丢弃。
在 Camera/Transforms.cpp 查看构造函数，在 Core/Math.cpp 查看 Mat4*Vec4 的第四列计算。

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
TestResult TestHW18()
{
    const int hw = 18;

    constexpr float halfPi = 3.14159265358979323846f * 0.5f;
    MathTestContext checks(hw);

    {
        checks.check(checks.near4(MakePosition4({2, 3, 4}), Vec4{2, 3, 4, 1}), "Position w=1");
        checks.check(checks.near4(MakeDirection4({2, 3, 4}), Vec4{2, 3, 4, 0}), "Direction w=0");
        checks.check(checks.near4(Add(Vec4{1, 2, 3, 1}, Vec4{2, 3, 4, 0}), Vec4{3, 5, 7, 1}), "Vec4 Add");
        checks.check(checks.near4(Multiply(Vec4{1, 2, 3, 4}, 2.0f), Vec4{2, 4, 6, 8}),
                     "Vec4 Multiply scalar");
        // 建立第四列为 (10,20,30) 的平移矩阵；后面分别变换 w=1 的位置与 w=0 的方向。
        const Mat4 translation = MakeTranslation3D(10, 20, 30);
        checks.check(checks.near4(Multiply(translation, MakePosition4({1, 2, 3})), Vec4{11, 22, 33, 1}),
                     "Translation affects a position");
        checks.check(checks.near4(Multiply(translation, MakeDirection4({1, 2, 3})), Vec4{1, 2, 3, 0}),
                     "Translation does NOT affect a direction");
    }
    std::printf("[HW%02d] 结果：%d PASS, %d FAIL\n\n", hw, checks.result.passed, checks.result.failed);
    return {checks.result.passed, checks.result.failed};
}
