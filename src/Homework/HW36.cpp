/*
【HW36：三维 MSAA 与样本深度，已实现】
选择：Config.h 的 kActiveHomework=36，或 --hw 36。
调用：InitializeApplication -> TestHW36 的真实数值检查；通过后 Runner -> RenderHW36。
具体绘图流程：RenderHW36 -> Occlusion(1/4) -> RasterizeAttributedTriangle -> 逐样本深度 -> Resolve -> Publish。
结果由 Publish 组织成独立 600×600 中文对照画布。
数学与计算过程：
RenderTarget 为每个像素保存四份覆盖、颜色、深度，四个偏移为 0.25/0.75 的组合。
蓝色远面与红色近面在同一三维屏幕空间重叠，先逐样本深度比较，再平均颜色 Resolve。
覆盖边缘的远近关系不被一个像素共享深度替代；本参考实现按样本着色便于检查。
计算示例：远面 NDC z=0.6 映射 depth=0.8，近面 z=-0.4 映射 depth=0.3；0.3<0.8，近面写入。
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
TestResult TestHW36()
{
    MathTestContext checks(36);
    RenderTarget target(8, 8, 4);
    target.Clear({});
    PipelineOptions o;
    o.shading = Shading::Flat;
    PipelineStats stats;
    auto v = [](float x, float y, float z, Vec3 color) {
        return PipelineVertex{{x, y}, z, 1, {{}, {}, color, {}}};
    };
    RasterizeAttributedTriangle(v(0, 0, 0.6f, {0, 0, 1}), v(8, 0, 0.6f, {0, 0, 1}), v(0, 8, 0.6f, {0, 0, 1}),
                                target, o, stats);
    bool partial = false;
    for (int s = 0; s < 4; ++s)
        partial |= target.depths[target.SampleIndex(3, 4, s)] == 1;
    checks.check(partial, "边缘像素存在未覆盖的独立深度样本");
    RasterizeAttributedTriangle(v(0, 0, -0.4f, {1, 0, 0}), v(8, 0, -0.4f, {1, 0, 0}),
                                v(0, 8, -0.4f, {1, 0, 0}), target, o, stats);
    bool near = true;
    for (int s = 0; s < 4; ++s)
        near &= std::abs(target.depths[target.SampleIndex(2, 2, s)] - 0.3f) < 1e-5f;
    checks.check(near, "四份样本分别被近面更新为 0.3");
    RasterizeAttributedTriangle(v(0, 0, 0.6f, {0, 0, 1}), v(8, 0, 0.6f, {0, 0, 1}), v(0, 8, 0.6f, {0, 0, 1}),
                                target, o, stats);
    checks.check(checks.near3(target.Resolve().At(2, 5), {1, 0, 0}), "重画远面不覆盖已通过的近样本");
    std::printf("[HW36] 结果：%d PASS，%d FAIL\n", checks.result.passed, checks.result.failed);
    return checks.result;
}

// 图形入口：按注释标明的算法生成实际对照图，不使用静态图片冒充算法结果。
void RenderHW36()
{
    Publish(36, L"三维 MSAA", L"近红面与远蓝面：独立样本覆盖、深度、颜色后解析",
            {{Occlusion(1), L"中心 1 样本"}, {Occlusion(4), L"每像素 4 样本深度"}});
}
