#pragma once
#include "geometry/Mesh.h"
#include "geometry/ModelAsset.h"
#include "material/Material.h"
#include "renderer/FrameBuffer.h"
#include "renderer/TriangleRasterizer.h"
#include "scene/Camera.h"
#include <cstddef>
#include <vector>

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

// Rasterizer.cpp 已实现，窗口与命令行共用现有固定 Raster 缓冲的经典着色入口。
// 不必先写下面的 Rasterizer 类/动态 FrameBuffer，也不用实现 Texture、Ray、BVH。
// 参数 model 是含 AutoFit 的完整仿射模型矩阵；相机、光源、世界属性使用同一空间。
// 本阶段先使用透视相机；Evaluate 的 eye-P 不适用于正交相机的平行视线，正交着色暂明确报错。
// 原有 DrawMeshBasic 的投影能力不受影响；后续可通过统一观察方向参数扩展正交光照。
// 步骤 1：检查 Mesh 索引/属性、相机、材质、光源；mvp=P*V*M；法线复用 Transform::NormalMatrix。
//   求法线逆转置前消去线性部分的整体正尺度，归一化后方向不变，避免小缩放的绝对阈值误判。
//   Mesh::IsValid 尚未实现时，沿用 DrawMeshBasic 的已有检查，不能调用空声明造成链接失败。
// 步骤 2：逐原始三角形生成 ClipVertex；Pworld=M*Position，Nworld=normalMatrix*Nmodel 后归一化。
//   奇异模型矩阵明确报错；法线不乘 MVP，不作为 w=1 的位置变换，不能给法线加平移。
//   缺失/零顶点法线时，本三角形三个法线统一回退为世界几何面法线；退化面跳过。
//   几何面法线 normalize((Bworld-Aworld).Cross(Cworld-Aworld)) 由绕序决定，不能朝相机强行翻转。
// 步骤 3（均在裁剪前）：Flat 用面心/几何面法线/平均 UV 调 Evaluate 一次，三个 color 相同；
//   Gouraud 用各顶点世界位置/变换后的顶点法线/UV 各调一次；Phong 只准备属性。
//   使用纯材质色，不使用旧 palette，不额外乘 Vertex::color，不对光照结果先编码为 sRGB。
// 步骤 4：复用 ClipTriangleToFrustum，同步裁剪世界属性和已算出的 color；
//   多边形三角扇 -> Transform::ToScreen -> RasterizeTriangleShaded；不要在裁剪后重算 Flat/Gouraud。
// 步骤 5：返回真实统计；不清屏、不写文件、不重新加载 OBJ、不整幅 Resolve。
//   统计约定：input=原始三角形数；rasterized=成功 ToScreen 并提交的裁剪后三角形数；
//   rejected=无效/退化/完全裁掉/没有任何子三角形成功提交的原始三角形数。
//   rasterized 是提交数，不是通过深度测试的像素数；裁剪可能使它大于 input。
// 单样本固定目标不需要 Resolve；窗口继续使用 Raster::Pixels()，缓冲来源保持唯一。
RenderStats DrawMeshShaded(const Mesh &mesh, const Mat4 &model, const Camera &camera, const Material &material,
                           const std::vector<Light> &lights, const TriangleRenderOptions &options = {});
// 按 triangleMaterials 选择材质；overrideMaterial 非空时整体替换材质，仍使用同一套管线。
RenderStats DrawModelShaded(const ModelAsset &asset, const Mat4 &model, const Camera &camera,
                           const std::vector<Light> &lights, const TriangleRenderOptions &options = {},
                           const Material *overrideMaterial = nullptr);

// 后续动态缓冲练习，在 Rasterizer.cpp 实现；本轮固定缓冲着色不依赖此类。
// 管线只借用 FrameBuffer，调用者负责其生命周期。
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
