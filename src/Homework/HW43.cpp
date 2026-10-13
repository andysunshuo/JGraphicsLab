/*
【HW43：Phong Shading，已实现】
选择：Config.h 的 kActiveHomework=43，或 --hw 43。
调用：InitializeApplication -> TestHW43 的真实数值检查；通过后 Runner -> RenderHW43。
具体绘图流程：RenderHW43 -> Render(CoarseSphere,Phong) -> 裁剪 -> InterpolateAttributes(normal/position) -> BlinnPhong -> Publish。
结果由 Publish 组织成独立 600×600 中文对照画布。
数学与计算过程：
先插值世界位置与法线，重新归一化法线，再在每个片元计算 Blinn-Phong 光照。
与 HW40 的光照模型共用权威实现；这里改变的是光照计算发生在顶点还是片元。
细小高光在面内部也可出现，和 Gouraud 的顶点颜色插值不同。
计算示例：normalize((-1,0,1)) 与 normalize((1,0,1)) 各占一半，平均后重新归一化得 (0,0,1)。
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
TestResult TestHW43()
{
    MathTestContext checks(43);
    PipelineVertex a, b, c;
    a.attributes.normal = Normalize({-1, 0, 1});
    b.attributes.normal = Normalize({1, 0, 1});
    const auto fragment = InterpolateAttributes(a, b, c, {0.5f, 0.5f, 0}, true);
    checks.check(checks.near3(Normalize(fragment.normal), {0, 0, 1}), "插值后重新归一化获得片元法线");
    Lighting l;
    l.lightPosition = l.eye = {0, 0, 1};
    l.shininess = 48;
    const auto perPixel = BlinnPhong({}, fragment.normal, l),
               perVertex = Multiply(
                   Add(BlinnPhong({}, a.attributes.normal, l), BlinnPhong({}, b.attributes.normal, l)), 0.5f);
    checks.check(perPixel.x > perVertex.x, "面内高光不等于端点光照颜色平均");
    std::printf("[HW43] 结果：%d PASS，%d FAIL\n", checks.result.passed, checks.result.failed);
    return checks.result;
}

// 图形入口：按注释标明的算法生成实际对照图，不使用静态图片冒充算法结果。
void RenderHW43()
{
    PipelineOptions gouraud, phong;
    gouraud.shading = Shading::Gouraud;
    phong.shading = Shading::Phong;
    gouraud.light.shininess = phong.light.shininess = 72;
    Publish(43, L"Phong 逐片元着色", L"同一低细分球与高光参数，比较高光细节",
            {{Render(CoarseSphere(), gouraud), L"Gouraud：顶点算光"},
             {Render(CoarseSphere(), phong), L"Phong：片元算光"}});
}
