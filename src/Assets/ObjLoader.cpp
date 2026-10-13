#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include "Assets/ObjLoader.h"
#include "Assets/ResourcePaths.h"
#include <chrono>
#include <cmath>
#include <fstream>
#include <map>
#include <sstream>
#include <tuple>

namespace Assets
{
MeshLoadResult LoadObj(const std::filesystem::path &path, const LoadLimits &limits)
{
    const auto start = std::chrono::steady_clock::now();
    MeshLoadResult result;
    try
    {
        if (std::filesystem::file_size(path) > limits.maxFileBytes)
            throw std::runtime_error("OBJ 文件超过加载大小上限");
        std::ifstream file(path, std::ios::binary);
        if (!file)
            throw std::runtime_error("无法打开 OBJ 文件");
        // 按块读取并检查预算，文件读取期间增大也不会触发无上限分配。
        std::string text;
        char block[8192];
        while (file.read(block, sizeof(block)) || file.gcount())
        {
            const auto bytes = static_cast<std::size_t>(file.gcount());
            if (text.size() > limits.maxFileBytes || bytes > limits.maxFileBytes - text.size())
                throw std::runtime_error("OBJ 文件读取期间超过上限");
            text.append(block, bytes);
        }
        if (!file.eof())
            throw std::runtime_error("OBJ 文件读取失败");
        // 只检查资源预算，真正的语法、负索引、面三角化由 tinyobjloader 完成。
        std::size_t positions = 0, attributes = 0, estimatedTriangles = 0;
        std::istringstream lines(text);
        std::string line;
        while (std::getline(lines, line))
        {
            if (line.size() > limits.maxLineBytes)
                throw std::runtime_error("OBJ 单行超过内存预算");
            std::istringstream words(line);
            std::string token;
            words >> token;
            if (token == "v")
                ++positions;
            if (token == "vn" || token == "vt")
                ++attributes;
            if (token == "f")
            {
                std::size_t corners = 0;
                while (words >> token && token[0] != '#')
                    ++corners;
                if (corners >= 3)
                    estimatedTriangles += corners - 2;
            }
            if (positions > limits.maxPositions || attributes > limits.maxAttributeRecords ||
                estimatedTriangles > limits.maxTriangles)
                throw std::runtime_error("OBJ 顶点、属性或三角形数量超过加载上限");
        }
        tinyobj::ObjReader reader;
        tinyobj::ObjReaderConfig config;
        config.triangulate = true;
        config.vertex_color = false;
        // 此阶段不读取 MTL：保留几何和属性，着色由 RenderOptions 决定。
        if (!reader.ParseFromString(text, "", config))
            throw std::runtime_error("tinyobjloader 解析失败：" + reader.Error());
        if (!reader.Warning().empty())
            result.warning = "tinyobjloader 警告：" + reader.Warning();
        const auto &attrib = reader.GetAttrib();
        const auto &shapes = reader.GetShapes();
        if (attrib.vertices.size() / 3 > limits.maxPositions ||
            attrib.normals.size() / 3 + attrib.texcoords.size() / 2 > limits.maxAttributeRecords)
            throw std::runtime_error("OBJ 解析属性超过加载预算");
        std::map<std::tuple<int, int, int>, Index> unified;
        for (const auto &shape : shapes)
        {
            std::size_t offset = 0;
            for (const auto corners : shape.mesh.num_face_vertices)
            {
                if (corners != 3 || offset + corners > shape.mesh.indices.size())
                    throw std::runtime_error("OBJ 面三角化结果无效");
                if (result.mesh.indices.size() / 3 >= limits.maxTriangles)
                    throw std::runtime_error("OBJ 三角形数量超过上限");
                for (std::size_t k = 0; k < 3; ++k)
                {
                    const auto idx = shape.mesh.indices[offset + k];
                    if (idx.vertex_index < 0 ||
                        static_cast<std::size_t>(idx.vertex_index) >= attrib.vertices.size() / 3 ||
                        idx.normal_index < -1 ||
                        (idx.normal_index >= 0 &&
                         static_cast<std::size_t>(idx.normal_index) >= attrib.normals.size() / 3) ||
                        idx.texcoord_index < -1 ||
                        (idx.texcoord_index >= 0 &&
                         static_cast<std::size_t>(idx.texcoord_index) >= attrib.texcoords.size() / 2))
                        throw std::runtime_error("OBJ 包含非法位置、法线或 UV 索引");
                    const auto key = std::make_tuple(idx.vertex_index, idx.normal_index, idx.texcoord_index);
                    auto it = unified.find(key);
                    if (it == unified.end())
                    {
                        if (result.mesh.vertices.size() >= limits.maxVertices)
                            throw std::runtime_error("OBJ 统一顶点数量超过上限");
                        Vertex v;
                        const std::size_t p = static_cast<std::size_t>(idx.vertex_index) * 3;
                        v.position = {attrib.vertices[p], attrib.vertices[p + 1], attrib.vertices[p + 2]};
                        if (idx.normal_index >= 0)
                        {
                            const std::size_t n = static_cast<std::size_t>(idx.normal_index) * 3;
                            v.normal = {attrib.normals[n], attrib.normals[n + 1], attrib.normals[n + 2]};
                        }
                        if (idx.texcoord_index >= 0)
                        {
                            const std::size_t uv = static_cast<std::size_t>(idx.texcoord_index) * 2;
                            v.uv = {attrib.texcoords[uv], attrib.texcoords[uv + 1]};
                        }
                        if (!IsFinite(v.position) || !IsFinite(v.normal) || !std::isfinite(v.uv.x) ||
                            !std::isfinite(v.uv.y))
                            throw std::runtime_error("OBJ 包含非有限位置、法线或 UV");
                        const Index index = static_cast<Index>(result.mesh.vertices.size());
                        result.mesh.vertices.push_back(v);
                        it = unified.emplace(key, index).first;
                    }
                    result.mesh.indices.push_back(it->second);
                }
                offset += corners;
            }
        }
        if (result.mesh.indices.empty())
            throw std::runtime_error("OBJ 不含可渲染三角形");
    }
    catch (const std::exception &e)
    {
        result.mesh = {};
        result.error = "加载模型失败：" + PathUtf8(path) + "；" + e.what();
    }
    result.milliseconds =
        std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
    return result;
}
} // namespace Assets
