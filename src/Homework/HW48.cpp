/*
【HW48：透视正确属性插值，已实现】
选择：Config.h 的 kActiveHomework=48，或 --hw 48。
调用：InitializeApplication -> TestHW48 的真实数值检查；通过后 Runner -> RenderHW48。
具体绘图流程：RenderHW48 -> Ground/CheckerTexture -> RenderGround -> 裁剪 -> InterpolateAttributes -> SampleBilinear -> Publish。
结果由 Publish 组织成独立 600×600 中文对照画布。
数学与计算过程：
一般属性 q=(sum lambda*q/w)/(sum lambda/w)，不同深度的 UV 不能直接线性插值。
保留每顶点 reciprocalW；齐次裁剪先线性生成 worldPosition/normal/uv，除 w 后才用修正权重。
深度仍线性插值 NDC z，不把它错误套用一般属性公式。左地面故意仿射，右图透视正确。
计算示例：两点 u=0/1、w=1/2、λ=1/2：u=(0+0.5/2)/(0.5/1+0.5/2)=1/3，直接插值得 1/2。
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
TestResult TestHW48()
{
    MathTestContext checks(48);
    PipelineVertex a, b, c;
    a.reciprocalW = 1;
    b.reciprocalW = 0.5f;
    a.attributes.uv = {0, 0};
    b.attributes.uv = {1, 0};
    checks.check(checks.near(InterpolateAttributes(a, b, c, {0.5f, 0.5f, 0}, true).uv.x, 1 / 3.0f),
                 "不同 w 的校正 UV 为三分之一");
    checks.check(checks.near(InterpolateAttributes(a, b, c, {0.5f, 0.5f, 0}, false).uv.x, 0.5f),
                 "未校正会错误得到一半");
    ClipVertex x{{0, 0, -2, 1}, {}, {0, 0, 0}, {1, 0, 0}, {0, 0}},
        y{{1, 0, 0, 1}, {}, {2, 2, 2}, {0, 1, 0}, {1, 1}};
    const auto midpoint = InterpolateClipVertex(x, y, 0.25f);
    checks.check(checks.near2(midpoint.uv, {0.25f, 0.25f}) &&
                     checks.near3(midpoint.worldPosition, {0.5f, 0.5f, 0.5f}),
                 "裁剪交点也保留一般属性");
    std::printf("[HW48] 结果：%d PASS，%d FAIL\n", checks.result.passed, checks.result.failed);
    return checks.result;
}

// 图形入口：按注释标明的算法生成实际对照图，不使用静态图片冒充算法结果。
void RenderHW48()
{
    PipelineOptions affine, correct;
    affine.shading = correct.shading = Shading::Texture;
    affine.texture = correct.texture = &CheckerTexture();
    affine.perspectiveCorrect = false;
    correct.perspectiveCorrect = true;
    Publish(48, L"透视校正", L"同一倾斜地面与重复纹理，左故意仿射，右正确",
            {{RenderGround(affine), L"直接 lambda 插值"}, {RenderGround(correct), L"插值 q/w 与 1/w"}});
}
