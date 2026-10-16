#include "renderer/Rasterizer.h"
#include <chrono>
#include <cmath>
#include <stdexcept>
#include <algorithm>

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

namespace
{
template <class GetMaterial>
RenderStats DrawShaded(const Mesh &mesh, const Mat4 &model, const Camera &camera,
                       const std::vector<Light> &lights, const TriangleRenderOptions &options,
                       GetMaterial &&getMaterial)
{
    const auto start = std::chrono::steady_clock::now();
    if (mesh.indices.empty() || mesh.indices.size() % 3 || !model.IsFinite())
        throw std::invalid_argument("着色模型为空、索引数无效或模型矩阵非有限");
    if (camera.projectionType != ProjectionType::Perspective)
        throw std::invalid_argument("当前经典着色入口使用透视相机");
    if (model.m[3][0] != 0 || model.m[3][1] != 0 || model.m[3][2] != 0 || model.m[3][3] != 1)
        throw std::invalid_argument("模型矩阵必须是仿射变换");
    if (lights.size() > 16)
        throw std::invalid_argument("光源数量超过 16 盏预算");
    const auto mvp = camera.ViewProjectionMatrix() * model;
    // 消去线性部分的整体正尺度，法线归一化后的方向不变。
    // 防止毫米等大单位 OBJ 的 AutoFit 小缩放触发绝对行列式阈值，仍复用唯一逆转置实现。
    auto normalModel = model;
    float linearScale = 0;
    for (int row = 0; row < 3; ++row)
        for (int col = 0; col < 3; ++col)
            linearScale = std::max(linearScale, std::abs(model.m[row][col]));
    if (linearScale == 0)
        throw std::invalid_argument("模型线性变换退化");
    for (int row = 0; row < 3; ++row)
        for (int col = 0; col < 3; ++col)
            normalModel.m[row][col] /= linearScale;
    const auto normalMatrix = Transform::NormalMatrix(normalModel);
    if (!mvp.IsFinite() || !normalMatrix.IsFinite())
        throw std::invalid_argument("变换矩阵非有限");
    RenderStats stats;
    stats.inputTriangles = mesh.indices.size() / 3;
    for (std::size_t face = 0; face < stats.inputTriangles; ++face)
    {
        const auto &material = getMaterial(face);
        ClipVertex v[3];
        bool valid = true, missingNormal = false;
        for (int corner = 0; corner < 3; ++corner)
        {
            const auto index = mesh.indices[face * 3 + corner];
            if (index >= mesh.vertices.size())
                throw std::invalid_argument("着色模型顶点索引越界");
            const auto &source = mesh.vertices[index];
            const auto world = model * Vec4::Position(source.position);
            v[corner] = {mvp * Vec4::Position(source.position), {world.x, world.y, world.z},
                         normalMatrix * source.normal, source.uv, {1, 1, 1}};
            valid &= v[corner].position.IsFinite() && v[corner].worldPosition.IsFinite() &&
                     v[corner].normal.IsFinite() && source.uv.IsFinite();
            if (valid)
                v[corner].normal = v[corner].normal.Normalized();
            missingNormal |= v[corner].normal.LengthSquared() < 1e-16f;
        }
        if (!valid)
        {
            ++stats.rejectedTriangles;
            continue;
        }
        const auto cross = (v[1].worldPosition - v[0].worldPosition).Cross(v[2].worldPosition - v[0].worldPosition);
        if (!cross.IsFinite() || cross.LengthSquared() < 1e-20f)
        {
            ++stats.rejectedTriangles;
            continue;
        }
        const auto faceNormal = cross.Normalized();
        if (missingNormal)
            for (auto &corner : v)
                corner.normal = faceNormal;
        if (options.shading == ShadingMode::Flat)
        {
            const auto center = (v[0].worldPosition + v[1].worldPosition + v[2].worldPosition) / 3.f;
            const auto uv = (v[0].uv + v[1].uv + v[2].uv) / 3.f;
            const auto color = material.Evaluate(center, faceNormal, camera.eye, uv, lights);
            for (auto &corner : v)
                corner.color = color;
        }
        else if (options.shading == ShadingMode::Gouraud)
            for (auto &corner : v)
                corner.color = material.Evaluate(corner.worldPosition, corner.normal, camera.eye, corner.uv, lights);

        const auto polygon = ClipTriangleToFrustum(v[0], v[1], v[2]);
        bool submitted = false;
        for (std::size_t i = 1; i + 1 < polygon.size(); ++i)
        {
            ScreenVertex a, b, c;
            if (Transform::ToScreen(polygon[0], Raster::WIDTH, Raster::HEIGHT, a) &&
                Transform::ToScreen(polygon[i], Raster::WIDTH, Raster::HEIGHT, b) &&
                Transform::ToScreen(polygon[i + 1], Raster::WIDTH, Raster::HEIGHT, c))
            {
                RasterizeTriangleShaded(a, b, c, material, camera, lights, options);
                ++stats.rasterizedTriangles;
                submitted = true;
            }
        }
        if (!submitted)
            ++stats.rejectedTriangles;
    }
    stats.milliseconds = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
    return stats;
}
} // namespace
RenderStats DrawMeshShaded(const Mesh &mesh, const Mat4 &model, const Camera &camera, const Material &material,
                           const std::vector<Light> &lights, const TriangleRenderOptions &options)
{
    return DrawShaded(mesh, model, camera, lights, options, [&](std::size_t) -> const Material & { return material; });
}
RenderStats DrawModelShaded(const ModelAsset &asset, const Mat4 &model, const Camera &camera,
                           const std::vector<Light> &lights, const TriangleRenderOptions &options,
                           const Material *overrideMaterial)
{
    if (!overrideMaterial && asset.triangleMaterials.size() != asset.mesh.indices.size() / 3)
        throw std::invalid_argument("逐三角形材质表长度与模型不一致");
    return DrawShaded(asset.mesh, model, camera, lights, options,
        [&](std::size_t face) -> const Material & {
            if (overrideMaterial)
                return *overrideMaterial;
            const auto id = asset.triangleMaterials[face];
            if (id >= asset.materials.size())
                throw std::invalid_argument("三角形材质索引越界");
            return asset.materials[id];
        });
}
