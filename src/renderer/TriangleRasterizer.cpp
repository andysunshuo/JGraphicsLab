#include "renderer/TriangleRasterizer.h"
#include "material/Material.h"
#include "scene/Camera.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

float EdgeFunction(const Vec2 &a, const Vec2 &b, const Vec2 &p)
{
    Vec2 ab = b - a;
    Vec2 ap = p - a;
    return ab.Cross(ap);
    /*
    > 0：在边的**左侧**
    <0：在边的**右侧**
    =0：点落在边上
    */
}
bool ComputeBarycentric(const Vec2 &a, const Vec2 &b, const Vec2 &c, const Vec2 &p, Barycentric &out)
{
    const float area = EdgeFunction(a, b, c);

    const Barycentric candidate{ EdgeFunction(b, c, p) / area, EdgeFunction(c, a, p) / area,
                            EdgeFunction(a, b, p) / area };

    out = candidate;
    return true;

}
// 一点在在两个三角形内部，它属于 左上。
bool IsTopLeftEdge(const Vec2 &a, const Vec2 &b)
{
    return b.y < a.y || (b.y == a.y && b.x < a.x);
}
// 视锥体裁剪
float EvaluateClipPlane(const Vec4 &p, int plane)
{
    switch (plane)
    {
    case 0:
        return p.x + p.w;
    case 1:
        return p.w - p.x;
    case 2:
        return p.y + p.w;
    case 3:
        return p.w - p.y;
    case 4:
        return p.z + p.w;
    case 5:
        return p.w - p.z;
    default:
        throw std::invalid_argument("裁剪平面编号无效");
    }
}

//  计算“边与裁剪平面的交点”时，这个新顶点的数据从哪里来？
// 已知线段 AB 上的一个交点比例 t，计算这个交点的完整 ClipVertex。
ClipVertex InterpolateClipVertex(const ClipVertex &a, const ClipVertex &b, float t)
{
    return { Vec4::Lerp(a.position, b.position, t), Vec3::Lerp(a.worldPosition, b.worldPosition, t),
         Vec3::Lerp(a.normal, b.normal, t), Vec2::Lerp(a.uv, b.uv, t), Vec3::Lerp(a.color, b.color, t) };

}
// 把一个多边形裁剪掉一个平面外的部分
// 一次只裁剪一个平面。

std::vector<ClipVertex> ClipPolygonAgainstPlane(const std::vector<ClipVertex>& polygon, int plane)
{
    std::vector<ClipVertex> result;
    result.reserve(polygon.size() + 1);  // 提前给 vector 准备一些容量。
    auto previous = polygon.back();    // .back 返回 vector 的最后一个元素。 
    // 循环处理，这里最重要。  C-A  A-B  B-C 
    // 计算 previous 到裁剪平面的距离
    // 重要
    float dp = EvaluateClipPlane(previous.position, plane);
    for (const auto& current : polygon)
    {
        const float dc = EvaluateClipPlane(current.position, plane);

        if ((dp >= 0) != (dc >= 0))
        {
            // 双精度距离差避免两个有限 float 相减溢出。
            const double denominator = static_cast<double>(dp) - dc;
            if (denominator != 0)
                result.push_back(InterpolateClipVertex(previous, current, static_cast<float>(dp / denominator)));
        }
        // 如果 current 在裁剪平面内部，就把 current 保留下来。
        if (dc >= 0)
            result.push_back(current);
        // 更新循环。
        previous = current;
        dp = dc;
    }
    return result;
}
// 把三角形依次裁剪 6 个视锥体平面
std::vector<ClipVertex> ClipTriangleToFrustum(const ClipVertex &a, const ClipVertex &b, const ClipVertex &c)
{
    if (!a.position.IsFinite() || !b.position.IsFinite() || !c.position.IsFinite())
        return {};
    std::vector<ClipVertex> polygon{a, b, c};
    for (int plane = 0; plane < 6 && !polygon.empty(); ++plane)
        polygon = ClipPolygonAgainstPlane(polygon, plane);
    return polygon;
}


//  一个像素落在三角形内部以后，这个像素的颜色、UV、法线、世界坐标到底是多少？
// abc三角形顶点， 重心坐标。
// 直接用屏幕空间重心坐标插值 3D 属性，会产生透视畸变。
// 现在阶段已经是光栅化阶段，顶点已经走完：Clip 裁剪 → 透视除法 → NDC → 视口变换到屏幕空间
ScreenVertex InterpolateAttributes(const ScreenVertex& a, const ScreenVertex& b, const ScreenVertex& c,
    const Barycentric& w, bool corrected)
{
    auto weights = w;
    // Perspective-Correct Interpolation
    //透视正确插值。 
    // 研究
    if (corrected)  //正交投影 为 false
    {
        weights = { w.alpha * a.reciprocalW, w.beta * b.reciprocalW, w.gamma * c.reciprocalW };
        const float sum = weights.alpha + weights.beta + weights.gamma;
        if (!std::isfinite(sum) || sum <= 0)
            throw std::invalid_argument("透视插值分母无效");
        weights = { weights.alpha / sum, weights.beta / sum, weights.gamma / sum };
    }
    // 输入三个 Vec3
    // 按照当前权重混合
    const auto mix3 = [&](const Vec3& x, const Vec3& y, const Vec3& z) {
        return x * weights.alpha + y * weights.beta + z * weights.gamma;
        };
    ScreenVertex result;
    result.x = a.x * w.alpha + b.x * w.beta + c.x * w.gamma;
    result.y = a.y * w.alpha + b.y * w.beta + c.y * w.gamma;
    // NDC 深度在屏幕空间线性插值，不再除一次 w。
    result.ndcZ = a.ndcZ * w.alpha + b.ndcZ * w.beta + c.ndcZ * w.gamma;
    result.reciprocalW = a.reciprocalW * w.alpha + b.reciprocalW * w.beta + c.reciprocalW * w.gamma;
    result.worldPosition = mix3(a.worldPosition, b.worldPosition, c.worldPosition);
    result.normal = mix3(a.normal, b.normal, c.normal);
    result.color = mix3(a.color, b.color, c.color);
    result.uv = a.uv * weights.alpha + b.uv * weights.beta + c.uv * weights.gamma;
    return result;
}


