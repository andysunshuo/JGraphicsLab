/*
【HW26 学习说明：透视投影】
运行选择：Config.h 中 kActiveHomework=26，或启动参数 --hw 26。
当前状态：已有实现。以下调用说明对应当前代码，不自动实现缺失作业。
测试调度：InitializeApplication -> RunMathHomeworkTest(26) -> TestHW26。
输出方式：只有数学测试回调；后续 Runner 清空帧缓冲，没有作业图形可画，黑窗口属于正常行为。

【调用流程、数学原理与计算过程】
实际调用：TestHW26 -> MakePerspective -> tan；再用 Multiply 得到 clip，计算 clip.z/clip.w。
数学原理：f=1/tan(FOVy/2)，xClip=f*x/aspect、yClip=f*y、wClip=-zView。
深度项为 zClip=-(far+near)*zView/(far-near)-2*near*far/(far-near)；除 w 后近端 -1、远端 1。
计算过程：FOV=90°、aspect=1、near=1、far=11；近点 clip.w=1，远点 clip.w=11。
相机点 (2,0,-2,1) -> clip=(2,0,0.2,2)，再除 w 会得到 NDC=(1,0,0.1)。
本章共 4 项检查，包含 aspect=0 的参数异常；这里只验证投影矩阵，完整管线要先裁剪再除 w。
在 Camera/Transforms.cpp 观察 f、m.m[2][2]、m.m[2][3]、m.m[3][2]。

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
TestResult TestHW26()
{
    const int hw = 26;

    constexpr float halfPi = 3.14159265358979323846f * 0.5f;
    MathTestContext checks(hw);

    {
        // 建立 FOV=90° 的透视矩阵；进入其中查看 f=1/tan(FOV/2) 和 wClip=-zView。
        const Mat4 proj = MakePerspective(halfPi, 1.0f, 1.0f, 11.0f);
        // 分别投影近点与远点；后续以 zClip/wClip 验证 NDC 深度端点。
        const Vec4 nearClip = Multiply(proj, Vec4{0, 0, -1, 1});
        const Vec4 farClip = Multiply(proj, Vec4{0, 0, -11, 1});
        checks.check(checks.near(nearClip.w, 1) && checks.near(nearClip.z / nearClip.w, -1),
                     "Perspective near -> NDC z=-1");
        checks.check(checks.near(farClip.w, 11) && checks.near(farClip.z / farClip.w, 1),
                     "Perspective far -> NDC z=+1");
        checks.check(checks.near4(Multiply(proj, Vec4{2, 0, -2, 1}), Vec4{2, 0, 0.2f, 2}),
                     "Perspective fov=90 aspect=1, view(2,0,-2)");
        bool rejected = false;
        try
        {
            (void)MakePerspective(halfPi, 0.0f, 1.0f, 11.0f);
        }
        catch (const std::invalid_argument &)
        {
            rejected = true;
        }
        checks.check(rejected, "Perspective rejects invalid aspect");
    }
    std::printf("[HW%02d] 结果：%d PASS, %d FAIL\n\n", hw, checks.result.passed, checks.result.failed);
    return {checks.result.passed, checks.result.failed};
}
