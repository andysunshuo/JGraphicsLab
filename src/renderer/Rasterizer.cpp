#include "renderer/Rasterizer.h"
#include <chrono>
#include <cmath>
#include <stdexcept>

RenderStats DrawMeshBasic(const Mesh &mesh, const Mat4 &model, const Camera &camera, bool faceColors,
                          Raster::Color solidColor)
{
    const auto start = std::chrono::steady_clock::now();
    RenderStats stats;
    if (mesh.indices.size() % 3)
        throw std::invalid_argument("Mesh 索引数量不是三的倍数");
    if (!model.IsFinite())
        throw std::invalid_argument("Model 矩阵包含非有限值");
    const auto mvp = camera.ViewProjectionMatrix() * model;
    if (!mvp.IsFinite())
        throw std::invalid_argument("MVP 矩阵包含非有限值");
    
    //固定颜色
    //  这里的计算过程是：
    //-三角形 0、1：使用第一种颜色。
     //    - 三角形 2、3：使用第二种颜色。
     //   - 每两个三角形换一种颜色，六种颜色循环使用。
     
   
    constexpr Raster::Color palette[] = {0x005698d6, 0x00eb9b4a, 0x004db8a9, 0x00a68bd4, 0x00e3cc65, 0x00df767f};

    stats.inputTriangles = mesh.indices.size() / 3;
    for (std::size_t face = 0; face < stats.inputTriangles; ++face)
    {
        ClipVertex vertices[3];
        bool valid = true;
        const auto color = Raster::DecodeSrgbColor(faceColors ? palette[(face / 2) % 6] : solidColor);
        for (int corner = 0; corner < 3; ++corner)
        {
            const auto index = mesh.indices[3 * face + corner];
            if (index >= mesh.vertices.size())
                throw std::invalid_argument("Mesh 顶点索引越界");
            const auto &vertex = mesh.vertices[index];
            const auto world = model * Vec4::Position(vertex.position);
            vertices[corner] = {
                mvp * Vec4::Position(vertex.position), {world.x, world.y, world.z}, vertex.normal, vertex.uv, color};
            valid &= vertices[corner].position.IsFinite() && vertex.position.IsFinite() && vertex.normal.IsFinite() &&
                     vertex.uv.IsFinite();
        }
        if (!valid)
        {
            ++stats.rejectedTriangles;
            continue;
        }
        const auto polygon = ClipTriangleToFrustum(vertices[0], vertices[1], vertices[2]);
        if (polygon.size() < 3)
        {
            ++stats.rejectedTriangles;
            continue;
        }
        for (std::size_t i = 1; i + 1 < polygon.size(); ++i)
        {
            ScreenVertex a, b, c;
            if (Transform::ToScreen(polygon[0], Raster::WIDTH, Raster::HEIGHT, a) &&
                Transform::ToScreen(polygon[i], Raster::WIDTH, Raster::HEIGHT, b) &&
                Transform::ToScreen(polygon[i + 1], Raster::WIDTH, Raster::HEIGHT, c))
            {
                RasterizeTriangle(a, b, c);
                ++stats.rasterizedTriangles;
            }
        }
    }
    stats.milliseconds = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
    return stats;
}
