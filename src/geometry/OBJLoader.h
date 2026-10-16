#pragma once
#include "geometry/Mesh.h"
#include "geometry/ModelAsset.h"
#include <string>

// OBJLoader.cpp 使用现有 tinyobjloader 解析 OBJ/MTL；Load 保留原几何接口及失败返回空行为。
// 位置、UV、法线的独立索引交给 tinyobjloader 解析，再按三元组去重。
// 支持负索引、多边形三角化和 v//vn；缺失法线生成平滑法线，完整入口可补盒式 UV。
class OBJLoader
{
  public:
    // 路径使用 UTF-8；相对路径先查 exe 旁，再查编译时资产目录。
    // 失败打印中文原因并返回空 Mesh，调用者需检查 indices.empty()。
    // 不要在帧循环重新 Load：首次保存 Mesh，后续复用。
    // 现有预算：文件 64 MiB、统一顶点 150 万、三角形 50 万；保留这些检查。
    static Mesh Load(const std::string &filepath);
    // 完整资源入口：tinyobjloader 读取 mtllib/usemtl，保留逐三角形材质。
    // 缺失法线按原位置索引生成面积加权平滑法线；缺失 UV 可生成盒式投影。
    // OBJ/MTL/贴图失败均抛带路径的中文异常；MTL 相对 OBJ，贴图相对所属 MTL。
    static ModelAsset LoadAsset(const std::string &filepath, bool generateMissingUV = true);
    // 独立材质替换：选择文件中的第一个材质覆盖整个模型，不重新解析 OBJ。
    static Material LoadMaterial(const std::string &mtlPath, const std::string &textureOverride = {});
};
