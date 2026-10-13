/*
【HW40：Blinn-Phong 高光，已实现】
选择：Config.h 的 kActiveHomework=40，或 --hw 40。
调用：InitializeApplication -> TestHW40 的真实数值检查；通过后 Runner -> RenderHW40。
具体绘图流程：RenderHW40 -> Render(Sphere) -> DrawMesh -> 片元法线 -> BlinnPhong/Lambert -> Publish。
结果由 Publish 组织成独立 600×600 中文对照画布。
数学与计算过程：
h=normalize(l+v)，高光为 specular*pow(max(0,n·h),shininess)，加到 Lambert 项。
材质 baseColor、ambient/diffuse/specular、shininess 和光源位置均可手动调整。
在背光面关闭高光；左右对照展示相同球体增加高光后的变化。
计算示例：n=l=v=(0,0,1) 时 n·h=1；测试颜色为 0.2×(0.1+0.8)+0.6=0.78。
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
TestResult TestHW40()
{
    MathTestContext checks(40);
    Lighting l;
    l.baseColor = {0.2f, 0.2f, 0.2f};
    l.ambient = 0.1f;
    l.diffuse = 0.8f;
    l.specular = 0.6f;
    l.shininess = 8;
    l.eye = l.lightPosition = {0, 0, 1};
    checks.check(checks.near3(BlinnPhong({}, {0, 0, 1}, l), {0.78f, 0.78f, 0.78f}),
                 "正对半程向量产生单位高光因子");
    checks.check(checks.near3(BlinnPhong({}, {0, 0, -1}, l), {0.02f, 0.02f, 0.02f}), "背光面没有错误高光");
    const auto broad = BlinnPhong({}, Normalize({0, 0.5f, 1}), l);
    l.shininess = 64;
    const auto narrow = BlinnPhong({}, Normalize({0, 0.5f, 1}), l);
    checks.check(narrow.x < broad.x, "较大高光指数使离轴高光更集中");
    std::printf("[HW40] 结果：%d PASS，%d FAIL\n", checks.result.passed, checks.result.failed);
    return checks.result;
}

// 图形入口：按注释标明的算法生成实际对照图，不使用静态图片冒充算法结果。
void RenderHW40()
{
    PipelineOptions diffuse, gloss;
    diffuse.specular = false;
    gloss.light.shininess = 64;
    Publish(
        40, L"Blinn-Phong", L"半程向量 h=normalize(l+v)，指数控制高光宽度",
        {{Render(Sphere(), diffuse), L"环境 + 漫反射"}, {Render(Sphere(), gloss), L"增加 Specular 高光"}});
}