//  到底哪些像素属于三角形，以及这些像素的深度是多少？
// 把一个屏幕空间三角形转换成一组像素。
namespace
{
// 基础面色和光照共用同一覆盖、深度、插值循环；shade 仅负责线性颜色。
template <class Shade>
void RasterizeCoveredTriangle(ScreenVertex a, ScreenVertex b, ScreenVertex c,
    bool depthTest, bool perspectiveCorrect, bool interpolateOnly, Shade&& shade)
{
    // 检查值
    const auto valid = [](const ScreenVertex& v) {
        return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.ndcZ) && std::isfinite(v.reciprocalW) &&
            v.reciprocalW > 0 && v.color.IsFinite();
        };
    if (!valid(a) || !valid(b) || !valid(c))
        return;

    // 算面积
    float area = EdgeFunction({ a.x, a.y }, { b.x, b.y }, { c.x, c.y });
    if (!std::isfinite(area) || std::abs(area) < 1e-7f)
        return;
    if (area < 0)
    {
        // 重要，为什么要交换值。
        std::swap(b, c);
        area = -area;
    }

    // 先裁浮点包围盒，再转换 int，防止巨大坐标导致整数溢出。
    // 为什么先 float 裁剪，再转 int
    const float minX = std::max(0.f, std::min({ a.x, b.x, c.x })),
        maxX = std::min(float(Raster::WIDTH - 1), std::max({ a.x, b.x, c.x }));
    const float minY = std::max(0.f, std::min({ a.y, b.y, c.y })),
        maxY = std::min(float(Raster::HEIGHT - 1), std::max({ a.y, b.y, c.y }));
    if (minX > maxX || minY > maxY)
        return;


    const Vec2 p0{ a.x, a.y }, p1{ b.x, b.y }, p2{ c.x, c.y };
    const auto same = [](const Vec3& x, const Vec3& y) { return x.x == y.x && x.y == y.y && x.z == y.z; };
    const bool flat = interpolateOnly && same(a.color, b.color) && same(a.color, c.color);
    const auto flatColor = flat ? Raster::EncodeLinearColor(a.color) : Raster::Color{};

    // 进入真正的光栅化循环  逐行↓逐列↓逐像素
    for (int y = int(std::floor(minY)); y <= int(std::ceil(maxY)); ++y)
        for (int x = int(std::floor(minX)); x <= int(std::ceil(maxX)); ++x)
        {
            const Vec2 sample{ x + .5f, y + .5f };
            const float e0 = EdgeFunction(p1, p2, sample), e1 = EdgeFunction(p2, p0, sample),
                e2 = EdgeFunction(p0, p1, sample);
            if (!(e0 > 0 || (e0 == 0 && IsTopLeftEdge(p1, p2))) || !(e1 > 0 || (e1 == 0 && IsTopLeftEdge(p2, p0))) ||
                !(e2 > 0 || (e2 == 0 && IsTopLeftEdge(p0, p1))))
                continue;
            const Barycentric w{ e0 / area, e1 / area, e2 / area };
            const float z = a.ndcZ * w.alpha + b.ndcZ * w.beta + c.ndcZ * w.gamma;
            if (!std::isfinite(z) || z < -1.00001f || z > 1.00001f)
                continue;
            const float depth = std::clamp(z * .5f + .5f, 0.f, 1.f);
            if (!depthTest || Raster::DepthTest(x, y, depth))
                Raster::PutPixel(
                    x, y, flat ? flatColor : Raster::EncodeLinearColor(shade(InterpolateAttributes(a, b, c, w, perspectiveCorrect))));
        }
}

} // namespace

void RasterizeTriangle(ScreenVertex a, ScreenVertex b, ScreenVertex c)
{
    RasterizeCoveredTriangle(a, b, c, true, true, true, [](const ScreenVertex &v) { return v.color; });
}
void RasterizeTriangleShaded(const ScreenVertex &a, const ScreenVertex &b, const ScreenVertex &c,
                             const Material &material, const Camera &camera, const std::vector<Light> &lights,
                             const TriangleRenderOptions &options)
{
    const bool perPixel = options.shading == ShadingMode::Phong;
    if (!perPixel && options.shading != ShadingMode::Flat && options.shading != ShadingMode::Gouraud)
        throw std::invalid_argument("三角形着色方式无效");
    RasterizeCoveredTriangle(a, b, c, options.depthTest, options.perspectiveCorrect, !perPixel,
        [&](const ScreenVertex &v) {
            return perPixel ? material.Evaluate(v.worldPosition, v.normal, camera.eye, v.uv, lights) : v.color;
        });
}
