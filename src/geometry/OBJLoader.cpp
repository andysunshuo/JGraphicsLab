#include "OBJLoader.h"
#define TINYOBJLOADER_IMPLEMENTATION
// 当前 MSVC 与缓存版本的 fast_float constexpr 不兼容，使用库提供的标准解析路径。
#define TINYOBJLOADER_DISABLE_FAST_FLOAT
#include "tiny_obj_loader.h"
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <stdexcept>
#include <tuple>
#include <windows.h>

namespace
{
constexpr std::uintmax_t kMaxFileBytes = 64 * 1024 * 1024;
constexpr std::size_t kMaxVertices = 1500000, kMaxTriangles = 500000;
std::filesystem::path ResolveModel(const std::string &name)
{
    const auto requested = std::filesystem::u8path(name);
    if (requested.is_absolute())
        return requested;
    // 首选 exe 旁资源，其次源工程目录，最后尝试调用者给出的相对路径。
    std::wstring executable(32768, L'\0');
    const DWORD length = GetModuleFileNameW(nullptr, executable.data(), static_cast<DWORD>(executable.size()));
    if (length && length < executable.size())
    {
        executable.resize(length);
        const auto candidate = std::filesystem::path(executable).parent_path() / requested;
        if (std::filesystem::is_regular_file(candidate))
            return candidate;
    }
    auto relative = requested;
    auto first = relative.begin();
    if (first != relative.end() && (*first == "assets" || *first == "Assets"))
        relative = relative.lexically_relative(*first);
    const auto candidate = std::filesystem::path(JGRAPHICSLAB_ASSET_DIR) / relative;
    if (std::filesystem::is_regular_file(candidate))
        return candidate;
    return requested;
}
bool Finite(Vec3 v)
{
    return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z);
}
} // namespace

Mesh OBJLoader::Load(const std::string &filepath)
{
    try
    {
        const auto path = ResolveModel(filepath);
        std::error_code error;
        const auto size = std::filesystem::file_size(path, error);
        if (error)
            throw std::runtime_error("模型文件不存在或不可读取");
        if (size > kMaxFileBytes)
            throw std::runtime_error("模型文件超过 64 MiB 预算");
        std::ifstream file(path, std::ios::binary);
        if (!file)
            throw std::runtime_error("无法打开模型文件");
        std::string source(static_cast<std::size_t>(size), '\0');
        if (size && !file.read(source.data(), static_cast<std::streamsize>(size)))
            throw std::runtime_error("模型文件读取不完整");
        tinyobj::ObjReader reader;
        tinyobj::ObjReaderConfig config;
        config.triangulate = true;
        if (!reader.ParseFromString(source, {}, config))
            throw std::runtime_error("tinyobjloader 无法解析模型：" + reader.Error());
        const auto &attributes = reader.GetAttrib();
        if (attributes.vertices.size() / 3 > 500000 || attributes.normals.size() / 3 > 1000000 ||
            attributes.texcoords.size() / 2 > 1000000)
            throw std::runtime_error("模型属性数量超过预算");
        Mesh mesh;
        // 三种独立索引组成键，保留接缝；缺失 UV/法线时保留零值。
        std::map<std::tuple<int, int, int>, unsigned int> unique;
        for (const auto &shape : reader.GetShapes())
        {
            if (shape.mesh.indices.size() % 3 != 0)
                throw std::runtime_error("三角化后的索引数量无效");
            for (const auto &index : shape.mesh.indices)
            {
                if (index.vertex_index < 0 ||
                    static_cast<std::size_t>(index.vertex_index) >= attributes.vertices.size() / 3 ||
                    index.texcoord_index < -1 ||
                    (index.texcoord_index >= 0 &&
                     static_cast<std::size_t>(index.texcoord_index) >= attributes.texcoords.size() / 2) ||
                    index.normal_index < -1 ||
                    (index.normal_index >= 0 &&
                     static_cast<std::size_t>(index.normal_index) >= attributes.normals.size() / 3))
                    throw std::runtime_error("模型包含非法位置、UV 或法线索引");
                const auto key = std::make_tuple(index.vertex_index, index.texcoord_index, index.normal_index);
                auto found = unique.find(key);
                if (found == unique.end())
                {
                    if (mesh.vertices.size() >= kMaxVertices)
                        throw std::runtime_error("统一顶点超过预算");
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
                    if (!Finite(vertex.position) || !Finite(vertex.normal) || !std::isfinite(vertex.uv.x) ||
                        !std::isfinite(vertex.uv.y))
                        throw std::runtime_error("模型包含非有限坐标或属性");
                    const auto id = static_cast<unsigned int>(mesh.vertices.size());
                    mesh.vertices.push_back(vertex);
                    found = unique.emplace(key, id).first;
                }
                if (mesh.indices.size() >= kMaxTriangles * 3)
                    throw std::runtime_error("模型三角形超过预算");
                mesh.indices.push_back(found->second);
            }
        }
        if (mesh.indices.empty())
            throw std::runtime_error("模型不包含可绘制的三角面");
        return mesh;
    }
    catch (const std::exception &error)
    {
        std::cerr << "OBJ 加载失败（" << filepath << "）：" << error.what() << '\n';
        return {};
    }
}
