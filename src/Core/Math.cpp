#include "Core/Math.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <stdexcept>
#include <limits>

float Clamp(
    float value,
    float minValue,
    float maxValue)
{
    if (value < minValue)
    {
        return minValue;
    }

    if (value > maxValue)
    {
        return maxValue;
    }

    return value;
}

// ============================================================
// HW16 — Vec2
// 二维向量数学
// ============================================================

// 功能：
// 计算两个向量之和。
//
// 输入：
// a=(ax,ay)
// b=(bx,by)
//
// 输出：
// a+b
//
// 测试：
// (1,2)+(3,4)=(4,6)
Vec2 Add(const Vec2& a, const Vec2& b)
{
    return { a.x + b.x, a.y + b.y };
}

// 功能：
// 计算两个向量之差。
//
// 测试：
// (5,7)-(2,3)=(3,4)
Vec2 Subtract(const Vec2& a, const Vec2& b)
{
    return { a.x - b.x, a.y - b.y };
}

// 功能：
// 计算二维向量点积。
//
// 数学公式：
// dot(a,b)=ax*bx+ay*by
//
// 测试：
// dot((1,2),(3,4))=11
//
// 思考：
// 两个互相垂直的非零向量，点积是多少？
float Dot(const Vec2& a, const Vec2& b)
{
    return a.x * b.x + a.y * b.y;
}

// 功能：
// 计算向量长度。
//
// 数学公式：
// length=sqrt(x*x+y*y)
//
// 测试：
// (3,4) 的长度为 5。
float Length(const Vec2& v)
{
    return std::sqrt(Dot(v, v));
}

// 功能：
// 将向量变成单位向量。
//
// 输入：
// 任意非零向量。
//
// 输出：
// 方向不变，长度为 1 的向量。
//
// 数学公式：
// normalized(v)=v/length(v)
//
// 思考：
// 如果输入的是零向量，应该如何处理？
Vec2 Normalize(const Vec2& v)
{
    const float length = Length(v);
    if (length <= 1e-7f) return { 0.0f, 0.0f };  // 零向量没有确定方向。
    return { v.x / length, v.y / length };
}

// ------------------------------------------------------------
// HW16 补充 — Vec2 数乘
// 输入：(3,4)，scalar=2。
// 期望输出：(6,8)。
// 为什么需要数乘？移动、速度计算、归一化都需要。
// ------------------------------------------------------------
Vec2 Multiply(const Vec2& v, float scalar)
{
    return { v.x * scalar, v.y * scalar };
}

// ------------------------------------------------------------
// HW17.1 — Vec3 加法
// 输入：(1,2,3) 与 (4,5,6)
// 期望：(5,7,9)
// 提示：每个对应分量相加。
// ------------------------------------------------------------
Vec3 Add(const Vec3& a, const Vec3& b)
{
    return { a.x + b.x, a.y + b.y, a.z + b.z };
}

// HW17.2 — Vec3 减法；测试：(5,7,9)-(1,2,3)=(4,5,6)。
Vec3 Subtract(const Vec3& a, const Vec3& b)
{
    return { a.x - b.x, a.y - b.y, a.z - b.z };
}

// HW17.3 — Vec3 数乘；测试：2*(1,2,3)=(2,4,6)。
Vec3 Multiply(const Vec3& v, float scalar)
{
    return { v.x * scalar, v.y * scalar, v.z * scalar };
}

