#pragma once
#include "Core/Math.h"
#include <vector>

struct TestResult
{
    int passed = 0;
    int failed = 0;
};
// 复用原测试的比较规则，并输出实际值与预期值。
class MathTestContext
{
  public:
    explicit MathTestContext(int homework) : homework_(homework)
    {
    }
    TestResult result;
    bool near(float actual, float expected, float eps = 1e-4f) const;
    bool near2(const Vec2 &actual, const Vec2 &expected) const;
    bool near3(const Vec3 &actual, const Vec3 &expected) const;
    bool near4(const Vec4 &actual, const Vec4 &expected) const;
    bool nearMat3(const Mat3 &actual, const Mat3 &expected) const;
    bool nearMat4(const Mat4 &actual, const Mat4 &expected) const;
    void check(bool ok, const char *description);

  private:
    int homework_;
};

void PrintVec2Test(const char *title, const Vec2 &v);

void PrintVec3Test(const char *title, const Vec3 &v);

void PrintVec4Test(const char *title, const Vec4 &v);

void PrintMat3Test(const char *title, const Mat3 &mat);

void PrintMat4Test(const char *title, const Mat4 &mat);
