#include "application/Renderer.h"
#include "geometry/OBJLoader.h"
#include "renderer/Rasterizer.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <limits>
#include <stdexcept>
#include "assets/AssetPath.h"
#include <filesystem>
#include <cwctype>
#include <memory>
#include <chrono>

namespace
{
// 仅缓存当前模型，切换路径时替换，避免每帧解析与无限增长的资源表。
std::string cachedPath;
ModelAsset cachedAsset;
std::string cachedMaterialKey;
Material cachedMaterial;
Mat4 cachedFit;
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
    RenderScene(SceneSettings{});
}
void RenderScene(const std::string &modelPath)
{
    SceneSettings settings;
    settings.modelPath = modelPath;
    settings.basic = true;
    RenderScene(settings);
}
std::string ModelPathFromName(const std::string &name)
{
    if (name == "cube") return "assets/models/cube.obj";
    if (name == "triangle") return "assets/models/triangle.obj";
    if (name == "icosahedron") return "assets/models/low_poly_icosahedron.obj";
    if (name == "bunny") return "assets/models/stanfordbunny.obj";
    if (name == "room") return "assets/models/viking_room/viking_room.obj";
    return name;
}
void RenderScene(const SceneSettings &settings)
{
    const auto modelPath = ModelPathFromName(settings.modelPath);
    if (cachedPath != modelPath || cachedAsset.mesh.indices.empty())
    {
        auto loaded = OBJLoader::LoadAsset(modelPath);
        const auto fit = AutoFit(loaded.mesh);
        std::printf("资源加载：%s；%.3f 毫秒；%zu 个材质；%zu 张贴图；UV=%s\n", modelPath.c_str(),
                    loaded.loadMilliseconds, loaded.materials.size(), loaded.textureCount,
                    loaded.generatedUV ? "缺失部分使用自动盒式投影" : "模型原配");
        cachedAsset = std::move(loaded);
        cachedFit = fit;
        cachedPath = modelPath;
    }
    const bool overrideMaterial = settings.material != "auto" || !settings.texture.empty();
    const auto materialKey = settings.material + '\n' + settings.texture;
    if (!settings.basic && overrideMaterial && cachedMaterialKey != materialKey)
    {
        const auto materialStart = std::chrono::steady_clock::now();
        auto path = settings.material;
        if (path == "wood" || path == "brick" || path == "stone" || path == "gold" || path == "plain" || path == "auto")
            path = "assets/materials/" + (path == "auto" ? std::string("plain") : path) + ".mtl";
        auto material = OBJLoader::LoadMaterial(path, settings.texture);
        cachedMaterial = std::move(material);
        cachedMaterialKey = materialKey;
        std::printf("替换材质/贴图加载：%s；%.3f 毫秒\n", path.c_str(),
            std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - materialStart).count());
    }
    for (auto &material : cachedAsset.materials)
        material.model = settings.lighting;
    cachedMaterial.model = settings.lighting;
    Raster::Clear(Raster::MakeColor(18, 22, 30));
    Raster::ClearDepth();
    // 手动实验位置：模型旋转、相机 eye/target、FOV 与 Near/Far。
    Camera camera;
    camera.eye = {0, 0, 3.4f};
    camera.aspect = float(Raster::WIDTH) / Raster::HEIGHT;
    // Viking Room 原资源 Z 向上，单独设置演示朝向；不修改 OBJ 或项目坐标约定。
    const bool room = modelPath.find("viking_room") != std::string::npos;
    if (room)
        camera.eye = {2.6f, 2.f, -2.6f};
    const Vec3 rotation = room ? Vec3{-1.57079633f, 0, 0} : Vec3{.5f, .65f, 0};
    const auto model = Transform::Model({0, 0, 0}, rotation, {1, 1, 1}) * cachedFit;
    Light key;
    key.type = LightType::Directional;
    key.direction = room ? Vec3{-1, -1, 1} : Vec3{-1.f, .7f, .1f};
    key.intensity = .85f;
    Light fill;
    fill.type = LightType::Directional;
    fill.direction = {1, -1.f, -.3f};
    fill.intensity = .35f;
    const std::vector<Light> lights{key, fill};
    TriangleRenderOptions options;
    options.shading = settings.shading;
    const auto stats = settings.basic ? DrawMeshBasic(cachedAsset.mesh, model, camera)
        : DrawModelShaded(cachedAsset, model, camera, lights, options, overrideMaterial ? &cachedMaterial : nullptr);
    std::printf("模型：%s；%d×%d；提交 %zu 面，栅格 %zu 面，拒绝 %zu 面；%.3f 毫秒\n", modelPath.c_str(), Raster::WIDTH,
                Raster::HEIGHT, stats.inputTriangles, stats.rasterizedTriangles, stats.rejectedTriangles,
                stats.milliseconds);
}
bool SaveRenderedImage(const std::string &utf8Path)
{
    const auto path = std::filesystem::u8path(utf8Path);
    if (!path.parent_path().empty())
        std::filesystem::create_directories(path.parent_path());
    auto extension = path.extension().wstring();
    std::transform(extension.begin(), extension.end(), extension.begin(), [](wchar_t c) { return std::towlower(c); });
    if (extension == L".png") return Raster::SavePNG(utf8Path.c_str());
    if (extension == L".ppm") return Raster::SavePPM(utf8Path.c_str());
    throw std::invalid_argument("图片输出扩展名仅支持 .png 和 .ppm");
}