// ------------------------------------------------------------
// HW17.4 — Dot Product，三维点积
// dot(a,b)=ax*bx+ay*by+az*bz
// 测试：Dot((1,2,3),(4,5,6)) = 32。
// 思考：单位向量的点积与它们夹角余弦的关系？
// ------------------------------------------------------------
float Dot(const Vec3& a, const Vec3& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

// ------------------------------------------------------------
// HW17.5 — Cross Product，三维叉积
// cross(a,b) = (ay*bz-az*by, az*bx-ax*bz, ax*by-ay*bx)
// 测试：Cross((1,0,0),(0,1,0)) = (0,0,1)。
// 思考：为什么交换 a、b 的顺序，结果方向会反转？
// ------------------------------------------------------------
Vec3 Cross(const Vec3& a, const Vec3& b)
{
    // 右手法则：X 叉乘 Y 得到 +Z。
    return { a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x };
}

// HW17.6 — 长度；测试：Length((2,3,6))=7。
float Length(const Vec3& v)
{
    return std::sqrt(Dot(v, v));
}

// HW17.7 — 归一化；测试：Normalize((0,0,5))=(0,0,1)。
Vec3 Normalize(const Vec3& v)
{
    const float length = Length(v);
    if (length <= 1e-7f) return { 0.0f, 0.0f, 0.0f };
    return Multiply(v, 1.0f / length);
}

// HW18.3 — Vec4 加法；测试：(1,2,3,1)+(2,3,4,0)=(3,5,7,1)。
Vec4 Add(const Vec4& a, const Vec4& b)
{
    return { a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
}

// HW18.4 — Vec4 数乘；测试：Multiply((1,2,3,4),2)=(2,4,6,8)。
Vec4 Multiply(const Vec4& v, float scalar)
{
    return { v.x * scalar, v.y * scalar, v.z * scalar, v.w * scalar };
}

// ------------------------------------------------------------
// HW19.1 — 3x3 单位矩阵
// 对角线为 1，其他为 0；I * v = v。
// 注意：这里使用普通的 Mat3{ }，初始值为全 0，
// 需要自己填写对角元素！
// ------------------------------------------------------------
Mat3 IdentityMat3()
{
    Mat3 m = {};
    for (int i = 0; i < 3; ++i) m.m[i][i] = 1.0f;
    return m;
}

// ------------------------------------------------------------
// HW19.2 — 3x3 矩阵乘法：C = A * B
// C[row][col] = sum(A[row][k] * B[k][col]), k=0..2
// 测试：I*A=A、A*I=A；矩阵乘法通常不满足交换律。
// ------------------------------------------------------------
Mat3 Multiply(const Mat3& a, const Mat3& b)
{
    Mat3 result = {};
    // 左矩阵的某一行，与右矩阵的某一列做点积。
    for (int row = 0; row < 3; ++row)
        for (int col = 0; col < 3; ++col)
            for (int k = 0; k < 3; ++k)
                result.m[row][col] += a.m[row][k] * b.m[k][col];
    return result;
}

// ------------------------------------------------------------
// HW19.3 — 3x3 矩阵乘 Vec3
// result[i] = M[i][0]*v.x + M[i][1]*v.y + M[i][2]*v.z
// 测试：Multiply(IdentityMat3(),{2,3,1}) -> (2,3,1)
// ------------------------------------------------------------
Vec3 Multiply(const Mat3& matrix, const Vec3& v)
{
    return {
        matrix.m[0][0] * v.x + matrix.m[0][1] * v.y + matrix.m[0][2] * v.z,
        matrix.m[1][0] * v.x + matrix.m[1][1] * v.y + matrix.m[1][2] * v.z,
        matrix.m[2][0] * v.x + matrix.m[2][1] * v.y + matrix.m[2][2] * v.z
    };
}

// ------------------------------------------------------------
// HW20.1 — 4x4 单位矩阵
// 测试：I * (2,3,4,1) = (2,3,4,1)
// ------------------------------------------------------------
Mat4 IdentityMat4()
{
    Mat4 m = {};
    for (int i = 0; i < 4; ++i) m.m[i][i] = 1.0f;
    return m;
}

// ------------------------------------------------------------
// HW20.2 — 4x4 矩阵乘法
// C[row][col] = sum(A[row][k] * B[k][col]), k=0..3
// 测试：A*I=I*A=A。
// ------------------------------------------------------------
Mat4 Multiply(const Mat4& a, const Mat4& b)
{
    Mat4 result = {};
    for (int row = 0; row < 4; ++row)
        for (int col = 0; col < 4; ++col)
            for (int k = 0; k < 4; ++k)
                result.m[row][col] += a.m[row][k] * b.m[k][col];
    return result;
}

// ------------------------------------------------------------
// HW20.3 — 4x4 矩阵乘 Vec4
// 输入矩阵、(x,y,z,w)，返回变换后的四维坐标。
// 注意：不能丢掉结果中的 w；投影后 w 往往不等于 1。
// ------------------------------------------------------------
Vec4 Multiply(const Mat4& matrix, const Vec4& v)
{
    return {
        matrix.m[0][0] * v.x + matrix.m[0][1] * v.y + matrix.m[0][2] * v.z + matrix.m[0][3] * v.w,
        matrix.m[1][0] * v.x + matrix.m[1][1] * v.y + matrix.m[1][2] * v.z + matrix.m[1][3] * v.w,
        matrix.m[2][0] * v.x + matrix.m[2][1] * v.y + matrix.m[2][2] * v.z + matrix.m[2][3] * v.w,
        matrix.m[3][0] * v.x + matrix.m[3][1] * v.y + matrix.m[3][2] * v.z + matrix.m[3][3] * v.w
    };
}
