#include "application/Renderer.h"
#include "geometry/OBJLoader.h"
#include "renderer/Rasterizer.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <limits>
#include <stdexcept>

namespace
{
// 仅缓存当前模型，切换路径时替换，避免每帧解析与无限增长的资源表。
std::string cachedPath;
Mesh cachedMesh;
Mat4 AutoFit(const Mesh &mesh)
{
    Vec3 low{std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(),
             std::numeric_limits<float>::infinity()};
    Vec3 high = -low;
    for (const auto &v : mesh.vertices)
    {
        if (!v.position.IsFinite())
            throw std::runtime_error("模型位置包含非有限值");
        low = {std::min(low.x, v.position.x), std::min(low.y, v.position.y), std::min(low.z, v.position.z)};
        high = {std::max(high.x, v.position.x), std::max(high.y, v.position.y), std::max(high.z, v.position.z)};
    }
    const auto size = high - low;
    const float extent = std::max({size.x, size.y, size.z});
    if (!std::isfinite(extent) || extent < 1e-8f)
        throw std::runtime_error("模型包围盒为空或退化");
    const auto center = low + size * .5f;
    const float scale = 1.8f / extent;
    return Transform::Scale({scale, scale, scale}) * Transform::Translation(-center);
}
} // namespace
void RenderScene()
{
    RenderScene(kDefaultModelPath);
}
void RenderScene(const std::string &modelPath)
{
    Raster::Clear(Raster::MakeColor(18, 22, 30));
    Raster::ClearDepth();
    if (cachedPath != modelPath || cachedMesh.indices.empty())
    {
        auto loaded = OBJLoader::Load(modelPath);
        if (loaded.vertices.empty() || loaded.indices.empty())
            throw std::runtime_error("OBJ 加载失败：" + modelPath);
        cachedMesh = std::move(loaded);
        cachedPath = modelPath;
    }
    // 手动实验位置：模型旋转、相机 eye/target、FOV 与 Near/Far。
    Camera camera;
    camera.eye = {0, 0, 3.4f};
    camera.aspect = float(Raster::WIDTH) / Raster::HEIGHT;
    const auto model = Transform::Model({0, 0, 0}, {.5f, .65f, 0}, {1, 1, 1}) * AutoFit(cachedMesh);
    const auto stats = DrawMeshBasic(cachedMesh, model, camera);
    std::printf("模型：%s；%d×%d；提交 %zu 面，栅格 %zu 面，拒绝 %zu 面；%.3f 毫秒\n", modelPath.c_str(), Raster::WIDTH,
                Raster::HEIGHT, stats.inputTriangles, stats.rasterizedTriangles, stats.rejectedTriangles,
                stats.milliseconds);
}
