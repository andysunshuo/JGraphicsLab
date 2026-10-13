/*
【HW41：Flat Shading，已实现】
选择：Config.h 的 kActiveHomework=41，或 --hw 41。
调用：InitializeApplication -> TestHW41 的真实数值检查；通过后 Runner -> RenderHW41。
具体绘图流程：RenderHW41 -> Render(CoarseSphere,Flat) -> 面心/面法线 -> BlinnPhong -> 常量属性插值 -> Publish。
结果由 Publish 组织成独立 600×600 中文对照画布。
数学与计算过程：
DrawMesh 每个面只在面心按面法线计算一次光照，三个顶点共享同一结果。
所以插值颜色仍为同一常量；低细分球能看出逐面平面感。
对照图保留相同几何，只把右图改成插值法线的逐片元着色。
计算示例：同面颜色 C=(0.2,0.4,0.6)，权重 0.2+0.3+0.5=1，所以 ΣλC=C。
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
TestResult TestHW41()
{
    MathTestContext checks(41);
    PipelineVertex a, b, c;
    a.attributes.color = b.attributes.color = c.attributes.color = {0.2f, 0.4f, 0.6f};
    checks.check(
        checks.near3(InterpolateAttributes(a, b, c, {0.2f, 0.3f, 0.5f}, true).color, {0.2f, 0.4f, 0.6f}),
        "同面常量颜色在任意重心权重下保持");
    checks.check(checks.near3(FaceNormal({0, 0, 0}, {0, 1, 0}, {1, 0, 0}), {0, 0, -1}),
                 "面心着色使用真实绕序面法线");
    std::printf("[HW41] 结果：%d PASS，%d FAIL\n", checks.result.passed, checks.result.failed);
    return checks.result;
}

// 图形入口：按注释标明的算法生成实际对照图，不使用静态图片冒充算法结果。
void RenderHW41()
{
    PipelineOptions flat, phong;
    flat.shading = Shading::Flat;
    Publish(41, L"Flat Shading", L"同一低细分球：每面一次光照 vs 每片元光照",
            {{Render(CoarseSphere(), flat), L"逐面 Flat"}, {Render(CoarseSphere(), phong), L"逐片元对照"}});
}
