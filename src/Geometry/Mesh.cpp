#include "Geometry/Mesh.h"
#include "Camera/Transforms.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

bool IsFinite(const Vec3 &v)
{
    return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z);
}
bool GetBounds(const Mesh &mesh, Bounds &bounds)
{
    if (mesh.vertices.empty())
        return false;
    bounds.min = bounds.max = mesh.vertices.front().position;
    for (const Vertex &vertex : mesh.vertices)
    {
        const Vec3 p = vertex.position;
        if (!IsFinite(p))
            return false;
        bounds.min = {std::min(bounds.min.x, p.x), std::min(bounds.min.y, p.y), std::min(bounds.min.z, p.z)};
        bounds.max = {std::max(bounds.max.x, p.x), std::max(bounds.max.y, p.y), std::max(bounds.max.z, p.z)};
    }
    return true;
}
Mat4 MakeAutoFitModel(const Mesh &mesh, float targetExtent)
{
    Bounds bounds;
    if (!GetBounds(mesh, bounds) || !std::isfinite(targetExtent) || targetExtent <= 0)
        throw std::invalid_argument("模型包围盒或目标尺寸无效");
    const Vec3 extent = Subtract(bounds.max, bounds.min);
    const float longest = std::max({extent.x, extent.y, extent.z});
    if (!std::isfinite(longest) || longest <= 1e-7f)
        throw std::invalid_argument("模型包围盒退化或数值溢出");
    const Vec3 center = Add(Multiply(bounds.min, 0.5f), Multiply(bounds.max, 0.5f));
    const float scale = targetExtent / longest;
    return Multiply(MakeScale3D(scale, scale, scale), MakeTranslation3D(-center.x, -center.y, -center.z));
}
