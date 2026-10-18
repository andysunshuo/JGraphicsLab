#pragma once
#include "RasterWindow.h"
#include "geometry/Vertex.h"
#include <vector>

class FrameBuffer;
struct Material;
struct Camera;
struct Light;

// 基础入口：填充三角形、Top-left 覆盖、透视正确颜色插值和深度测试，无光照/材质。
void RasterizeTriangle(ScreenVertex a, ScreenVertex b, ScreenVertex c);

struct Barycentric
{
    float alpha = 0, beta = 0, gamma = 0;
};
enum class ShadingMode
{
    Flat,    // 原始三角形的世界空间面心/几何面法线求一次光照，三个顶点携带同一结果。
    Gouraud, // 原始三个顶点各调用一次材质，光栅化时透视正确插值线性颜色。
    Phong    // 光栅化时插值世界位置/法线/UV，每个可见采样点调用一次材质。
};
// 与 LightingModel 正交：Phong 是“在哪里算光照”，BlinnPhong 是“用什么公式”。
// 例如 shading=Phong + material.model=Lambert 也是合法组合。
struct TriangleRenderOptions
{
    ShadingMode shading = ShadingMode::Phong;
    bool depthTest = true, perspectiveCorrect = true;
};

// (b-a)×(p-a)：CCW 内侧为正；以像素中心 (x+0.5,y+0.5) 或四样本偏移测试。
float EdgeFunction(const Vec2 &a, const Vec2 &b, const Vec2 &point);
// 复用边函数求三个权重；有向面积接近零时 false，不改 out；权重和为 1。
bool ComputeBarycentric(const Vec2 &a, const Vec2 &b, const Vec2 &c, const Vec2 &point, Barycentric &out);
// 本工程 Y 向上且使用上述边函数：边值为零时仅接受 dy<0 || (dy==0 && dx<0)。
bool IsTopLeftEdge(const Vec2 &start, const Vec2 &end);

// 平面编号 0左/1右/2下/3上/4近/5远，内部距离为 x+w,w-x,y+w,w-y,z+w,w-z。
float EvaluateClipPlane(const Vec4 &position, int planeIndex);
// 对 position（包含 w）、worldPosition、normal、uv、color 同步 a+(b-a)*t。
ClipVertex InterpolateClipVertex(const ClipVertex &a, const ClipVertex &b, float t);
// Sutherland-Hodgman 四种内外边情况；交点 t=da/(da-db)，检查分母。
std::vector<ClipVertex> ClipPolygonAgainstPlane(const std::vector<ClipVertex> &polygon, int planeIndex);
// 依次调用六个平面；结果按三角扇绘制，绝不在裁剪前除 w。
std::vector<ClipVertex> ClipTriangleToFrustum(const ClipVertex &a, const ClipVertex &b, const ClipVertex &c);

// 一般属性 q=Σ(lambda*q/w)/Σ(lambda/w)；normal 在着色时再归一化。
// screen x/y 和 NDC z 仍按屏幕重心线性插值，不能把深度再除一次 w。
ScreenVertex InterpolateAttributes(const ScreenVertex &a, const ScreenVertex &b, const ScreenVertex &c,
                                   const Barycentric &weights, bool perspectiveCorrect = true);

// 在 TriangleRasterizer.cpp 补全；复用当前 Raster 的 1000×1000 颜色/深度缓冲。
// 不依赖尚未实现的动态 FrameBuffer，也不引入 MSAA，先保持每像素中心一个样本。
// a/b/c 已经裁剪和视口变换；Flat/Gouraud 的 color 由 DrawMeshShaded 在裁剪前准备。
// 沿用 RasterizeTriangle 的有效性/有向面积/包围盒/Top-left/重心/深度代码，禁止改坐标约定。
// 推荐把二者共有的覆盖循环提成 cpp 内部函数，各入口仅提供不同的线性颜色计算方式，
// 而不是长期维护两套边函数、深度公式或裁剪算法；补全顺序见 docs/从模型到渲染_函数补全顺序.md。
// 像素流程：覆盖 -> 屏幕线性 ndcZ -> [0,1] -> 深度测试 -> 属性插值 -> 计算线性 RGB -> 写颜色。
// Flat/Gouraud：使用 InterpolateAttributes(...).color，不要对已经照明的颜色再照明。
// Phong：Material::Evaluate(插值世界位置,插值法线,camera.eye,插值UV,lights)。
// perspectiveCorrect=false 仅用于比较插值错误；深度始终按屏幕重心线性插值。
// depthTest=false 时跳过深度比较和深度写入；不能先更新深度再决定禁用。
// 末尾 Raster::PutPixel(x,y,Raster::EncodeLinearColor(linearColor))；这里不清屏、不 Resolve。
// 无逐片元 vector 分配、无磁盘读取、无射线；无效几何跳过，非法材质/光源明确报错。
void RasterizeTriangleShaded(const ScreenVertex &a, const ScreenVertex &b, const ScreenVertex &c,
                             const Material &material, const Camera &camera, const std::vector<Light> &lights,
                             const TriangleRenderOptions &options = {});

// 以下是后续动态 FrameBuffer 扩展，保留原有声明；本轮固定缓冲着色不依赖它：
// 1 检查坐标/退化并统一 CCW；2 限制包围盒；3 覆盖与 Top-left；4 插值深度；
// 5 逐样本 DepthTest；6 属性与 Material::Evaluate；7 WriteSample。
// Flat/Gouraud 在 Rasterizer::DrawMesh 中预计算颜色，Phong 在这里逐片元照明。
// target.Samples()==4 时使用四个 (0.25/0.75) 偏移，独立覆盖和深度。
// 不每片元分配临时 vector，不加载纹理，结束 Resolve 由 DrawMesh 负责。
void RasterizeTriangle3D(const ScreenVertex &a, const ScreenVertex &b, const ScreenVertex &c, FrameBuffer &target,
                         const Material &material, const Camera &camera, const std::vector<Light> &lights,
                         const TriangleRenderOptions &options = {});
