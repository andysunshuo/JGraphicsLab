#include "Camera/Camera.h"
#include <cmath>
#include <stdexcept>
Mat4 Camera::View() const
{
    const auto finite = [](const Vec3 &v) {
        return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z);
    };
    if (!finite(eye) || !finite(target) || !finite(up))
        throw std::invalid_argument("相机位置、目标或上方向包含无效坐标");
    return LookAt(eye, target, up);
}
Mat4 Camera::Projection() const
{
    if (!std::isfinite(fovRadians) || !std::isfinite(aspect) || !std::isfinite(nearPlane) ||
        !std::isfinite(farPlane))
        throw std::invalid_argument("相机投影参数包含无效数值");
    return MakePerspective(fovRadians, aspect, nearPlane, farPlane);
}
