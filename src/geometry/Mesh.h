#pragma once
#include "geometry/Vertex.h"
#include <limits>
#include <string>
#include <vector>

// 包围盒与 Mesh 的辅助算法以后放在 geometry/Mesh.cpp；本次不新增 .cpp。
struct AABB
{
    // 空盒初值不包含原点，首次 Expand 才形成有效盒，避免正坐标模型错误扩大到零。
    Vec3 min{std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(),
             std::numeric_limits<float>::infinity()};
    Vec3 max{-std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity(),
             -std::numeric_limits<float>::infinity()};
    bool IsValid() const; // 所有坐标有限且每轴 min<=max。
    Vec3 Center() const;  // (min+max)/2。
    Vec3 Extent() const;  // max-min。
    void Expand(const Vec3 &point);
    void Expand(const AABB &other);
};

struct Mesh
{
    std::vector<Vertex> vertices;
    // 每连续三个整数是一面；三种 OBJ 索引的组合映射由 OBJLoader 负责。
    std::vector<unsigned int> indices;

    // 检查非空、索引数量是 3 的倍数、每个索引合法、所有属性有限。
    // 发现问题返回 false，并通过可选 error 输出中文原因。
    bool IsValid(std::string *error = nullptr) const;
    bool ComputeBounds(AABB &out) const; // 空网格返回 false，不改 out。
    // 每面叉积 (B-A)×(C-A)；退化面跳过，邻接面累加后归一化。
    // areaWeighted=true 累加未归一化叉积，否则累加单位法线。
    // 接缝/硬边由索引拆分保持；不能按“坐标相同”强行焊接顶点。
    void RecalculateNormals(bool areaWeighted = true);
};
