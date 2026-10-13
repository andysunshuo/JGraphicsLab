/*
【HW38：面法线与顶点法线，已实现】
选择：Config.h 的 kActiveHomework=38，或 --hw 38。
调用：InitializeApplication -> TestHW38 的真实数值检查；通过后 Runner -> RenderHW38。
具体绘图流程：RenderHW38 -> CoarseSphere -> ComputeVertexNormals -> Render(FaceNormal/VertexNormal) -> Publish。
结果由 Publish 组织成独立 600×600 中文对照画布。
数学与计算过程：
面法线为 normalize((B-A)×(C-A))；顶点累加所有邻接面未归一化叉积，再归一化。
叉积长度正比于面面积，因而这是面积加权平滑；索引拆分可保留硬边/UV 接缝。
左面法线颜色按三角形常量，右顶点法线在片元处插值并归一化。
计算示例：A=(0,0,0)、B=(1,0,0)、C=(0,1,0)：(B-A)×(C-A)=(0,0,1)，归一化后不变。
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
TestResult TestHW38()
{
    MathTestContext checks(38);
    checks.check(checks.near3(FaceNormal({0, 0, 0}, {1, 0, 0}, {0, 1, 0}), {0, 0, 1}), "面叉积的右手方向");
    Mesh mesh;
    mesh.vertices = {{{0, 0, 0}, {}, {}}, {{1, 0, 0}, {}, {}}, {{0, 1, 0}, {}, {}}, {{-1, 0, 0}, {}, {}}};
    mesh.indices = {0, 1, 2, 0, 2, 3};
    const auto faces = ComputeVertexNormals(mesh);
    checks.check(faces.size() == 2 && checks.near3(mesh.vertices[0].normal, {0, 0, 1}),
                 "邻接面的面积加权法线");
    checks.check(checks.near3(FaceNormal({0, 0, 0}, {0, 0, 0}, {0, 0, 0}), {}), "退化面安全返回零法线");
    std::printf("[HW38] 结果：%d PASS，%d FAIL\n", checks.result.passed, checks.result.failed);
    return checks.result;
}

// 图形入口：按注释标明的算法生成实际对照图，不使用静态图片冒充算法结果。
void RenderHW38()
{
    Mesh mesh = CoarseSphere();
    ComputeVertexNormals(mesh);
    PipelineOptions face, smooth;
    face.shading = Shading::FaceNormal;
    smooth.shading = Shading::VertexNormal;
    Publish(38, L"面与顶点法线", L"相同网格：逐面方向 vs 邻接面积加权的平滑方向",
            {{Render(mesh, face), L"面法线"}, {Render(mesh, smooth), L"面积加权顶点法线"}});
}
