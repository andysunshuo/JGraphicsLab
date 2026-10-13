/*
【HW39：Lambert 环境与漫反射，已实现】
选择：Config.h 的 kActiveHomework=39，或 --hw 39。
调用：InitializeApplication -> TestHW39 的真实数值检查；通过后 Runner -> RenderHW39。
具体绘图流程：RenderHW39 -> Render(Sphere) -> DrawMesh -> 插值世界位置/法线 -> Lambert -> Resolve -> Publish。
结果由 Publish 组织成独立 600×600 中文对照画布。
数学与计算过程：
统一在世界空间归一化 n 和 l，颜色=baseColor*(ambient+diffuse*max(0,n·l))。
左图只有环境项，右图增加漫反射；背光截断为零，不使用负亮度。
这是教学强度约定，没有混入 PBR 或距离衰减。
计算示例：base=0.5、ambient=0.1、diffuse=0.8：正对光为 0.5×0.9=0.45，背光为 0.05。
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
TestResult TestHW39()
{
    MathTestContext checks(39);
    Lighting l;
    l.baseColor = {0.5f, 0.5f, 0.5f};
    l.ambient = 0.1f;
    l.diffuse = 0.8f;
    l.lightPosition = {0, 0, 1};
    checks.check(checks.near3(Lambert({}, {0, 0, 1}, l), {0.45f, 0.45f, 0.45f}), "正对光源的环境加漫反射");
    checks.check(checks.near3(Lambert({}, {0, 0, -1}, l), {0.05f, 0.05f, 0.05f}), "背光只保留环境项");
    checks.check(checks.near3(Lambert({}, {0, 0, 5}, l), {0.45f, 0.45f, 0.45f}),
                 "法线归一化保证亮度不随长度改变");
    std::printf("[HW39] 结果：%d PASS，%d FAIL\n", checks.result.passed, checks.result.failed);
    return checks.result;
}

// 图形入口：按注释标明的算法生成实际对照图，不使用静态图片冒充算法结果。
void RenderHW39()
{
    PipelineOptions ambient, lit;
    ambient.specular = false;
    lit.specular = false;
    ambient.light.diffuse = 0;
    Publish(39, L"Lambert 漫反射", L"n·光线方向决定表面明暗，背光项截断为零",
            {{Render(Sphere(), ambient), L"仅 Ambient"}, {Render(Sphere(), lit), L"Ambient + Diffuse"}});
}
