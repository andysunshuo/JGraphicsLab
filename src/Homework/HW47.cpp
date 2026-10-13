/*
【HW47：双线性滤波，已实现】
选择：Config.h 的 kActiveHomework=47，或 --hw 47。
调用：InitializeApplication -> TestHW47 的真实数值检查；通过后 Runner -> RenderHW47。
具体绘图流程：RenderHW47 -> FourColors -> Magnify(false/true) -> SampleNearest/SampleBilinear -> Publish。
结果由 Publish 组织成独立 600×600 中文对照画布。
数学与计算过程：
在 s=u*width-0.5、t=(1-v)*height-0.5 附近取四 texel，按小数部分 a、b 加权。
C=(1-a)(1-b)C00+a(1-b)C10+(1-a)bC01+abC11。
两个面板放大同一 2×2 彩色纹理，最近邻分块，双线性产生连续过渡；都在线性颜色空间滤波。
计算示例：UV=(0.5,0.5) 位于四 texel 中心，a=b=0.5；红绿蓝白各占 1/4 得 (0.5,0.5,0.5)。
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
Image FourColors()
{
    Image image(2, 2);
    image.At(0, 0) = {1, 0, 0};
    image.At(1, 0) = {0, 1, 0};
    image.At(0, 1) = {0, 0, 1};
    image.At(1, 1) = {1, 1, 1};
    return image;
}
} // namespace

// 数学入口：计算实际结果，与明确的预期比较，返回真实 PASS/FAIL 数量。
TestResult TestHW47()
{
    MathTestContext checks(47);
    const auto image = FourColors();
    checks.check(checks.near3(SampleBilinear(image, {0.5f, 0.5f}, AddressMode::Clamp), {0.5f, 0.5f, 0.5f}),
                 "四 texel 中心等权平均");
    checks.check(checks.near3(SampleBilinear(image, {0.25f, 0.75f}, AddressMode::Clamp), {1, 0, 0}),
                 "texel 中心权重退化为一个点");
    checks.check(checks.near3(SampleBilinear(image, {-10, 10}, AddressMode::Clamp), {1, 0, 0}),
                 "边界邻域 Clamp 安全");
    std::printf("[HW47] 结果：%d PASS，%d FAIL\n", checks.result.passed, checks.result.failed);
    return checks.result;
}

// 图形入口：按注释标明的算法生成实际对照图，不使用静态图片冒充算法结果。
void RenderHW47()
{
    const auto image = FourColors();
    Publish(47, L"双线性滤波", L"同一 2×2 纹理放大，左最近邻、右双线性",
            {{Magnify(image, false), L"Nearest 分块"}, {Magnify(image, true), L"Bilinear 连续过渡"}});
}
