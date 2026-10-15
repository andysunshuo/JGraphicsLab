#pragma once
#include "material/Texture.h"
#include "scene/Light.h"
#include <memory>
#include <vector>

enum class LightingModel
{
    Unlit,
    NormalVisualization,
    Lambert,
    BlinnPhong
};

// 在 Material.cpp 实现；本阶段不要求 PBR、阴影或法线贴图的 TBN 变换。
struct Material
{
    LightingModel model = LightingModel::BlinnPhong;
    Vec3 baseColor{0.8f, 0.5f, 0.2f};
    Vec3 ambient{0.05f, 0.05f, 0.05f};
    Vec3 diffuse{0.8f, 0.8f, 0.8f};
    Vec3 specular{0.5f, 0.5f, 0.5f};
    float shininess = 32;
    // 共享只读纹理，不在片元函数加载；无纹理时使用纯色。
    std::shared_ptr<const Texture> albedoTexture;
    TextureFilter filter = TextureFilter::Bilinear;
    TextureWrap wrap = TextureWrap::Repeat;

    // baseColor 与纹理线性 RGB 逐通道相乘；无纹理直接返回 baseColor。
    Vec3 BaseColorAt(const Vec2 &uv, float lod = 0) const;
    // n/l/v 全部归一化：ambient*base + diffuse*base*max(0,n·l)*光强。
    // Blinn-Phong 再加 specular*pow(max(0,n·normalize(l+v)),shininess)*光强。
    // 背光不加高光，环境项只加一次；复用 Light 的方向与衰减计算。
    // Unlit 返回材质色；法线可视化返回 (normalize(n)+1)/2，不冒充物理光照。
    Vec3 Evaluate(const Vec3 &worldPosition, const Vec3 &worldNormal, const Vec3 &eye, const Vec2 &uv,
                  const std::vector<Light> &lights, float lod = 0) const;
};
