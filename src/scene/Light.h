#pragma once
#include "math/Vec3.h"

enum class LightType
{
    Directional,
    Point
};

// 在 Light.cpp 实现；所有位置和方向均在世界空间，与材质光照保持一致。
struct Light
{
    LightType type = LightType::Point;
    Vec3 position{2, 2, 3};
    Vec3 direction{0, -1, -1}; // 方向光的“传播方向”，不是表面指向光的方向。
    Vec3 color{1, 1, 1};
    float intensity = 1;
    float constantAttenuation = 1, linearAttenuation = 0, quadraticAttenuation = 0;

    // 点光返回 normalize(position-surface)，方向光返回 normalize(-direction)。
    // 零方向返回零，不能除零；默认无距离衰减，方便先学习 Lambert。
    Vec3 DirectionFrom(const Vec3 &surfacePosition) const;
    // 点光衰减 1/(kc+kl*d+kq*d*d)，方向光不衰减；返回 color*intensity*attenuation。
    // 系数要求有限、非负且分母为正；非法设置抛 std::invalid_argument。
    Vec3 RadianceAt(const Vec3 &surfacePosition) const;
};
