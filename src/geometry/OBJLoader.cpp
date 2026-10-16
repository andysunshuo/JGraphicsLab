#include "geometry/OBJLoader.h"
#include "assets/AssetPath.h"
#define TINYOBJLOADER_IMPLEMENTATION
// 当前 MSVC 与缓存版本的 fast_float constexpr 不兼容，使用库提供的标准解析路径。
#define TINYOBJLOADER_DISABLE_FAST_FLOAT
#include "tiny_obj_loader.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <tuple>

namespace
{
constexpr std::size_t kMaxVertices = 1500000, kMaxTriangles = 500000;
using Path = std::filesystem::path;

// 语法仍由 tinyobjloader 解析；这里只提供有预算、支持 UTF-8 路径的 MTL 文件读取。
class MaterialFiles : public tinyobj::MaterialReader
{
  public:
    explicit MaterialFiles(Path root) : root_(std::move(root)) {}
    bool operator()(const std::string &name, std::vector<tinyobj::material_t> *materials,
                    std::map<std::string, int> *map, std::string *warning, std::string *error) override
    {
        if (++fileCount_ > 16)
            throw std::runtime_error("模型引用的 MTL 超过 16 份预算");
        const auto path = (root_ / std::filesystem::u8path(name)).lexically_normal();
        const auto text = ReadAssetText(path, 4 * 1024 * 1024);
        // 在交给解析器分配材质对象前先检查数量预算；不重复解析 MTL 语法。
        std::istringstream counter(text);
        std::string line;
        auto count = materials->size();
        while (std::getline(counter, line))
        {
            std::istringstream words(line);
            std::string tag;
            words >> tag;
            if (tag == "newmtl" && ++count > 256)
                throw std::runtime_error("模型材质数量超过 256 份预算");
        }
        std::istringstream stream(text);
        const auto before = materials->size();
        tinyobj::LoadMtl(map, materials, &stream, warning, error);
        if (materials->size() > 256)
            throw std::runtime_error("模型材质数量超过 256 份预算");
        directories.resize(materials->size(), path.parent_path());
        for (auto i = before; i < directories.size(); ++i)
            directories[i] = path.parent_path();
        return error->empty();
    }
    std::vector<Path> directories;
  private:
    Path root_;
    int fileCount_ = 0;
};
struct TextureCache
{
    std::map<std::pair<Path, TextureEncoding>, std::shared_ptr<const Texture>> textures;
    std::size_t bytes = 0;
    std::shared_ptr<const Texture> Load(const Path &path, TextureEncoding encoding)
    {
        const auto key = std::make_pair(std::filesystem::absolute(path).lexically_normal(), encoding);
        if (const auto found = textures.find(key); found != textures.end())
            return found->second;
        if (textures.size() >= 16)
            throw std::runtime_error("一个资源的贴图数量超过 16 张预算");
        auto texture = std::make_shared<Texture>();
        std::string error;
        if (!texture->Load(AssetPathUtf8(key.first), encoding, &error))
            throw std::runtime_error(error);
        const auto cost = static_cast<std::size_t>(texture->Width()) * texture->Height() * sizeof(Vec3);
        if (bytes + cost > 128 * 1024 * 1024)
            throw std::runtime_error("一个资源的线性贴图数据超过 128 MiB 预算");
        bytes += cost;
        textures.emplace(key, texture);
        return texture;
    }
};
Vec3 Color(const tinyobj::real_t value[3])
{
    const Vec3 result{value[0], value[1], value[2]};
    if (!result.IsFinite() || result.x < 0 || result.x > 1 || result.y < 0 || result.y > 1 || result.z < 0 || result.z > 1)
        throw std::runtime_error("MTL 颜色系数必须是有限的 0～1 分量");
    return result;
}
Material ConvertMaterial(const tinyobj::material_t &source, const Path &folder, TextureCache &cache)
{
    Material material;
    material.baseColor = Color(source.diffuse);
    material.diffuse = {1, 1, 1}; // Kd 已放入基色，不再平方 Kd。
    material.ambient = Color(source.ambient);
    material.specular = Color(source.specular);
    if (!std::isfinite(source.shininess) || source.shininess < 0)
        throw std::runtime_error("MTL 高光指数必须有限且非负");
    material.shininess = std::max(1.f, source.shininess);
    material.uvScale = {source.diffuse_texopt.scale[0], source.diffuse_texopt.scale[1]};
    material.uvOffset = {source.diffuse_texopt.origin_offset[0], source.diffuse_texopt.origin_offset[1]};
    material.wrap = source.diffuse_texopt.clamp ? TextureWrap::Clamp : TextureWrap::Repeat;
    if (!material.uvScale.IsFinite() || !material.uvOffset.IsFinite())
        throw std::runtime_error("MTL 纹理变换参数不是有限值");
    if (!source.diffuse_texname.empty())
        material.albedoTexture = cache.Load(folder / std::filesystem::u8path(source.diffuse_texname), TextureEncoding::SrgbColor);
    if (!source.specular_texname.empty())
        material.specularTexture = cache.Load(folder / std::filesystem::u8path(source.specular_texname), TextureEncoding::LinearData);
    return material;
}
// 缺失 UV 的面用包围盒主轴投影；拆分角点保留接缝，不修改源 OBJ。
void GenerateBoxUV(Mesh &mesh, const std::vector<bool> &hasUV)
{
    Vec3 low = mesh.vertices.front().position, high = low;
    for (const auto &v : mesh.vertices)
    {
        low = {std::min(low.x, v.position.x), std::min(low.y, v.position.y), std::min(low.z, v.position.z)};
        high = {std::max(high.x, v.position.x), std::max(high.y, v.position.y), std::max(high.z, v.position.z)};
    }
    const auto size = high - low;
    const float extent = std::max({size.x, size.y, size.z});
    if (!std::isfinite(extent) || extent <= 1e-8f)
        throw std::runtime_error("无法给退化包围盒生成 UV");
    for (std::size_t face = 0; face < mesh.indices.size(); face += 3)
    {
        if (hasUV[mesh.indices[face]] && hasUV[mesh.indices[face + 1]] && hasUV[mesh.indices[face + 2]])
            continue;
        const auto a = mesh.vertices[mesh.indices[face]].position;
        const auto b = mesh.vertices[mesh.indices[face + 1]].position;
        const auto c = mesh.vertices[mesh.indices[face + 2]].position;
        const auto normal = (b - a).Cross(c - a);
        for (int corner = 0; corner < 3; ++corner)
        {
            auto vertex = mesh.vertices[mesh.indices[face + corner]];
            const auto p = (vertex.position - low) / extent;
            if (std::abs(normal.x) >= std::max(std::abs(normal.y), std::abs(normal.z)))
                vertex.uv = {p.z, p.y};
            else if (std::abs(normal.y) >= std::abs(normal.z))
                vertex.uv = {p.x, p.z};
            else
                vertex.uv = {p.x, p.y};
            if (mesh.vertices.size() >= kMaxVertices)
                throw std::runtime_error("生成 UV 后顶点数超过预算");
            mesh.indices[face + corner] = static_cast<unsigned int>(mesh.vertices.size());
            mesh.vertices.push_back(vertex);
        }
    }
}
} // namespace

