#include "material/Material.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace
{

    constexpr float kEpsilon = 1e-8f;

    // ============================================================
    // 基础数学辅助函数
    // ============================================================

    // 判断 float 是否为有限值。
    // NaN、+inf、-inf 都返回 false。
    bool IsFinite(float value)
    {
        return std::isfinite(value);
    }

    // 判断 Vec2 是否全部为有限值。
    bool IsFinite(const Vec2& v)
    {
        return IsFinite(v.x) &&
            IsFinite(v.y);
    }

    // 判断 Vec3 是否全部为有限值。
    bool IsFinite(const Vec3& v)
    {
        return IsFinite(v.x) &&
            IsFinite(v.y) &&
            IsFinite(v.z);
    }

    // ============================================================
    // Vec3 点乘
    //
    // Dot(A, B)
    // = Ax * Bx + Ay * By + Az * Bz
    // ============================================================

    float Dot3(const Vec3& a, const Vec3& b)
    {
        return a.x * b.x +
            a.y * b.y +
            a.z * b.z;
    }

    // ============================================================
    // Vec3 分量乘法
    //
    // 也就是 Hadamard Product：
    //
    // A ⊙ B
    // = (Ax * Bx,
    //    Ay * By,
    //    Az * Bz)
    //
    // 注意：
    // 这不是普通的矩阵乘法，也不是 Dot Product。
    // ============================================================

    Vec3 Hadamard3(const Vec3& a, const Vec3& b)
    {
        return Vec3{
            a.x * b.x,
            a.y * b.y,
            a.z * b.z
        };
    }

    // ============================================================
    // 安全归一化
    //
    // 返回：
    //   正常向量 -> 单位向量
    //   零向量   -> 零向量
    //
    // 这里不把零向量当成错误。
    // 因为在光照计算中：
    //
    // N = 0
    // L = 0
    // V = 0
    //
    // 都可以按照“该方向没有贡献”处理。
    //
    // 注意：
    // 这里直接使用 x/y/z，避免依赖 Vec3::Normalize() 的具体实现。
    // ============================================================

    Vec3 NormalizeOrZero(const Vec3& v)
    {
        if (!IsFinite(v))
        {
            throw std::invalid_argument(
                "归一化失败：Vec3 包含非有限值");
        }

        const float lengthSquared =
            v.x * v.x +
            v.y * v.y +
            v.z * v.z;

        if (!IsFinite(lengthSquared))
        {
            throw std::invalid_argument(
                "归一化失败：Vec3 长度包含非有限值");
        }

        if (lengthSquared <= kEpsilon * kEpsilon)
        {
            return Vec3{};
        }

        const float length = std::sqrt(lengthSquared);

        if (!IsFinite(length) || length <= kEpsilon)
        {
            return Vec3{};
        }

        return v / length;
    }

    // ============================================================
    // 判断 Vec3 是否为零向量。
    // ============================================================

    bool IsZero(const Vec3& v)
    {
        return
            std::abs(v.x) <= kEpsilon &&
            std::abs(v.y) <= kEpsilon &&
            std::abs(v.z) <= kEpsilon;
    }

    // ============================================================
    // 验证颜色/系数是否位于 [0, 1]。
    //
    // 当前 Material 的设计明确要求：
    // baseColor
    // ambient
    // diffuse
    // specular
    //
    // 每个分量都必须在 [0,1]。
    // ============================================================

    void ValidateColor01(const Vec3& value, const char* name)
    {
        if (!IsFinite(value))
        {
            throw std::invalid_argument(
                std::string(name) + " 包含非有限值");
        }

        if (value.x < 0.0f || value.x > 1.0f ||
            value.y < 0.0f || value.y > 1.0f ||
            value.z < 0.0f || value.z > 1.0f)
        {
            throw std::invalid_argument(
                std::string(name) + " 的分量必须位于 [0,1]");
        }
    }

    // ============================================================
    // 验证 Material 参数。
    // ============================================================

    void ValidateMaterialParameters(const Material& material)
    {
        ValidateColor01(material.baseColor, "baseColor");
        ValidateColor01(material.ambient, "ambient");
        ValidateColor01(material.diffuse, "diffuse");
        ValidateColor01(material.specular, "specular");

        if (!IsFinite(material.shininess))
        {
            throw std::invalid_argument(
                "shininess 必须是有限值");
        }

        if (material.shininess < 1.0f)
        {
            throw std::invalid_argument(
                "shininess 必须 >= 1");
        }
    }

    // ============================================================
    // 验证 Evaluate 的公共输入。
    //
    // worldPosition、worldNormal、eye：世界空间。
    // uv：当前虽然不参与纹理计算，但仍然检查其数值合法性。
    // lod：当前没有真正参与采样，只要求是有限值。
    // ============================================================

    void ValidateEvaluateInputs(
        const Vec3& worldPosition,
        const Vec3& worldNormal,
        const Vec3& eye,
        const Vec2& uv,
        float lod)
    {
        if (!IsFinite(worldPosition))
        {
            throw std::invalid_argument(
                "worldPosition 包含非有限值");
        }

        if (!IsFinite(worldNormal))
        {
            throw std::invalid_argument(
                "worldNormal 包含非有限值");
        }

        if (!IsFinite(eye))
        {
            throw std::invalid_argument(
                "eye 包含非有限值");
        }

        if (!IsFinite(uv))
        {
            throw std::invalid_argument(
                "uv 包含非有限值");
        }

        if (!IsFinite(lod))
        {
            throw std::invalid_argument(
                "lod 必须是有限值");
        }
    }

} // namespace

