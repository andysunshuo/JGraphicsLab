#include "scene/Light.h"

#include <cmath>
#include <stdexcept>

namespace
{

    bool IsFiniteVec3(const Vec3& v)
    {
        return std::isfinite(v.x) &&
            std::isfinite(v.y) &&
            std::isfinite(v.z);
    }

    bool IsValidLightType(LightType type)
    {
        switch (type)
        {
        case LightType::Directional:
        case LightType::Point:
            return true;

        default:
            return false;
        }
    }

} // namespace

Vec3 Light::DirectionFrom(const Vec3& surfacePosition) const
{
    // ------------------------------------------------------------
    // 1. 检查 LightType
    // ------------------------------------------------------------
    if (!IsValidLightType(type))
    {
        throw std::invalid_argument("Light::DirectionFrom: 非法的光源类型");
    }

    // ------------------------------------------------------------
    // 2. 检查表面位置
    // ------------------------------------------------------------
    if (!IsFiniteVec3(surfacePosition))
    {
        throw std::invalid_argument("Light::DirectionFrom: surfacePosition 包含非有限值");
    }

    // ------------------------------------------------------------
    // 3. 根据光源类型计算：
    //
    // Point:
    //     delta = light.position - surfacePosition
    //
    // Directional:
    //     direction 表示“光线传播方向”
    //     所以表面指向光源的方向是：
    //
    //     L = -direction
    // ------------------------------------------------------------
    Vec3 delta;

    switch (type)
    {
    case LightType::Point:
    {
        if (!IsFiniteVec3(position))
        {
            throw std::invalid_argument("Light::DirectionFrom: 点光源 position 包含非有限值");
        }

        delta = {
            position.x - surfacePosition.x,
            position.y - surfacePosition.y,
            position.z - surfacePosition.z
        };

        break;
    }

    case LightType::Directional:
    {
        if (!IsFiniteVec3(direction))
        {
            throw std::invalid_argument("Light::DirectionFrom: 方向光 direction 包含非有限值");
        }

        delta = {
            -direction.x,
            -direction.y,
            -direction.z
        };

        break;
    }

    default:
        throw std::invalid_argument("Light::DirectionFrom: 非法的光源类型");
    }

    // ------------------------------------------------------------
    // 4. 零方向特殊处理
    //
    // Point Light 如果刚好位于 surfacePosition：
    //
    //     delta = (0,0,0)
    //
    // 这时没有明确的“从表面指向光源”的方向。
    //
    // 直接返回零向量，避免除以 0。
    // ------------------------------------------------------------
    const float lengthSquared =
        delta.x * delta.x +
        delta.y * delta.y +
        delta.z * delta.z;

    if (!std::isfinite(lengthSquared))
    {
        throw std::invalid_argument("Light::DirectionFrom: 光源方向长度不是有限值");
    }

    if (lengthSquared <= 0.0f)
    {
        return Vec3{ 0, 0, 0 };
    }

    // ------------------------------------------------------------
    // 5. 归一化
    //
    // 最终 L 必须是单位向量。
    // ------------------------------------------------------------
    const float length = std::sqrt(lengthSquared);

    if (!std::isfinite(length) || length <= 0.0f)
    {
        throw std::invalid_argument("Light::DirectionFrom: 无法计算有效的光源方向");
    }

    const Vec3 result{
        delta.x / length,
        delta.y / length,
        delta.z / length
    };

    if (!IsFiniteVec3(result))
    {
        throw std::invalid_argument("Light::DirectionFrom: 归一化后的方向包含非有限值");
    }

    return result;
}

