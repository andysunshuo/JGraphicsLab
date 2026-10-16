#pragma once
#include "geometry/Mesh.h"
#include "material/Material.h"
#include <string>
#include <vector>

// 一个 OBJ 的几何与材质表分开保存；每个原始三角形有一个材质编号。
// 资源只在加载/切换时解析；渲染函数只访问内存，不打开磁盘文件。
struct ModelAsset
{
    Mesh mesh;
    std::vector<Material> materials;
    std::vector<std::string> materialNames;
    std::vector<unsigned int> triangleMaterials;
    bool generatedUV = false; // 缺失 UV 使用自动盒式投影，不冒充模型原配 UV。
    std::size_t textureCount = 0;
    double loadMilliseconds = 0;
};
