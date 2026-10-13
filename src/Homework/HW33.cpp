/*
【HW33：信号采样，已实现】
选择：Config.h 的 kActiveHomework=33，或 --hw 33。
调用：InitializeApplication -> TestHW33 的真实数值检查；通过后 Runner -> RenderHW33。
具体绘图流程：RenderHW33 -> SignalGraph(4/32) -> SineSamples -> 画连续曲线和离散折线 -> Publish。
结果由 Publish 组织成独立 600×600 中文对照画布。
数学与计算过程：
采样时刻 t=n/fs，样本为 sin(2*pi*f*n/fs)。
本章用 3 Hz 信号分别按 4 Hz、32 Hz 采样；低采样率低于 Nyquist 的 6 Hz，会产生混叠。
图中灰线为连续信号，红线连接实际离散样本，亮色点标出采样位置。
计算示例：f=3、fs=4、n=1：sin(2π×3/4)=-1；采样率 4 小于 2f=6，无法恢复原信号。
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
std::vector<float> SineSamples(float frequency, float sampleRate, int count)
{
    if (!std::isfinite(frequency) || frequency < 0 || !std::isfinite(sampleRate) || sampleRate <= 0 ||
        count < 1 || count > 4096)
        throw std::invalid_argument("信号频率、采样率或样本数无效");
    std::vector<float> samples(count);
    for (int n = 0; n < count; ++n)
        samples[n] = std::sin(2 * 3.14159265358979323846f * frequency * n / sampleRate);
    return samples;
}
Image SignalGraph(int rate)
{
    Image image(256, 256, {0.015f, 0.018f, 0.03f});
    const auto samples = SineSamples(3, static_cast<float>(rate), rate + 1);
    for (int y = 0; y < 256; ++y)
        for (int x = 0; x < 256; ++x)
        {
            const float t = x / 255.0f, screenY = (1 - 2 * y / 255.0f) / 0.8f;
            Vec3 color{0.015f, 0.018f, 0.03f};
            if (std::abs(screenY) < 0.015f)
                color = {0.15f, 0.15f, 0.15f};
            if (std::abs(screenY - std::sin(6 * 3.14159265358979323846f * t)) < 0.025f)
                color = {0.35f, 0.4f, 0.45f};
            const int n = std::min(rate - 1, static_cast<int>(t * rate));
            const float a = t * rate - n;
            if (std::abs(screenY - (samples[n] * (1 - a) + samples[n + 1] * a)) < 0.025f)
                color = {0.9f, 0.06f, 0.015f};
            const int near = static_cast<int>(std::lround(t * rate));
            if (std::hypot((t - static_cast<float>(near) / rate) * 256, (screenY - samples[near]) * 100) < 3)
                color = {0.9f, 0.85f, 0.05f};
            image.At(x, y) = color;
        }
    return image;
}
} // namespace

// 数学入口：计算实际结果，与明确的预期比较，返回真实 PASS/FAIL 数量。
TestResult TestHW33()
{
    MathTestContext checks(33);
    const auto samples = SineSamples(3, 4, 5);
    checks.check(checks.near(samples[0], 0) && checks.near(samples[1], -1) && checks.near(samples[3], 1),
                 "3 Hz 在 4 Hz 的实际样本");
    const auto dense = SineSamples(3, 32, 33);
    checks.check(checks.near(dense[8], -1), "32 Hz 采样保留连续曲线位置");
    bool rejected = false;
    try
    {
        (void)SineSamples(3, 0, 5);
    }
    catch (const std::invalid_argument &)
    {
        rejected = true;
    }
    checks.check(rejected, "拒绝零采样率");
    std::printf("[HW33] 结果：%d PASS，%d FAIL\n", checks.result.passed, checks.result.failed);
    return checks.result;
}

// 图形入口：按注释标明的算法生成实际对照图，不使用静态图片冒充算法结果。
void RenderHW33()
{
    Publish(33, L"信号与 Nyquist 采样", L"同一 3 Hz 正弦：灰线连续、红线离散、亮点样本",
            {{SignalGraph(4), L"4 Hz：发生混叠"}, {SignalGraph(32), L"32 Hz：充分采样"}});
}
