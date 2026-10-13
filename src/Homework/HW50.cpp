/*
【HW50：LOD 与三线性滤波，已实现】
选择：Config.h 的 kActiveHomework=50，或 --hw 50。
调用：InitializeApplication -> TestHW50 的真实数值检查；通过后 Runner -> RenderHW50。
具体绘图流程：RenderHW50 -> RenderGround -> 每片元估计 UV 屏幕导数 -> EstimateLod -> SampleTrilinear -> Publish。
结果由 Publish 组织成独立 600×600 中文对照画布。
数学与计算过程：
按屏幕 UV 导数估计 rho=max(length(dUV/dx*size),length(dUV/dy*size))，LOD=log2(rho)。
选择相邻两个级别，各自双线性采样，再按 LOD 的小数部分混合；放大时限制到 L0。
左右是同一倾斜地面：左只用基础级双线性，右自动选择 Mipmap，减少远处高频闪烁。
计算示例：像素足迹 rho=4 时 LOD=log2(4)=2；LOD=0.25 的结果为 0.75×L0 双线性 + 0.25×L1 双线性。
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
TestResult TestHW50()
{
    MathTestContext checks(50);
    std::vector<Image> known{Image(2, 2, {1, 0, 0}), Image(1, 1, {0, 0, 1})};
    checks.check(checks.near3(SampleTrilinear(known, {0.5f, 0.5f}, 0.25f), {0.75f, 0, 0.25f}),
                 "LOD 小数在两级双线性结果间插值");
    checks.check(checks.near3(SampleTrilinear(known, {0.5f, 0.5f}, -3), {1, 0, 0}), "放大时钳制到基础级");
    checks.check(checks.near(EstimateLod(Image(64, 64), {1 / 16.0f, 0}, {0, 0}), 2),
                 "四 texel 像素足迹对应 LOD=2");
    checks.check(checks.near3(SampleTrilinear(known, {0, 0}, 99), {0, 0, 1}), "远距离 LOD 钳制到最后一级");
    std::printf("[HW50] 结果：%d PASS，%d FAIL\n", checks.result.passed, checks.result.failed);
    return checks.result;
}

// 图形入口：按注释标明的算法生成实际对照图，不使用静态图片冒充算法结果。
void RenderHW50()
{
    PipelineOptions base, tri;
    base.shading = tri.shading = Shading::Texture;
    base.texture = tri.texture = &CheckerTexture();
    base.filter = TextureFilter::Bilinear;
    tri.filter = TextureFilter::Trilinear;
    tri.mipmaps = &CheckerMipmaps();
    Publish(50, L"LOD 与三线性滤波", L"同一地面：基础级混叠 vs 由屏幕足迹选择 Mipmap",
            {{RenderGround(base), L"只有 L0 双线性"}, {RenderGround(tri), L"自动 LOD 三线性"}});
}
