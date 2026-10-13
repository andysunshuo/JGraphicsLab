/*
【HW34：二维卷积与低通滤波，已实现】
选择：Config.h 的 kActiveHomework=34，或 --hw 34。
调用：InitializeApplication -> TestHW34 的真实数值检查；通过后 Runner -> RenderHW34。
具体绘图流程：RenderHW34 -> Checker -> Decimate；另一分支 Convolve(Gaussian) -> Decimate -> Publish。
结果由 Publish 组织成独立 600×600 中文对照画布。
数学与计算过程：
Convolve 按 g[x,y]=sum K[i,j]*I[x-i,y-j] 进行真正卷积；越界采用边缘钳制。
默认 3×3 Gaussian 核为外积 (1,2,1)^T*(1,2,1)/16，权重和 1 保持常量亮度。
高频棋盘图直接抽取与低通后抽取作对照；非对称核测试验证方向没有误写成相关。
计算示例：单位脉冲中心滤波后为 4/16=0.25；常量 0.25 乘权重总和 1 仍为 0.25。
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
std::vector<float> Gaussian()
{
    return {1 / 16.0f, 2 / 16.0f, 1 / 16.0f, 2 / 16.0f, 4 / 16.0f,
            2 / 16.0f, 1 / 16.0f, 2 / 16.0f, 1 / 16.0f};
}
} // namespace

// 数学入口：计算实际结果，与明确的预期比较，返回真实 PASS/FAIL 数量。
TestResult TestHW34()
{
    MathTestContext checks(34);
    Image constant(5, 5, {0.25f, 0.25f, 0.25f});
    const auto kernel = Gaussian();
    const auto filtered = Convolve(constant, kernel, 3);
    checks.check(checks.near3(filtered.At(0, 0), {0.25f, 0.25f, 0.25f}), "常量亮度与边缘钳制保持");
    Image impulse(5, 5);
    impulse.At(2, 2) = {1, 1, 1};
    checks.check(checks.near(Convolve(impulse, kernel, 3).At(2, 2).x, 0.25f), "脉冲中心对应 Gaussian 权重");
    std::vector<float> asymmetric(9);
    asymmetric[0] = 1;
    const auto shifted = Convolve(impulse, asymmetric, 3);
    checks.check(checks.near(shifted.At(1, 1).x, 1) && checks.near(shifted.At(3, 3).x, 0),
                 "非对称核执行卷积而非相关");
    std::printf("[HW34] 结果：%d PASS，%d FAIL\n", checks.result.passed, checks.result.failed);
    return checks.result;
}

// 图形入口：按注释标明的算法生成实际对照图，不使用静态图片冒充算法结果。
void RenderHW34()
{
    const auto source = Checker(192, 192, 1);
    Publish(34, L"二维卷积与降采样", L"先抑制高频，再缩小图像可以减少混叠",
            {{Decimate(source, 3), L"直接抽样"},
             {Decimate(Convolve(source, Gaussian(), 3), 3), L"Gaussian 后抽样"}});
}
