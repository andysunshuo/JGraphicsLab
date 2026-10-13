#pragma once
#include "Core/Color.h"
#include <vector>

// 二维数学向量
//
// 与 Point2 的数据结构相同，但数学含义不同。
//
// Point2：表示一个位置。
// Vec2：表示一个方向或位移。
//
// 例如：
// Vec2 direction = {3.0f, 4.0f};
//
// HW16 会专门实现 Vec2 的数学运算。
struct Vec2
{
    float x;
    float y;
};

float Clamp(
    float value,
    float minValue,
    float maxValue);

Vec2 Add(const Vec2& a, const Vec2& b);

Vec2 Subtract(const Vec2& a, const Vec2& b);

float Dot(const Vec2& a, const Vec2& b);

float Length(const Vec2& v);

Vec2 Normalize(const Vec2& v);

Vec2 Multiply(const Vec2& v, float scalar);

// ============================================================
// HW17 ~ HW31：三维数学、MVP 与 CPU 光栅化
// ============================================================
// 本部分采用统一约定：
// 1. 右手坐标系；相机朝局部 -Z 方向观察。
// 2. 矩阵采用 m[row][column]，向量为列向量：p' = M * p。
// 3. 组合顺序：Clip = Projection * View * Model * Position。
// 4. OpenGL 风格 Clip/NDC：-w<=x,y,z<=w；NDC z∈[-1,1]。
// 5. Viewport 保持当前 Framebuffer 的 Y 向上约定。
// 6. 深度值存入 depthBuffer 前映射到 [0,1]；越小越近。
// 7. HW15~HW31 已完成，可通过 RunMathHomeworkTest 校验。
// ============================================================

// ============================================================
// 四、HW17 / HW18：向量结构与运算
// ============================================================

// HW17 — Vec3：三维向量。可表示位移、方向、法线，也可保存三维位置。
// 示例：Vec3 v = {1.0f, 2.0f, 3.0f};
struct Vec3
{
    float x;
    float y;
    float z;
};

// HW18 — Vec4：四维向量。第四个分量 w 与齐次坐标有关。
// Position：(x,y,z,1)，Direction：(x,y,z,0)。
// 平移矩阵影响 Position，不影响 Direction。
struct Vec4
{
    float x;
    float y;
    float z;
    float w;
};

Vec3 Add(const Vec3& a, const Vec3& b);

Vec3 Subtract(const Vec3& a, const Vec3& b);

Vec3 Multiply(const Vec3& v, float scalar);

float Dot(const Vec3& a, const Vec3& b);

Vec3 Cross(const Vec3& a, const Vec3& b);

float Length(const Vec3& v);

Vec3 Normalize(const Vec3& v);

Vec4 Add(const Vec4& a, const Vec4& b);

Vec4 Multiply(const Vec4& v, float scalar);

// ============================================================
// 五、HW19 / HW20：矩阵结构与基础运算
// ============================================================

// m[row][col] = 第 row 行、第 col 列；从 0 开始编号。
struct Mat3
{
    float m[3][3] = {};
};

struct Mat4
{
    float m[4][4] = {};
};

Mat3 IdentityMat3();

Mat3 Multiply(const Mat3& a, const Mat3& b);

Vec3 Multiply(const Mat3& matrix, const Vec3& v);

Mat4 IdentityMat4();

Mat4 Multiply(const Mat4& a, const Mat4& b);

Vec4 Multiply(const Mat4& matrix, const Vec4& v);