ModelAsset OBJLoader::LoadAsset(const std::string &filepath, bool generateMissingUV)
{
    const auto start = std::chrono::steady_clock::now();
    const auto path = ResolveAssetPath(filepath);
    std::istringstream stream(ReadAssetText(path, 64 * 1024 * 1024));
    tinyobj::attrib_t attributes;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> sourceMaterials;
    std::string warning, error;
    MaterialFiles reader(path.parent_path());
    if (!tinyobj::LoadObj(&attributes, &shapes, &sourceMaterials, &warning, &error, &stream, &reader, true))
        throw std::runtime_error("OBJ 解析失败（" + filepath + "）：" + error);
    if (!warning.empty())
        std::cerr << "OBJ 加载提示（" << filepath << "）：" << warning << '\n';
    if (attributes.vertices.size() / 3 > 500000 || attributes.normals.size() / 3 > 1000000 ||
        attributes.texcoords.size() / 2 > 1000000)
        throw std::runtime_error("模型属性数量超过预算");
    ModelAsset result;
    auto &mesh = result.mesh;
    TextureCache cache;
    for (std::size_t i = 0; i < sourceMaterials.size(); ++i)
    {
        result.materials.push_back(ConvertMaterial(sourceMaterials[i], reader.directories.at(i), cache));
        result.materialNames.push_back(sourceMaterials[i].name);
    }
    const auto defaultMaterial = static_cast<unsigned int>(result.materials.size());
    Material fallback;
    fallback.baseColor = {.65f, .65f, .65f};
    fallback.ambient = {.15f, .15f, .15f};
    result.materials.push_back(fallback);
    result.materialNames.push_back("默认灰色");
    std::map<std::tuple<int, int, int>, unsigned int> unique;
    std::vector<unsigned int> sourcePositions;
    std::vector<bool> hasUV, hasNormal;
    std::vector<Vec3> normalSums(attributes.vertices.size() / 3);
    for (const auto &shape : shapes)
    {
        if (shape.mesh.indices.size() % 3)
            throw std::runtime_error("三角化后的索引数量无效");
        for (std::size_t face = 0; face < shape.mesh.indices.size() / 3; ++face)
        {
            if (result.triangleMaterials.size() >= kMaxTriangles)
                throw std::runtime_error("模型三角形超过 50 万预算");
            const int material = shape.mesh.material_ids.at(face);
            if (material >= static_cast<int>(sourceMaterials.size()) || material < -1)
                throw std::runtime_error("OBJ 材质编号越界");
            result.triangleMaterials.push_back(material < 0 ? defaultMaterial : static_cast<unsigned int>(material));
            for (int corner = 0; corner < 3; ++corner)
            {
                const auto &index = shape.mesh.indices[face * 3 + corner];
                if (index.vertex_index < 0 || static_cast<std::size_t>(index.vertex_index) >= attributes.vertices.size() / 3 ||
                    index.texcoord_index < -1 || (index.texcoord_index >= 0 && static_cast<std::size_t>(index.texcoord_index) >= attributes.texcoords.size() / 2) ||
                    index.normal_index < -1 || (index.normal_index >= 0 && static_cast<std::size_t>(index.normal_index) >= attributes.normals.size() / 3))
                    throw std::runtime_error("模型包含非法位置、UV 或法线索引");
                const auto key = std::make_tuple(index.vertex_index, index.texcoord_index, index.normal_index);
                auto found = unique.find(key);
                if (found == unique.end())
                {
                    if (mesh.vertices.size() >= kMaxVertices)
                        throw std::runtime_error("统一顶点超过 150 万预算");
                    Vertex vertex{};
                    const auto p = static_cast<std::size_t>(index.vertex_index) * 3;
                    vertex.position = {attributes.vertices[p], attributes.vertices[p + 1], attributes.vertices[p + 2]};
                    if (index.texcoord_index >= 0)
                    {
                        const auto uv = static_cast<std::size_t>(index.texcoord_index) * 2;
                        vertex.uv = {attributes.texcoords[uv], attributes.texcoords[uv + 1]};
                    }
                    if (index.normal_index >= 0)
                    {
                        const auto n = static_cast<std::size_t>(index.normal_index) * 3;
                        vertex.normal = {attributes.normals[n], attributes.normals[n + 1], attributes.normals[n + 2]};
                    }
                    if (!vertex.position.IsFinite() || !vertex.normal.IsFinite() || !vertex.uv.IsFinite())
                        throw std::runtime_error("模型包含非有限坐标或属性");
                    const auto id = static_cast<unsigned int>(mesh.vertices.size());
                    mesh.vertices.push_back(vertex);
                    sourcePositions.push_back(index.vertex_index);
                    hasUV.push_back(index.texcoord_index >= 0);
                    hasNormal.push_back(index.normal_index >= 0 && vertex.normal.LengthSquared() > 1e-16f);
                    found = unique.emplace(key, id).first;
                }
                mesh.indices.push_back(found->second);
            }
            const auto end = mesh.indices.size();
            const auto a = mesh.indices[end - 3], b = mesh.indices[end - 2], c = mesh.indices[end - 1];
            const auto normal = (mesh.vertices[b].position - mesh.vertices[a].position).Cross(mesh.vertices[c].position - mesh.vertices[a].position);
            if (!normal.IsFinite())
                throw std::runtime_error("模型面法线计算溢出");
            for (auto id : {a, b, c})
                normalSums[sourcePositions[id]] += normal;
        }
    }
    if (mesh.indices.empty())
        throw std::runtime_error("模型不包含可绘制的三角面：" + filepath);
    // 仅生成缺失法线；保留 OBJ 原有法线和硬边，不朝相机翻转。
    for (std::size_t i = 0; i < mesh.vertices.size(); ++i)
        if (!hasNormal[i])
            mesh.vertices[i].normal = normalSums[sourcePositions[i]].Normalized();
    if (generateMissingUV && std::find(hasUV.begin(), hasUV.end(), false) != hasUV.end())
    {
        GenerateBoxUV(mesh, hasUV);
        result.generatedUV = true;
    }
    result.textureCount = cache.textures.size();
    result.loadMilliseconds = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
    return result;
}
Mesh OBJLoader::Load(const std::string &filepath)
{
    try { return LoadAsset(filepath, false).mesh; }
    catch (const std::exception &error)
    {
        std::cerr << "OBJ 加载失败（" << filepath << "）：" << error.what() << '\n';
        return {};
    }
}
Material OBJLoader::LoadMaterial(const std::string &mtlPath, const std::string &textureOverride)
{
    const auto path = ResolveAssetPath(mtlPath);
    MaterialFiles reader(path.parent_path());
    std::vector<tinyobj::material_t> materials;
    std::map<std::string, int> map;
    std::string warning, error;
    if (!reader(AssetPathUtf8(path.filename()), &materials, &map, &warning, &error) || materials.empty())
        throw std::runtime_error("MTL 读取失败或没有材质：" + mtlPath + "；" + error);
    if (!warning.empty())
        std::cerr << "材质提示：" << warning << '\n';
    TextureCache cache;
    auto material = ConvertMaterial(materials.front(), path.parent_path(), cache);
    if (!textureOverride.empty())
        material.albedoTexture = cache.Load(ResolveAssetPath(textureOverride), TextureEncoding::SrgbColor);
    return material;
}
