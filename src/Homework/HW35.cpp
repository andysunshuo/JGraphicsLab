/*
【HW35：SSAA 超采样，已实现】
选择：Config.h 的 kActiveHomework=35，或 --hw 35。
调用：InitializeApplication -> TestHW35 的真实数值检查；通过后 Runner -> RenderHW35。
具体绘图流程：RenderHW35 -> Cube/MakeModelMatrix -> Render(96/192) -> DrawMesh -> Resolve -> Downsample -> Publish。
结果由 Publish 组织成独立 600×600 中文对照画布。
数学与计算过程：
分别完整渲染 96×96、192×192 目标，后者用 2×2 线性颜色均值缩回 96×96。
每个高分辨率像素都执行覆盖、属性、着色和深度；不是简单放大窗口。
Math 测试检查四个像素的平均以及真实三角形边缘产生中间覆盖值。
计算示例：2×2 像素红、绿、蓝、白相加为 (2,2,2)，除以 4 得线性颜色 (0.5,0.5,0.5)。
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

// 数学入口：计算实际结果，与明确的预期比较，返回真实 PASS/FAIL 数量。
TestResult TestHW35()
{
    MathTestContext checks(35);
    Image image(2, 2);
    image.At(0, 0) = {1, 0, 0};
    image.At(1, 0) = {0, 1, 0};
    image.At(0, 1) = {0, 0, 1};
    image.At(1, 1) = {1, 1, 1};
    checks.check(checks.near3(Downsample(image).At(0, 0), {0.5f, 0.5f, 0.5f}), "SSAA 四像素线性平均");
    PipelineOptions o;
    o.shading = Shading::Flat;
    RenderTarget high(64, 64);
    high.Clear({});
    PipelineStats stats;
    PipelineVertex a{{4.2f, 4.4f}, 0, 1, {{}, {}, {1, 0, 0}, {}}},
        b{{60.8f, 10.1f}, 0, 1, {{}, {}, {1, 0, 0}, {}}}, c{{20.6f, 61.2f}, 0, 1, {{}, {}, {1, 0, 0}, {}}};
    RasterizeAttributedTriangle(a, b, c, high, o, stats);
    const auto down = Downsample(high.Resolve());
    bool partial = false;
    for (const auto &pixel : down.pixels)
        partial |= pixel.x > 0 && pixel.x < 1;
    checks.check(stats.depthWrites > 0 && partial, "真正高分辨率三角形经滤波产生边缘中间色");
    std::printf("[HW35] 结果：%d PASS，%d FAIL\n", checks.result.passed, checks.result.failed);
    return checks.result;
}

// 图形入口：按注释标明的算法生成实际对照图，不使用静态图片冒充算法结果。
void RenderHW35()
{
    PipelineOptions o;
    o.shading = Shading::Flat;
    const auto model = MakeModelMatrix({0, 0, 0}, {0.3f, 0.8f, 0}, {0.7f, 0.7f, 0.7f});
    Publish(35, L"SSAA 超采样", L"相同三维场景：低分辨率 vs 高分辨率完整渲染再平均",
            {{Render(Cube(), o, model, 96), L"96×96 单采样"},
             {Downsample(Render(Cube(), o, model, 192)), L"192×192 → 96×96"}});
}