// ============================================================
// BaseColorAt
//
// 无贴图时直接返回基色，有贴图时返回基色与采样颜色的逐通道乘积。
//
// 读取阶段已缓存纹理；这里仅在线性颜色空间采样。
// ============================================================

Vec3 Material::BaseColorAt(
    const Vec2& uv,
    float lod) const
{
    ValidateColor01(baseColor, "baseColor");
    if (albedoTexture)
    {
        const Vec2 mapped{uv.x * uvScale.x + uvOffset.x, uv.y * uvScale.y + uvOffset.y};
        return baseColor.Hadamard(albedoTexture->Sample(mapped, filter, wrap, lod));
    }

    return baseColor;
}

// ============================================================
// EvaluateTerms
//
// 这是 Lambert / Blinn-Phong 光照公式的唯一实现位置。
//
// 输出：
//     ambient
//     diffuse
//     specular
//
// 最终颜色由 Evaluate() 负责：
//
//     ambient + diffuse + specular
// ============================================================

LightingTerms Material::EvaluateTerms(
    const Vec3& worldPosition,
    const Vec3& worldNormal,
    const Vec3& eye,
    const Vec2& uv,
    const std::vector<Light>& lights,
    float lod) const
{
    // --------------------------------------------------------
    // EvaluateTerms 只允许 Lambert / BlinnPhong。
    // --------------------------------------------------------

    if (model != LightingModel::Lambert &&
        model != LightingModel::BlinnPhong)
    {
        throw std::invalid_argument(
            "EvaluateTerms 仅支持 Lambert 或 BlinnPhong");
    }

    // --------------------------------------------------------
    // 输入合法性检查。
    // --------------------------------------------------------

    ValidateEvaluateInputs(
        worldPosition,
        worldNormal,
        eye,
        uv,
        lod);

    // --------------------------------------------------------
    // Material 参数检查。
    // --------------------------------------------------------

    ValidateMaterialParameters(*this);

    LightingTerms result{};

    // ========================================================
    // STEP 1
    //
    // 获取物体基础颜色。
    //
    // 无图：base = baseColor；有图：base = baseColor ⊙ texture(uv)。
    // ========================================================

    const Vec3 base = BaseColorAt(uv, lod);

    // ========================================================
    // STEP 2
    //
    // Ambient
    //
    // C_ambient
    //     = ka ⊙ baseColor
    //
    // 注意：
    //
    // Ambient 只计算一次。
    //
    // 不能写成：
    //
    // for (lights)
    // {
    //     ambient += ...
    // }
    //
    // 否则光源越多，环境光会被重复计算。
    // ========================================================

    result.ambient =
        Hadamard3(ambient, base);

    // ========================================================
    // STEP 3
    //
    // 计算表面法线 N。
    //
    // worldNormal 本身可能不是单位向量。
    //
    // 所以必须：
    //
    //     N = normalize(worldNormal)
    //
    // 特别是 Phong Shading：
    //
    // 三角形内部插值之后的法线通常不再是单位长度，
    // 因此必须重新 Normalize。
    // ========================================================

    const Vec3 N =
        NormalizeOrZero(worldNormal);

    // ========================================================
    // STEP 4
    //
    // 计算观察方向 V。
    //
    // P = 当前表面点
    // eye = 摄像机位置
    //
    // 所以：
    //
    //     V = normalize(eye - P)
    // ========================================================

    const Vec3 viewVector =
        eye - worldPosition;

    const Vec3 V =
        NormalizeOrZero(viewVector);

    // ========================================================
    // STEP 5
    //
    // 遍历所有光源。
    // ========================================================

    for (const Light& light : lights)
    {
        // ----------------------------------------------------
        // 从当前表面点 P 指向光源的方向。
        //
        //     L = DirectionFrom(P)
        //
        // Light 负责决定：
        //
        //     点光源
        //     方向光
        //     其他光源
        //
        // Material 只需要得到 L。
        // ----------------------------------------------------

        const Vec3 lightDirection =
            light.DirectionFrom(worldPosition);

        if (!IsFinite(lightDirection))
        {
            throw std::invalid_argument(
                "光源方向包含非有限值");
        }

        const Vec3 L =
            NormalizeOrZero(lightDirection);

        // ----------------------------------------------------
        // 获取当前表面点受到的光照强度。
        //
        //     I = RadianceAt(P)
        //
        // Material 不关心光源内部如何计算衰减。
        // ----------------------------------------------------

        const Vec3 I =
            light.RadianceAt(worldPosition);

        if (!IsFinite(I))
        {
            throw std::invalid_argument(
                "光源辐射强度包含非有限值");
        }

        // ----------------------------------------------------
        // Lambert 核心：
        //
        //     N · L
        //
        // 再进行：
        //
        //     max(0, N · L)
        //
        // 如果光源位于表面背面：
        //
        //     N · L < 0
        //
        // 则不应该产生漫反射。
        // ----------------------------------------------------

        const float ndotl =
            std::max(
                0.0f,
                Dot3(N, L));

        // ----------------------------------------------------
        // Diffuse
        //
        //     C_diffuse
        //       = kd
        //       ⊙ baseColor
        //       ⊙ I
        //       * max(0, N · L)
        //
        // 先计算：
        //
        //     kd ⊙ baseColor
        //
        // 再乘光源强度 I。
        // ----------------------------------------------------

        const Vec3 diffuseBase =
            Hadamard3(diffuse, base);

        const Vec3 diffuseLight =
            Hadamard3(diffuseBase, I);

        result.diffuse +=
            diffuseLight * ndotl;

        // ====================================================
        // STEP 6
        //
        // Blinn-Phong Specular
        //
        // Lambert 没有高光。
        //
        // 只有：
        //
        //     model == BlinnPhong
        //
        // 才进入这里。
        // ====================================================

        if (model == LightingModel::BlinnPhong &&
            ndotl > 0.0f &&
            !IsZero(V))
        {
            // ------------------------------------------------
            // Half Vector
            //
            //     H = normalize(L + V)
            //
            // H 是光线方向 L 与观察方向 V 的中间方向。
            // ------------------------------------------------

            const Vec3 H =
                NormalizeOrZero(L + V);

            // ------------------------------------------------
            // 如果 H 是零向量，
            // 就没有有效的高光方向。
            // ------------------------------------------------

            if (!IsZero(H))
            {
                // --------------------------------------------
                // Blinn-Phong：
                //
                //     specFactor
                //       = max(0, N · H)^shininess
                // --------------------------------------------

                const float ndoth =
                    std::max(
                        0.0f,
                        Dot3(N, H));

                const float specularFactor =
                    std::pow(
                        ndoth,
                        shininess);

                // --------------------------------------------
                //     C_specular
                //       = ks ⊙ I
                //       * specularFactor
                //
                // 注意：
                //
                // 本阶段明确不乘 baseColor。
                // --------------------------------------------

                Vec3 specularLight = Hadamard3(specular, I);
                if (specularTexture)
                {
                    const Vec2 mapped{uv.x * uvScale.x + uvOffset.x, uv.y * uvScale.y + uvOffset.y};
                    specularLight = specularLight.Hadamard(specularTexture->Sample(mapped, filter, wrap, lod));
                }

                result.specular +=
                    specularLight *
                    specularFactor;
            }
        }
    }

    if (!result.ambient.IsFinite() || !result.diffuse.IsFinite() || !result.specular.IsFinite())
        throw std::invalid_argument("光照合成结果不是有限值");
    return result;
}

