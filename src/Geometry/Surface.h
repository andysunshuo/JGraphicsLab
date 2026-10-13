#pragma once
#include "Geometry/Mesh.h"
#include "Geometry/Primitives.h"

// 与原 HW31 一致的共享边规则，供原光栅化和新增多采样管线共用。
bool IsTopLeftEdge(const Point2 &start, const Point2 &end);
Vec3 FaceNormal(Vec3 a, Vec3 b, Vec3 c);
// 累加每个邻接面的未归一化叉积，得到面积加权顶点法线，同时返回面法线。
std::vector<Vec3> ComputeVertexNormals(Mesh &mesh);
// 仿射矩阵左上 3×3 的逆转置；奇异矩阵给出明确错误。
Mat3 MakeNormalMatrix(const Mat4 &transform);
Mesh MakeUVSphere(int slices = 20, int stacks = 10);
Mesh MakeUVTriangle();
Mesh MakeUVCube();
