/*
【HW37：Top-left 共享边规则，已实现】
选择：Config.h 的 kActiveHomework=37，或 --hw 37。
调用：InitializeApplication -> TestHW37 的真实数值检查；通过后 Runner -> RenderHW37。
具体绘图流程：RenderHW37 -> 两次 RasterizeAttributedTriangle -> CoveredSample/IsTopLeftEdge -> coverage -> Publish。
结果由 Publish 组织成独立 600×600 中文对照画布。
数学与计算过程：
复用 IsTopLeftEdge 与 EdgeFunction；CCW 三角形边值等于零时只接受左/上边。
两面拼成正方形，coverage 缓冲显示每样本触碰次数：绿为一次、红为重复、黑为未覆盖。
同时检查退化三角形、顺逆时针规范化和屏幕包围盒边界。
计算示例：8×8 正方形由两个三角形组成，64 个中心样本均覆盖一次；共享对角线只由一侧接收。
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
TestResult TestHW37()
{
    MathTestContext checks(37);
    RenderTarget target(8, 8);
    target.Clear({});
    PipelineStats stats;
    PipelineOptions o;
    o.shading = Shading::Flat;
    auto v = [](float x, float y) { return PipelineVertex{{x, y}, 0, 1, {{}, {}, {0, 1, 0}, {}}}; };
    RasterizeAttributedTriangle(v(0, 0), v(8, 0), v(8, 8), target, o, stats);
    RasterizeAttributedTriangle(v(0, 0), v(8, 8), v(0, 8), target, o, stats);
    checks.check(
        std::all_of(target.coverage.begin(), target.coverage.end(), [](unsigned n) { return n == 1; }),
        "整块正方形共享边无重叠也无裂缝");
    const auto count = stats.depthWrites;
    RasterizeAttributedTriangle(v(2, 2), v(2, 2), v(2, 2), target, o, stats);
    checks.check(stats.depthWrites == count, "退化三角形不写入");
    RenderTarget reverse(8, 8);
    RasterizeAttributedTriangle(v(0, 0), v(8, 8), v(8, 0), reverse, o, stats);
    checks.check(std::accumulate(reverse.coverage.begin(), reverse.coverage.end(), 0u) == 36,
                 "反向绕序仍按同一覆盖规则规范化");
    std::printf("[HW37] 结果：%d PASS，%d FAIL\n", checks.result.passed, checks.result.failed);
    return checks.result;
}

// 图形入口：按注释标明的算法生成实际对照图，不使用静态图片冒充算法结果。
void RenderHW37()
{
    RenderTarget t(96, 96);
    PipelineOptions o;
    o.shading = Shading::Flat;
    PipelineStats stats;
    auto v = [](float x, float y) { return PipelineVertex{{x, y}, 0, 1, {{}, {}, {0.15f, 0.8f, 0.2f}, {}}}; };
    RasterizeAttributedTriangle(v(8, 8), v(88, 8), v(88, 88), t, o, stats);
    RasterizeAttributedTriangle(v(8, 8), v(88, 88), v(8, 88), t, o, stats);
    Image heat(96, 96);
    for (int y = 0; y < 96; ++y)
        for (int x = 0; x < 96; ++x)
        {
            const auto n = t.coverage[t.SampleIndex(x, y, 0)];
            heat.At(x, 95 - y) = n == 1 ? Vec3{0.02f, 0.85f, 0.1f} : n > 1 ? Vec3{1, 0, 0} : Vec3{};
        }
    Publish(37, L"共享边与退化规则", L"正方形两三角形：绿色表示一个样本只被覆盖一次",
            {{t.Resolve(), L"正常颜色结果"}, {heat, L"覆盖次数：绿一次/红重复"}});
}
