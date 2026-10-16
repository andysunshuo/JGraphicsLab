#pragma once
#include "math/Vec3.h"

enum class LightType
{
    Directional, // 平行光：所有表面的光线传播方向相同，不随距离衰减。
    Point        // 点光：由光源位置和表面位置决定方向，可选择距离衰减。
};

// 着色练习第一步，在 scene/Light.cpp 实现；这里不负责材质、阴影或射线求交。
// 调用关系：Material::EvaluateTerms -> DirectionFrom / RadianceAt。
// 所有位置和方向均在世界空间；不能把模型空间法线与世界空间光方向做点积。
struct Light
{
    LightType type = LightType::Point;
    Vec3 position{2, 2, 3};
    Vec3 direction{0, -1, -1}; // 方向光的“传播方向”，不是表面指向光的方向。
    Vec3 color{1, 1, 1}; // 线性 RGB，各通道在 [0,1]；不是 0～255 的显示颜色。
    float intensity = 1; // 有限且非负的亮度倍率，允许大于 1。
    float constantAttenuation = 1, linearAttenuation = 0, quadraticAttenuation = 0;

    //两个问题
    //① 光从哪个方向照过来？
    //② 光照到这里时有多强？
    // 计算过程：点光 delta=position-surfacePosition；方向光 delta=-direction；
    // 调用 delta.Normalized() 得到 L。L 始终“从表面指向光源”，供 max(0,N.Dot(L)) 使用。
    // 零方向返回零，不能除零；点光恰好位于表面时也返回零，跳过该光的漫反射与高光。
    // 方向光不能使用 position；非法类型或参与运算的非有限坐标抛 std::invalid_argument。
    // 给我一个表面位置 P，告诉我“从这个表面看，光源在哪个方向”。
    Vec3 DirectionFrom(const Vec3 &surfacePosition) const;


    // 计算过程：点光 d=length(position-surfacePosition)，
    // attenuation=1/(kc+kl*d+kq*d*d)；方向光 attenuation=1。
    // 返回 color*intensity*attenuation，不在这里乘 N.Dot(L)、材质或环境光。
    // 默认 (kc,kl,kq)=(1,0,0) 不衰减；(0,0,1) 才是距离平方反比。
    // 有效光源要求 color、intensity 和衰减系数有限且非负，color 各通道不超过 1；
    // 点光分母须为有限正数，结果须有限；非法设置抛 std::invalid_argument，不能生成 Inf/NaN。
    // 这只是经典经验光照的光强输入，不要求实现物理单位、阴影或环境贴图。
    Vec3 RadianceAt(const Vec3 &surfacePosition) const;
};
