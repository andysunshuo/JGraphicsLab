/*
【HW42：Gouraud Shading，已实现】
选择：Config.h 的 kActiveHomework=42，或 --hw 42。
调用：InitializeApplication -> TestHW42 的真实数值检查；通过后 Runner -> RenderHW42。
具体绘图流程：RenderHW42 -> Render(CoarseSphere,Gouraud) -> 三顶点光照 -> 裁剪 -> InterpolateAttributes(color) -> Publish。
结果由 Publish 组织成独立 600×600 中文对照画布。
数学与计算过程：
在三个顶点计算光照，裁剪时插值顶点光照属性，再用校正权重插值颜色。
片元不再计算光照；小高光落在面内但不落顶点上时，Gouraud 可能漏掉它。
本章用相同低细分网格对照 Flat 与 Gouraud。
计算示例：红绿蓝三个顶点的权重为 (0.2,0.3,0.5)，w 相同时结果就是 (0.2,0.3,0.5)。
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
TestResult TestHW42()
{
    MathTestContext checks(42);
    PipelineVertex a, b, c;
    a.attributes.color = {1, 0, 0};
    b.attributes.color = {0, 1, 0};
    c.attributes.color = {0, 0, 1};
    checks.check(
        checks.near3(InterpolateAttributes(a, b, c, {0.2f, 0.3f, 0.5f}, true).color, {0.2f, 0.3f, 0.5f}),
        "三顶点光照颜色插值");
    a.reciprocalW = 1;
    b.reciprocalW = 0.5f;
    c.reciprocalW = 1;
    checks.check(
        checks.near3(InterpolateAttributes(a, b, c, {0.5f, 0.5f, 0}, true).color, {2 / 3.0f, 1 / 3.0f, 0}),
        "顶点光照属性也经过透视校正");
    std::printf("[HW42] 结果：%d PASS，%d FAIL\n", checks.result.passed, checks.result.failed);
    return checks.result;
}

// 图形入口：按注释标明的算法生成实际对照图，不使用静态图片冒充算法结果。
void RenderHW42()
{
    PipelineOptions flat, gouraud;
    flat.shading = Shading::Flat;
    gouraud.shading = Shading::Gouraud;
    Publish(42, L"Gouraud 顶点着色", L"先在顶点照明，再插值颜色；同几何对比",
            {{Render(CoarseSphere(), flat), L"Flat 面颜色"},
             {Render(CoarseSphere(), gouraud), L"Gouraud 顶点颜色"}});
}
