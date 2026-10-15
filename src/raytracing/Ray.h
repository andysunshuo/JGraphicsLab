#pragma once
#include "math/Vec2.h"
#include "math/Vec3.h"
#include <limits>

// 在 Ray.cpp 实现；direction 不强制单位化，只有单位方向时 t 才等于距离。
struct Ray
{
    Vec3 origin{};
    Vec3 direction{0, 0, -1};
    float tMin = 1e-4f, tMax = std::numeric_limits<float>::infinity();
    Vec3 At(float t) const; // origin+direction*t。
    // origin/direction 有限、方向非零、tMin>=0、tMax>=tMin；允许 tMax 为正无穷。
    bool IsValid() const;
};
struct HitRecord
{
    float t = std::numeric_limits<float>::infinity();
    Vec3 position{}, normal{};
    Vec2 uv{};
    Vec3 barycentric{}; // 三顶点权重；给插值法线与 UV 复用。
    unsigned int triangleIndex = std::numeric_limits<unsigned int>::max();
    bool frontFace = true;
};

// Möller-Trumbore：计算 e1/e2、p=direction×e2、det=e1·p、u/v/t。
// 平行或退化面、u/v 越界、t 不在 Ray 范围内时返回 false，不改 out。
// 命中时填 position、面法线和重心坐标；Mesh 属性插值由 BVH 的面测试负责。
// 这是唯一的射线三角形求交实现，BVH 只能调用，不能复制另一份公式。
bool IntersectTriangle(const Ray &ray, const Vec3 &a, const Vec3 &b, const Vec3 &c, HitRecord &out,
                       bool cullBackFaces = false);
