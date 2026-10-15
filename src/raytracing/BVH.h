#pragma once
#include "geometry/Mesh.h"
#include "raytracing/Ray.h"
#include <cstddef>
#include <limits>
#include <vector>

// 在 BVH.cpp 实现：每轴求近平面/远平面 t，并更新区间交集。
// direction 某轴为零时直接检查 origin 在该轴范围内，不能除零；射线自身 t 范围也要参与。
bool IntersectAABB(const Ray &ray, const AABB &box, float &nearT, float &farT);

struct BVHNode
{
    static constexpr unsigned int InvalidIndex = std::numeric_limits<unsigned int>::max();
    AABB bounds;
    unsigned int left = InvalidIndex, right = InvalidIndex;
    unsigned int firstTriangle = 0, triangleCount = 0; // 叶节点引用 triangleOrder 的范围。
    bool IsLeaf() const;                               // triangleCount>0；内部节点不能当作几何面测试。
};

// BVH 只加速相同 Mesh，不改变输入几何；先暴力求交验证，再写 Build 和遍历。
class BVH
{
  public:
    // 保存 Mesh 指针；调用者必须保持 Mesh 存活且不再修改顶点/索引。
    // 验证预算/索引，按三角形包围盒中心最长轴做中位数划分。
    // 叶大小 1～64，深度 1～64；最多 50 万面/2N-1 节点，避免无限递归/分配。
    void Build(const Mesh &mesh, unsigned int leafSize = 4, unsigned int maxDepth = 32);
    void Build(Mesh &&mesh, unsigned int leafSize = 4, unsigned int maxDepth = 32) = delete;
    void Clear();
    bool Empty() const;
    std::size_t NodeCount() const;
    // 先箱后面：显式栈遍历，近节点优先，当前最近 t 缩短后续区间。
    // 叶节点转调唯一 IntersectTriangle，并用重心坐标插值 Mesh 法线与 UV。
    // 必须找到最近命中，不能遇到第一个命中就直接返回；未命中不改 out。
    bool Intersect(const Ray &ray, HitRecord &out) const;

  private:
    unsigned int BuildNode(unsigned int first, unsigned int count, unsigned int depth, unsigned int leafSize,
                           unsigned int maxDepth);
    const Mesh *mesh_ = nullptr;
    std::vector<BVHNode> nodes_;
    std::vector<unsigned int> triangleOrder_; // 保存面编号，不保存顶点索引，也不复制 Mesh。
};
