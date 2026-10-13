/*
【HW46：纹理解码与最近邻，已实现】
选择：Config.h 的 kActiveHomework=46，或 --hw 46。
调用：InitializeApplication -> TestHW46 的真实数值检查；通过后 Runner -> RenderHW46。
具体绘图流程：RenderHW46 -> ColorTexture -> LoadTexture/FromTexture（仅首次）-> Render -> SampleNearest -> Publish。
结果由 Publish 组织成独立 600×600 中文对照画布。
数学与计算过程：
ColorTexture 首次通过 WIC 解码 Assets/Textures/wood.jpg，缓存线性图像。
Nearest 将 UV 映射到 texel 中心坐标后选择最近整数，支持 Clamp/Repeat，负 UV 也正确环绕。
左图是实际解码材质，右图应用到 UV Cube；每帧不会重新从硬盘读取。
计算示例：2×2 图像的 UV=(0.25,0.75) 映射 s=0、t=0，选到左上红 texel；负 U 先环绕。
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
TestResult TestHW46()
{
    MathTestContext checks(46);
    Image image(2, 2);
    image.At(0, 0) = {1, 0, 0};
    image.At(1, 0) = {0, 1, 0};
    image.At(0, 1) = {0, 0, 1};
    image.At(1, 1) = {1, 1, 1};
    checks.check(checks.near3(SampleNearest(image, {0.25f, 0.75f}), {1, 0, 0}), "texel 中心与 V 翻转正确");
    checks.check(checks.near3(SampleNearest(image, {-0.25f, 0.75f}), {0, 1, 0}), "负 U 的 Repeat 环绕");
    checks.check(checks.near3(SampleNearest(image, {-3, 4}, AddressMode::Clamp), {1, 0, 0}),
                 "Clamp 防止纹理越界");
    checks.check(ColorTexture().width == 1024 && ColorTexture().height == 1024,
                 "真实木材 JPG 成功解码并缓存");
    std::printf("[HW46] 结果：%d PASS，%d FAIL\n", checks.result.passed, checks.result.failed);
    return checks.result;
}

// 图形入口：按注释标明的算法生成实际对照图，不使用静态图片冒充算法结果。
void RenderHW46()
{
    PipelineOptions texture;
    texture.shading = Shading::Texture;
    texture.texture = &ColorTexture();
    texture.filter = TextureFilter::Nearest;
    Publish(46, L"纹理与最近邻采样", L"实际解码 wood.jpg，再通过 UV 选择最近 texel",
            {{ColorTexture(), L"木材原图"},
             {Render(Cube(), texture, MakeScale3D(0.7f, 0.7f, 0.7f)), L"最近邻材质 Cube"}});
}
