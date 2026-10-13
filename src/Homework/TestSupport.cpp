#include "Homework/TestSupport.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <stdexcept>
#include <limits>

namespace
{
bool NearValue(float actual, float expected, float eps = 1e-4f)
{
    return std::isfinite(actual) && std::isfinite(expected) && std::abs(actual - expected) <= eps;
}
} // namespace
bool MathTestContext::near(float a, float b, float eps) const
{
    std::printf("  实际值：%.6f；预期值：%.6f\n", a, b);
    return NearValue(a, b, eps);
}
bool MathTestContext::near2(const Vec2 &a, const Vec2 &b) const
{
    PrintVec2Test("  实际值", a);
    PrintVec2Test("  预期值", b);
    return NearValue(a.x, b.x) && NearValue(a.y, b.y);
}
bool MathTestContext::near3(const Vec3 &a, const Vec3 &b) const
{
    PrintVec3Test("  实际值", a);
    PrintVec3Test("  预期值", b);
    return NearValue(a.x, b.x) && NearValue(a.y, b.y) && NearValue(a.z, b.z);
}
bool MathTestContext::near4(const Vec4 &a, const Vec4 &b) const
{
    PrintVec4Test("  实际值", a);
    PrintVec4Test("  预期值", b);
    return NearValue(a.x, b.x) && NearValue(a.y, b.y) && NearValue(a.z, b.z) && NearValue(a.w, b.w);
}
bool MathTestContext::nearMat3(const Mat3 &a, const Mat3 &b) const
{
    PrintMat3Test("  实际值", a);
    PrintMat3Test("  预期值", b);
    for (int r = 0; r < 3; ++r)
        for (int c = 0; c < 3; ++c)
            if (!NearValue(a.m[r][c], b.m[r][c]))
                return false;
    return true;
}
bool MathTestContext::nearMat4(const Mat4 &a, const Mat4 &b) const
{
    PrintMat4Test("  实际值", a);
    PrintMat4Test("  预期值", b);
    for (int r = 0; r < 4; ++r)
        for (int c = 0; c < 4; ++c)
            if (!NearValue(a.m[r][c], b.m[r][c]))
                return false;
    return true;
}
void MathTestContext::check(bool ok, const char *description)
{
    std::printf("[HW%02d] %s  %s\n", homework_, ok ? "PASS" : "FAIL", description);
    if (ok)
        ++result.passed;
    else
        ++result.failed;
}

// ============================================================
// 十六、作业辅助测试与场景绘制
// ============================================================
// 以下 Demo 复用已经实现的数学函数，负责组织数据和调用。
// 当前窗口只有 100x100：向量/矩阵结果用 printf 输出；
// HW15、HW21、HW29、HW30、HW31 也可以观察图像。
// 若 Windows GUI 没有控制台，可在调试器的输出/断点处查看变量。

void PrintVec2Test(const char *title, const Vec2 &v)
{
    std::printf("%s = (%.4f, %.4f)\n", title, v.x, v.y);
}

void PrintVec3Test(const char *title, const Vec3 &v)
{
    std::printf("%s = (%.4f, %.4f, %.4f)\n", title, v.x, v.y, v.z);
}

void PrintVec4Test(const char *title, const Vec4 &v)
{
    std::printf("%s = (%.4f, %.4f, %.4f, %.4f)\n", title, v.x, v.y, v.z, v.w);
}

void PrintMat3Test(const char *title, const Mat3 &mat)
{
    std::printf("%s:\n", title);
    for (int row = 0; row < 3; ++row)
        std::printf("  %8.4f %8.4f %8.4f\n", mat.m[row][0], mat.m[row][1], mat.m[row][2]);
}

void PrintMat4Test(const char *title, const Mat4 &mat)
{
    std::printf("%s:\n", title);
    for (int row = 0; row < 4; ++row)
        std::printf("  %8.4f %8.4f %8.4f %8.4f\n", mat.m[row][0], mat.m[row][1], mat.m[row][2],
                    mat.m[row][3]);
}
