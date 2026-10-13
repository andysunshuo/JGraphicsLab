#pragma once
#include "Geometry/Mesh.h"
#include "Camera/Camera.h"

enum class MeshColorMode
{
    Solid,
    FaceColor,
    FaceNormal
};
struct RenderOptions
{
    MeshColorMode colorMode = MeshColorMode::FaceNormal;
    Color3 solidColor{210, 180, 110};
    bool clearDepth = true;
};
struct RenderStats
{
    std::size_t inputTriangles = 0;
    std::size_t rasterizedTriangles = 0;
    std::size_t rejectedTriangles = 0;
    double milliseconds = 0;
};
// 复用 HW30 的六平面裁剪与 HW31 光栅化，保持 P*V*M 和像素约定。
RenderStats DrawMesh(const Mesh &mesh, const Mat4 &modelMatrix, const Camera &camera,
                     const RenderOptions &options = {});
