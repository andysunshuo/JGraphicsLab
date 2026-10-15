#pragma once
#include "math/Vec3.h"

// 在 Vec4.cpp 实现。齐次位置 w=1、方向 w=0；默认零向量不是一个位置。
class Vec4
{
  public:
    float x = 0, y = 0, z = 0, w = 0;

    Vec4 operator+(const Vec4 &other) const;
    Vec4 operator-(const Vec4 &other) const;
    Vec4 operator-() const;
    Vec4 operator*(float scalar) const;
    Vec4 operator/(float scalar) const;
    float Dot(const Vec4 &other) const;
    bool IsFinite() const;

    // 创建 {p.x,p.y,p.z,1} 与 {d.x,d.y,d.z,0}，不在调用处反复写规则。
    static Vec4 Position(const Vec3 &position);
    static Vec4 Direction(const Vec3 &direction);
    static Vec4 Lerp(const Vec4 &a, const Vec4 &b, float t);

    // 保留原签名：返回 {x/w,y/w,z/w,1}；不能对 w=0 的方向调用。
    // 先检查所有分量有限且 |w|>1e-8，失败时抛 std::invalid_argument。
    Vec4 PerspectiveDivide() const;
    // 不抛异常：先检查 epsilon 有限且非负、输入有限、|w|>epsilon。
    // 再在临时 Vec3 中计算除法，确认结果有限后才写 out；有限输入仍可能除法溢出。
    // 任一检查失败返回 false 且不改 out，不能先写 out 再检查。
    // 此通用数学函数允许负 w；绘制管线应在 Transform::ToScreen 额外要求 w>epsilon。
    // PerspectiveDivide 应转调此函数，共享一份除法和检查实现。
    bool TryPerspectiveDivide(Vec3 &out, float epsilon = 1e-8f) const;
};

Vec4 operator*(float scalar, const Vec4 &value);
