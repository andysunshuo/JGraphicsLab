/*
【HW44：法线逆转置变换，已实现】
选择：Config.h 的 kActiveHomework=44，或 --hw 44。
调用：InitializeApplication -> TestHW44 的真实数值检查；通过后 Runner -> RenderHW44。
具体绘图流程：RenderHW44 -> MakeModelMatrix -> Render -> MakeNormalMatrix -> 变换顶点法线 -> BlinnPhong -> Publish。
结果由 Publish 组织成独立 600×600 中文对照画布。
数学与计算过程：
n'=normalize(inverse(A)^T*n)，A 是 Model 的线性 3×3 部分，平移不影响法线。
非均匀缩放直接乘 A 会破坏法线与切向量的垂直关系；左图故意错误，右图正确。
MakeNormalMatrix 求余子式矩阵/行列式，并拒绝奇异矩阵。
计算示例：A=diag(2,1,1)，A⁻ᵀ=diag(0.5,1,1)；n=(1,1,0) 的正确方向为 normalize(0.5,1,0)。
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
TestResult TestHW44()
{
    MathTestContext checks(44);
    const auto model = MakeScale3D(2, 1, 1);
    const auto normalMatrix = MakeNormalMatrix(model);
    const auto n = Normalize({1, 1, 0}), t = Vec3{1, -1, 0};
    const auto correct = Normalize(Multiply(normalMatrix, n));
    const auto tangent4 = Multiply(model, MakeDirection4(t));
    Vec3 tangent{tangent4.x, tangent4.y, tangent4.z};
    checks.check(checks.near(Dot(correct, tangent), 0), "逆转置法线仍垂直于变换后的切线");
    checks.check(checks.nearMat3(MakeNormalMatrix(MakeTranslation3D(4, 5, 6)), IdentityMat3()),
                 "平移不进入法线矩阵");
    bool rejected = false;
    try
    {
        (void)MakeNormalMatrix(MakeScale3D(0, 1, 1));
    }
    catch (const std::invalid_argument &)
    {
        rejected = true;
    }
    checks.check(rejected, "拒绝不可逆的零尺度矩阵");
    std::printf("[HW44] 结果：%d PASS，%d FAIL\n", checks.result.passed, checks.result.failed);
    return checks.result;
}

// 图形入口：按注释标明的算法生成实际对照图，不使用静态图片冒充算法结果。
void RenderHW44()
{
    PipelineOptions wrong, correct;
    wrong.correctNormals = false;
    correct.correctNormals = true;
    const auto model = MakeModelMatrix({0, 0, 0}, {0.15f, 0.5f, 0}, {1.5f, 0.55f, 1});
    Publish(44, L"非均匀缩放与法线", L"左图故意直接乘 Model；右图使用逆转置",
            {{Render(Sphere(), wrong, model), L"错误：直接乘 A"},
             {Render(Sphere(), correct, model), L"正确：inverse(A) 转置"}});
}
