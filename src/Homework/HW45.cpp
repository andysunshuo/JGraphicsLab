/*
【HW45：UV 映射，已实现】
选择：Config.h 的 kActiveHomework=45，或 --hw 45。
调用：InitializeApplication -> TestHW45 的真实数值检查；通过后 Runner -> RenderHW45。
具体绘图流程：RenderHW45 -> MakeUVTriangle/MakeUVCube -> Render -> 属性裁剪/插值 -> UV 显色或采样 -> Publish。
结果由 Publish 组织成独立 600×600 中文对照画布。
数学与计算过程：
MakeUVTriangle 为三角形设置三个 UV；MakeUVCube 为六面各创建四个独立顶点并分配单位方形 UV。
UV 的 V 向上，图像行号向下，采样时 t=(1-v)*height-0.5。
共享位置允许不同法线/UV：Cube 使用 24 顶点、12 三角形，而不是强行共用 8 个角。
计算示例：三顶点 UV=(0,0),(1,0),(0,1)，λ=(0.25,0.25,0.5)、w 相同，结果 UV=(0.25,0.5)。
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
TestResult TestHW45()
{
    MathTestContext checks(45);
    const auto triangle = MakeUVTriangle(), cube = MakeUVCube();
    checks.check(triangle.vertices.size() == 3 && cube.vertices.size() == 24 && cube.indices.size() == 36,
                 "三角形与六面独立 UV 顶点");
    bool valid = true;
    for (const auto &v : cube.vertices)
        valid &= v.uv.x >= 0 && v.uv.x <= 1 && v.uv.y >= 0 && v.uv.y <= 1;
    checks.check(valid, "每面 UV 均在单位方形内");
    PipelineVertex a, b, c;
    a.attributes.uv = {0, 0};
    b.attributes.uv = {1, 0};
    c.attributes.uv = {0, 1};
    checks.check(checks.near2(InterpolateAttributes(a, b, c, {0.25f, 0.25f, 0.5f}, true).uv, {0.25f, 0.5f}),
                 "三角形 UV 重心插值");
    std::printf("[HW45] 结果：%d PASS，%d FAIL\n", checks.result.passed, checks.result.failed);
    return checks.result;
}

// 图形入口：按注释标明的算法生成实际对照图，不使用静态图片冒充算法结果。
void RenderHW45()
{
    PipelineOptions uv, texture;
    uv.shading = Shading::UV;
    texture.shading = Shading::Texture;
    texture.texture = &CheckerTexture();
    Publish(45, L"UV 坐标映射", L"左：三角形 UV 方向；右：六面各自映射棋盘格",
            {{Render(MakeUVTriangle(), uv), L"UV 可视化"},
             {Render(Cube(), texture, MakeScale3D(0.7f, 0.7f, 0.7f)), L"每面方形 UV"}});
}
