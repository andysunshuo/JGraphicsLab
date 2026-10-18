#pragma once
#include "material/Texture.h"
#include "scene/Light.h"
#include <memory>
#include <vector>

enum class LightingModel
{
    Unlit,               // 直接显示基础颜色，作为光照前后的对照。
    NormalVisualization, // 把法线方向映射为颜色，用来检查坐标空间。
    Lambert,             // 环境项 + 漫反射项。
    BlinnPhong           // 环境项 + 漫反射项 + 半程向量高光项。
};

// 可在调试器分别查看三个线性 RGB 分量；只保存计算结果，不再维护另一套光照公式。
// 在 Material.cpp 的 EvaluateTerms 填充；合成时 ambient+diffuse+specular。
// 环境项只计算一次，不随 lights.size() 增加；无光源时仍可有环境项。
struct LightingTerms
{
    Vec3 ambient{};
    Vec3 diffuse{};
    Vec3 specular{};
};

// 在 material/Material.cpp 补全；Blinn-Phong 公式的唯一权威位置。
// Flat/Gouraud/Phong 只改变调用 Evaluate 的位置，不能在光栅器再写三份光照公式。
// 支持纯色和 MTL 颜色/高光贴图；不实现 PBR、阴影、光追或法线贴图。
struct Material
{
    LightingModel model = LightingModel::BlinnPhong;
    Vec3 baseColor{0.8f, 0.5f, 0.2f}; // 物体的线性 RGB 基色，不是随机面色。
    Vec3 ambient{0.05f, 0.05f, 0.05f}; // 已合并环境光强的 ka，不随光源数量重复叠加。
    Vec3 diffuse{0.8f, 0.8f, 0.8f};    // 漫反射系数 kd，控制受光部分的亮度。
    Vec3 specular{0.5f, 0.5f, 0.5f};   // 高光系数 ks；本阶段高光不乘 baseColor。
    float shininess = 32; // 有限且 >=1；越大，高光通常越集中，不代表光源越亮。
    // 加载阶段建立共享只读纹理；片元函数只采样，不读磁盘。空指针表示使用纯色。
    std::shared_ptr<const Texture> albedoTexture;
    std::shared_ptr<const Texture> specularTexture; // map_Ks 按线性数据读取，只调制高光项。
    Vec2 uvScale{1, 1}, uvOffset{}; // map_Kd 的 -s/-o；先缩放再偏移。
    TextureFilter filter = TextureFilter::Bilinear;
    TextureWrap wrap = TextureWrap::Repeat;

    // 无图返回 baseColor；有图返回 baseColor.Hadamard(按 uvScale/uvOffset 采样的线性颜色)。
    // 不在此加载文件；只做最近邻/双线性/三线性采样，显示前统一编码 sRGB。
    Vec3 BaseColorAt(const Vec2 &uv, float lod = 0) const;

    // 用于 Lambert/BlinnPhong；其他模式调用本函数属于使用错误，抛 std::invalid_argument。
    // 步骤 1：base=BaseColorAt(uv,lod)，result.ambient=ambient.Hadamard(base)。
    // 步骤 2：N=normalize(worldNormal)，V=normalize(eye-worldPosition)。
    // 步骤 3：逐光获取 L=DirectionFrom(P)、I=RadianceAt(P)，ndotl=max(0,N.Dot(L))。
    //   result.diffuse += diffuse.Hadamard(base).Hadamard(I)*ndotl。
    // 步骤 4：仅 BlinnPhong 且 ndotl>0 且 V 非零时，H=normalize(L+V)，
    //   H 非零才累加 specular.Hadamard(I)*pow(max(0,N.Dot(H)),shininess)。
    //   如有 specularTexture，再逐通道乘其线性采样值，只调制高光，不调制漫反射。
    // Lambert 的 specular 恒为零；N/L 为零时该方向没有直接光；V/H 为零时仅跳过高光。
    // 非有限输入/非法系数须明确报错；baseColor/ambient/diffuse/specular 各分量在 [0,1]。
    // 不提前钳制三个分量或做 sRGB 编码，允许合成亮度超过 1，显示时统一处理。
    LightingTerms EvaluateTerms(const Vec3 &worldPosition, const Vec3 &worldNormal, const Vec3 &eye, const Vec2 &uv,
                                const std::vector<Light> &lights, float lod = 0) const;

    // 模式分派：Unlit -> BaseColorAt；NormalVisualization -> (normalize(N)+(1,1,1))*0.5；
    // Lambert/BlinnPhong -> EvaluateTerms -> 三项相加。未知枚举值抛 std::invalid_argument。
    // 零法线的可视化约定显示 (0.5,0.5,0.5)，便于发现缺失数据。
    // 输入位置、法线、eye 在世界空间；返回线性 RGB，不乘调试 palette 或 Vertex::color。
    // Phong 着色在这里重新归一化插值后的法线；不能只在顶点归一化一次。
    // 仅写屏幕时调用 Raster::EncodeLinearColor；不要在这里调用 MakeColor 或重复 Gamma 编码。
    Vec3 Evaluate(const Vec3 &worldPosition, const Vec3 &worldNormal, const Vec3 &eye, const Vec2 &uv,
                  const std::vector<Light> &lights, float lod = 0) const;
};
