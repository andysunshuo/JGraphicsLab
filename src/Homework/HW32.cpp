/*
【HW32：走样，已实现】
选择：Config.h 的 kActiveHomework=32，或 --hw 32。
调用：InitializeApplication -> TestHW32 的真实数值检查；通过后 Runner -> RenderHW32。
具体绘图流程：RenderHW32 -> Pattern(128,1/4) -> 遍历像素与子样本 -> 平均颜色 -> Publish。
结果由 Publish 组织成独立 600×600 中文对照画布。
数学与计算过程：
连续高频信号采样后可能变成低频条纹，产生锯齿/摩尔纹。
Pattern 中把重复棋盘格透视压缩，并叠加斜线；左右使用 1 个中心或 4×4 子样本。
整数 n 上 cos(2*pi*0.8*n) 与 cos(2*pi*0.2*n) 无法区分，这是频率混叠的数值例。
计算示例：n=1 时 cos(1.6π)=cos(0.4π)≈0.309017；4×4 子采样的颜色除以 16。
断点：本文件 Test/Render 和公共 Renderer/ImageProcessing、LearningPipeline、Geometry/Surface。
与原作业共用向量、矩阵、齐次裁剪、深度比较和 Top-left 规则；不更改 HW01～HW31 的输入。
*/
#include "Homework/LearningDemos.h"
#include "Renderer/Interpolation.h"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>
#include <cstdio>
using namespace HomeworkLearning;
namespace
{
Image Pattern(int size, int samples)
{
    Image result(size, size);
    for (int y = 0; y < size; ++y)
        for (int x = 0; x < size; ++x)
        {
            Vec3 sum{};
            for (int sy = 0; sy < samples; ++sy)
                for (int sx = 0; sx < samples; ++sx)
                {
                    const float u = (x + (sx + 0.5f) / samples) / size,
                                v = (y + (sy + 0.5f) / samples) / size;
                    const int ix = static_cast<int>(std::floor(30 * u / (v + 0.13f))),
                              iy = static_cast<int>(std::floor(30 * v / (v + 0.13f)));
                    const float c = ((ix + iy) & 1) ? 0.85f : 0.02f;
                    const Vec3 color =
                        std::abs(v - 0.65f * u - 0.15f) < 0.006f ? Vec3{1, 0.1f, 0.02f} : Vec3{c, c, c};
                    sum = Add(sum, color);
                }
            result.At(x, y) = Multiply(sum, 1.0f / (samples * samples));
        }
    return result;
}
} // namespace

// 数学入口：计算实际结果，与明确的预期比较，返回真实 PASS/FAIL 数量。
TestResult TestHW32()
{
    constexpr float pi = 3.14159265358979323846f;
    MathTestContext checks(32);
    for (int n = 0; n < 8; ++n)
        checks.check(checks.near(std::cos(2 * pi * 0.8f * n), std::cos(2 * pi * 0.2f * n)),
                     "整数采样的频率混叠");
    const auto a = Pattern(48, 1), b = Pattern(48, 4);
    bool different = false;
    for (std::size_t i = 0; i < a.pixels.size(); ++i)
        different |= Length(Subtract(a.pixels[i], b.pixels[i])) > 0.01f;
    checks.check(different, "增加子样本实际改变高频图案覆盖");
    std::printf("[HW32] 结果：%d PASS，%d FAIL\n", checks.result.passed, checks.result.failed);
    return checks.result;
}

// 图形入口：按注释标明的算法生成实际对照图，不使用静态图片冒充算法结果。
void RenderHW32()
{
    Publish(32, L"走样与摩尔纹", L"重复纹理和斜线：中心采样 vs 多位置平均",
            {{Pattern(128, 1), L"单点采样：锯齿/摩尔纹"}, {Pattern(128, 4), L"4×4 子采样对照"}});
}