// ============================================================
// Evaluate
//
// 统一的 Material 着色入口。
//
// 不同 LightingModel 在这里分派。
//
// Unlit
//     -> BaseColorAt
//
// NormalVisualization
//     -> 法线映射成颜色
//
// Lambert
//     -> EvaluateTerms
//
// BlinnPhong
//     -> EvaluateTerms
// ============================================================

Vec3 Material::Evaluate(
    const Vec3& worldPosition,
    const Vec3& worldNormal,
    const Vec3& eye,
    const Vec2& uv,
    const std::vector<Light>& lights,
    float lod) const
{
    // --------------------------------------------------------
    // 所有模式都检查输入。
    // --------------------------------------------------------

    ValidateEvaluateInputs(
        worldPosition,
        worldNormal,
        eye,
        uv,
        lod);

    // ========================================================
    // 根据 LightingModel 分派。
    // ========================================================

    switch (model)
    {
        // ========================================================
        // Unlit
        //
        // 完全不考虑光照。
        //
        //     C = baseColor
        // ========================================================

    case LightingModel::Unlit:
    {
        return BaseColorAt(uv, lod);
    }

    // ========================================================
    // Normal Visualization
    //
    //     N = normalize(worldNormal)
    //
    // 然后：
    //
    //     RGB = (N + 1) / 2
    //
    // 将：
    //
    //     [-1,1]
    //
    // 映射到：
    //
    //     [0,1]
    // ========================================================

    case LightingModel::NormalVisualization:
    {
        // 零法线的特殊约定：
        //
        //     (0.5, 0.5, 0.5)
        //
        // 这样一眼就能发现法线数据缺失。
        if (IsZero(worldNormal))
        {
            return Vec3{
                0.5f,
                0.5f,
                0.5f
            };
        }

        const Vec3 N =
            NormalizeOrZero(worldNormal);

        return
            (N + Vec3{ 1.0f, 1.0f, 1.0f }) *
            0.5f;
    }

    // ========================================================
    // Lambert
    //
    //     Ambient + Diffuse
    //
    // Specular 恒为 0。
    // ========================================================

    case LightingModel::Lambert:
    {
        const LightingTerms terms =
            EvaluateTerms(
                worldPosition,
                worldNormal,
                eye,
                uv,
                lights,
                lod);

        return
            terms.ambient +
            terms.diffuse;
    }

    // ========================================================
    // Blinn-Phong
    //
    //     Ambient + Diffuse + Specular
    // ========================================================

    case LightingModel::BlinnPhong:
    {
        const LightingTerms terms =
            EvaluateTerms(
                worldPosition,
                worldNormal,
                eye,
                uv,
                lights,
                lod);

        return
            terms.ambient +
            terms.diffuse +
            terms.specular;
    }

    // ========================================================
    // 防御性编程：
    //
    // enum class 理论上只能出现定义好的枚举值，
    // 但可以通过 static_cast 构造非法枚举值。
    // ========================================================

    default:
        throw std::invalid_argument(
            "未知的 LightingModel");
    }
}
