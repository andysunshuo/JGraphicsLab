#pragma once
#include "geometry/Mesh.h"
#include <string>

// OBJLoader.cpp 已有实现，本次保持其接口和源文件不变。
// 位置、UV、法线的独立索引交给 tinyobjloader 解析，再按三元组去重。
// 缺失 UV/法线为零；支持负索引、多边形三角化和 v//vn 写法。
class OBJLoader
{
  public:
    // 路径使用 UTF-8；相对路径先查 exe 旁，再查编译时资产目录。
    // 失败打印中文原因并返回空 Mesh，调用者需检查 indices.empty()。
    // 不要在帧循环重新 Load：首次保存 Mesh，后续复用。
    // 现有预算：文件 64 MiB、统一顶点 150 万、三角形 50 万；保留这些检查。
    static Mesh Load(const std::string &filepath);
};
