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
    Flat,
    Gouraud,
    Phong
};
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
// 以下材质扩展仅保留原有声明，本次基础管线不调用，也未实现光照：
// 1 检查坐标/退化并统一 CCW；2 限制包围盒；3 覆盖与 Top-left；4 插值深度；
// 5 逐样本 DepthTest；6 属性与 Material::Evaluate；7 WriteSample。
// Flat/Gouraud 在 Rasterizer::DrawMesh 中预计算颜色，Phong 在这里逐片元照明。
// target.Samples()==4 时使用四个 (0.25/0.75) 偏移，独立覆盖和深度。
// 不每片元分配临时 vector，不加载纹理，结束 Resolve 由 DrawMesh 负责。
void RasterizeTriangle3D(const ScreenVertex &a, const ScreenVertex &b, const ScreenVertex &c, FrameBuffer &target,
                         const Material &material, const Camera &camera, const std::vector<Light> &lights,
                         const TriangleRenderOptions &options = {});