Vec3 Light::RadianceAt(const Vec3& surfacePosition) const
{
    // ------------------------------------------------------------
    // 1. 检查光源类型
    // ------------------------------------------------------------
    if (!IsValidLightType(type))
    {
        throw std::invalid_argument("Light::RadianceAt: 非法的光源类型");
    }

    // ------------------------------------------------------------
    // 2. 检查 surfacePosition
    // ------------------------------------------------------------
    if (!IsFiniteVec3(surfacePosition))
    {
        throw std::invalid_argument("Light::RadianceAt: surfacePosition 包含非有限值");
    }

    // ------------------------------------------------------------
    // 3. 检查颜色
    //
    // color 是线性 RGB：
    //
    //     0 <= R,G,B <= 1
    //
    // ------------------------------------------------------------
    if (!IsFiniteVec3(color))
    {
        throw std::invalid_argument("Light::RadianceAt: color 包含非有限值");
    }

    if (color.x < 0.0f || color.x > 1.0f ||
        color.y < 0.0f || color.y > 1.0f ||
        color.z < 0.0f || color.z > 1.0f)
    {
        throw std::invalid_argument("Light::RadianceAt: color 必须位于 [0,1]");
    }

    // ------------------------------------------------------------
    // 4. 检查 intensity
    // ------------------------------------------------------------
    if (!std::isfinite(intensity) || intensity < 0.0f)
    {
        throw std::invalid_argument("Light::RadianceAt: intensity 必须是有限非负数");
    }

    // ------------------------------------------------------------
    // 5. 检查衰减参数
    // ------------------------------------------------------------
    if (!std::isfinite(constantAttenuation) ||
        !std::isfinite(linearAttenuation) ||
        !std::isfinite(quadraticAttenuation))
    {
        throw std::invalid_argument("Light::RadianceAt: 衰减参数包含非有限值");
    }

    if (constantAttenuation < 0.0f ||
        linearAttenuation < 0.0f ||
        quadraticAttenuation < 0.0f)
    {
        throw std::invalid_argument("Light::RadianceAt: 衰减参数必须是非负数");
    }

    // ------------------------------------------------------------
    // 6. 计算 attenuation
    //
    // Directional Light：
    //
    //     attenuation = 1
    //
    // Point Light：
    //
    //     d = |position - surfacePosition|
    //
    //     denominator =
    //         kc + kl*d + kq*d*d
    //
    //     attenuation = 1 / denominator
    // ------------------------------------------------------------
    float attenuation = 1.0f;

    switch (type)
    {
    case LightType::Directional:
    {
        // 方向光距离视为无限远，但我们不进行距离衰减。
        attenuation = 1.0f;
        break;
    }

    case LightType::Point:
    {
        if (!IsFiniteVec3(position))
        {
            throw std::invalid_argument("Light::RadianceAt: 点光源 position 包含非有限值");
        }

        const float dx = position.x - surfacePosition.x;
        const float dy = position.y - surfacePosition.y;
        const float dz = position.z - surfacePosition.z;

        // 使用 hypot，避免普通 dx*dx + dy*dy + dz*dz
        // 在极端数值下发生不必要的中间溢出。
        const float d = std::hypot(std::hypot(dx, dy), dz);

        if (!std::isfinite(d))
        {
            throw std::invalid_argument("Light::RadianceAt: 点光源距离不是有限值");
        }

        const float denominator =
            constantAttenuation +
            linearAttenuation * d +
            quadraticAttenuation * d * d;

        if (!std::isfinite(denominator) || denominator <= 0.0f)
        {
            throw std::invalid_argument(
                "Light::RadianceAt: 点光源衰减分母必须是有限正数");
        }

        attenuation = 1.0f / denominator;

        if (!std::isfinite(attenuation))
        {
            throw std::invalid_argument(
                "Light::RadianceAt: 衰减结果不是有限值");
        }

        break;
    }

    default:
        throw std::invalid_argument("Light::RadianceAt: 非法的光源类型");
    }

    // ------------------------------------------------------------
    // 7. 计算光源在当前表面点提供的光强
    //
    //     I = color * intensity * attenuation
    //
    // 注意：
    // 这里的 * 是“标量乘法”和逐通道缩放。
    //
    // 不在这里计算：
    //
    //     N.Dot(L)
    //
    // 也不计算：
    //
    //     BaseColor
    //
    // 这些属于 Material。
    // ------------------------------------------------------------
    const Vec3 result{
        color.x * intensity * attenuation,
        color.y * intensity * attenuation,
        color.z * intensity * attenuation
    };

    if (!IsFiniteVec3(result))
    {
        throw std::invalid_argument(
            "Light::RadianceAt: 光源强度结果包含非有限值");
    }

    return result;
}
/*
虽然上面代码很多，但图形学核心只有这几行。
Point Light 衰减
delta = position - surfacePosition;
L = normalize(delta);
Directional Light
delta = -direction;
L = normalize(delta);

第一阶段：Lambert
先实现最基础的漫反射：
*/

/*
Blinn-Phong 是经典的经验光照模型。
它基本上是在 Lambert 的基础上：
Ambient
+
Diffuse
+
Specular
*/