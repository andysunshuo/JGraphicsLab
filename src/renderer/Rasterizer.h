#pragma once
#include "geometry/Mesh.h"
#include "material/Material.h"
#include "renderer/FrameBuffer.h"
#include "renderer/TriangleRasterizer.h"
#include "scene/Camera.h"
#include <cstddef>

struct RenderOptions
{
    bool backFaceCulling = false;
    bool wireframe = false;
    TriangleRenderOptions triangle;
};
struct RenderStats
{
    std::size_t inputTriangles = 0, rasterizedTriangles = 0, rejectedTriangles = 0;
    double milliseconds = 0;
};

// 当前已实现的基础接口：面色直接显示，不使用 Material 或 Light。
// 不清空缓冲，便于多个 Mesh 共享深度；帧开始由调用者 Clear/ClearDepth。
RenderStats DrawMeshBasic(const Mesh &mesh, const Mat4 &model, const Camera &camera, bool faceColors = true,
                          Raster::Color solidColor = 0x00a8c8f0);

// 在 Rasterizer.cpp 实现。管线只借用 FrameBuffer，调用者负责其生命周期。
class Rasterizer
{
  public:
    explicit Rasterizer(FrameBuffer &target);
    // 不清目标，支持同一帧多个 Mesh；帧开始由调用者 Clear/ClearDepth。
    // 顺序：Mesh 验证 -> Model 世界属性/法线逆转置 -> P*V*M -> 六面裁剪 ->
    // 透视除法 -> 视口 -> 三角扇 -> RasterizeTriangle3D -> Resolve。
    // 面/顶点光照调用 Material::Evaluate，不能复制 Lambert 或 Blinn-Phong。
    // 模型与纹理在这里不能从磁盘加载；逐三角形临时空间限于裁剪结果。
    // 记录完整提交数和真实耗时，不用偷偷删除几何换取速度。
    RenderStats DrawMesh(const Mesh &mesh, const Mat4 &model, const Camera &camera, const Material &material,
                         const std::vector<Light> &lights, const RenderOptions &options = {});
    // 先完成二维线框实验；整数 Bresenham 支持所有八个方向，并检查屏幕越界。
    void DrawLine(const Vec2 &a, const Vec2 &b, Raster::Color color);

  private:
    FrameBuffer &target_;
};
