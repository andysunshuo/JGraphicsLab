#include "Renderer/MeshRenderer.h"
#include "Renderer/Clipping.h"
#include "Renderer/Rasterizer3D.h"
#include "Renderer/DepthBuffer.h"
#include <chrono>
#include <cmath>

RenderStats DrawMesh(const Mesh &mesh, const Mat4 &modelMatrix, const Camera &camera,
                     const RenderOptions &options)
{
    const auto start = std::chrono::steady_clock::now();
    RenderStats stats;
    stats.inputTriangles = mesh.indices.size() / 3;
    if (options.clearDepth)
        ClearDepthBuffer();
    const Mat4 mvp = Multiply(camera.Projection(), Multiply(camera.View(), modelMatrix));
    for (std::size_t tri = 0; tri < stats.inputTriangles; ++tri)
    {
        const Index i0 = mesh.indices[3 * tri], i1 = mesh.indices[3 * tri + 1],
                    i2 = mesh.indices[3 * tri + 2];
        if (i0 >= mesh.vertices.size() || i1 >= mesh.vertices.size() || i2 >= mesh.vertices.size())
        {
            ++stats.rejectedTriangles;
            continue;
        }
        const Vec3 p0 = mesh.vertices[i0].position, p1 = mesh.vertices[i1].position,
                   p2 = mesh.vertices[i2].position;
        if (!IsFinite(p0) || !IsFinite(p1) || !IsFinite(p2))
        {
            ++stats.rejectedTriangles;
            continue;
        }
        const Vec3 cross = Cross(Subtract(p1, p0), Subtract(p2, p0));
        const float areaSquared = Dot(cross, cross);
        if (!IsFinite(cross) || !std::isfinite(areaSquared) || areaSquared <= 0)
        {
            ++stats.rejectedTriangles;
            continue;
        }
        Color3 color = options.solidColor;
        if (options.colorMode == MeshColorMode::FaceColor && tri < mesh.triangleColors.size())
            color = mesh.triangleColors[tri];
        if (options.colorMode == MeshColorMode::FaceNormal)
        {
            // 显示模型空间面法线；此阶段不冒充 HW44 法线矩阵或光照实现。
            const Vec3 n = Normalize(cross);
            color = {(n.x + 1) * 127.5f, (n.y + 1) * 127.5f, (n.z + 1) * 127.5f};
        }
        if (!std::isfinite(color.r) || !std::isfinite(color.g) || !std::isfinite(color.b))
        {
            ++stats.rejectedTriangles;
            continue;
        }
        const ClipVertex v[3] = {{Multiply(mvp, MakePosition4(p0)), color},
                                 {Multiply(mvp, MakePosition4(p1)), color},
                                 {Multiply(mvp, MakePosition4(p2)), color}};
        bool valid = true;
        for (const ClipVertex &cv : v)
        {
            const Vec4 p = cv.position;
            valid &= std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z) && std::isfinite(p.w);
        }
        if (!valid)
        {
            ++stats.rejectedTriangles;
            continue;
        }
        // 裁剪临时内存只跟单个三角形相交的六平面有关，不跟模型规模增长。
        const auto polygon = ClipTriangleToFrustum(v[0], v[1], v[2]);
        if (polygon.size() < 3)
        {
            ++stats.rejectedTriangles;
            continue;
        }
        for (std::size_t j = 1; j + 1 < polygon.size(); ++j)
        {
            ScreenVertex3D a{}, b{}, c{};
            if (!MakeScreenVertex3D(polygon[0], a) || !MakeScreenVertex3D(polygon[j], b) ||
                !MakeScreenVertex3D(polygon[j + 1], c))
                continue;
            const auto validScreen = [](const ScreenVertex3D &s) {
                return std::isfinite(s.position.x) && std::isfinite(s.position.y) && std::isfinite(s.ndcZ) &&
                       s.position.x >= -1 && s.position.x <= Raster::WIDTH + 1 && s.position.y >= -1 &&
                       s.position.y <= Raster::HEIGHT + 1;
            };
            // 裁剪数值异常不得进入原光栅化函数的整数包围盒转换。
            if (!validScreen(a) || !validScreen(b) || !validScreen(c))
                continue;
            RasterizeTriangle3D(a, b, c);
            ++stats.rasterizedTriangles;
        }
    }
    stats.milliseconds =
        std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
    return stats;
}
