#include "renderer/TriangleRasterizer.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

float EdgeFunction(const Vec2 &a, const Vec2 &b, const Vec2 &p)
{
    return (b.x - a.x) * (p.y - a.y) - (b.y - a.y) * (p.x - a.x);
}
bool ComputeBarycentric(const Vec2 &a, const Vec2 &b, const Vec2 &c, const Vec2 &p, Barycentric &out)
{
    const float area = EdgeFunction(a, b, c);
    if (!std::isfinite(area) || std::abs(area) < 1e-7f)
        return false;
    const Barycentric candidate{EdgeFunction(b, c, p) / area, EdgeFunction(c, a, p) / area,
                                EdgeFunction(a, b, p) / area};
    if (!std::isfinite(candidate.alpha) || !std::isfinite(candidate.beta) || !std::isfinite(candidate.gamma))
        return false;
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
ClipVertex InterpolateClipVertex(const ClipVertex &a, const ClipVertex &b, float t)
{
    return {Vec4::Lerp(a.position, b.position, t), Vec3::Lerp(a.worldPosition, b.worldPosition, t),
            Vec3::Lerp(a.normal, b.normal, t), Vec2::Lerp(a.uv, b.uv, t), Vec3::Lerp(a.color, b.color, t)};
}
std::vector<ClipVertex> ClipPolygonAgainstPlane(const std::vector<ClipVertex> &polygon, int plane)
{
    std::vector<ClipVertex> result;
    if (polygon.empty())
        return result;
    if (polygon.size() > 16)
        throw std::invalid_argument("裁剪临时多边形超过预算");
    result.reserve(polygon.size() + 1);
    auto previous = polygon.back();
    float dp = EvaluateClipPlane(previous.position, plane);
    for (const auto &current : polygon)
    {
        const float dc = EvaluateClipPlane(current.position, plane);
        if (!std::isfinite(dp) || !std::isfinite(dc))
            throw std::invalid_argument("裁剪坐标非有限或过大");
        if ((dp >= 0) != (dc >= 0))
        {
            // 双精度距离差避免两个有限 float 相减溢出。
            const double denominator = static_cast<double>(dp) - dc;
            if (denominator != 0)
                result.push_back(InterpolateClipVertex(previous, current, static_cast<float>(dp / denominator)));
        }
        if (dc >= 0)
            result.push_back(current);
        previous = current;
        dp = dc;
    }
    return result;
}
std::vector<ClipVertex> ClipTriangleToFrustum(const ClipVertex &a, const ClipVertex &b, const ClipVertex &c)
{
    if (!a.position.IsFinite() || !b.position.IsFinite() || !c.position.IsFinite())
        return {};
    std::vector<ClipVertex> polygon{a, b, c};
    for (int plane = 0; plane < 6 && !polygon.empty(); ++plane)
        polygon = ClipPolygonAgainstPlane(polygon, plane);
    return polygon;
}
ScreenVertex InterpolateAttributes(const ScreenVertex &a, const ScreenVertex &b, const ScreenVertex &c,
                                   const Barycentric &w, bool corrected)
{
    auto weights = w;
    if (corrected)
    {
        weights = {w.alpha * a.reciprocalW, w.beta * b.reciprocalW, w.gamma * c.reciprocalW};
        const float sum = weights.alpha + weights.beta + weights.gamma;
        if (!std::isfinite(sum) || sum <= 0)
            throw std::invalid_argument("透视插值分母无效");
        weights = {weights.alpha / sum, weights.beta / sum, weights.gamma / sum};
    }
    const auto mix3 = [&](const Vec3 &x, const Vec3 &y, const Vec3 &z) {
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
void RasterizeTriangle(ScreenVertex a, ScreenVertex b, ScreenVertex c)
{
    const auto valid = [](const ScreenVertex &v) {
        return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.ndcZ) && std::isfinite(v.reciprocalW) &&
               v.reciprocalW > 0 && v.color.IsFinite();
    };
    if (!valid(a) || !valid(b) || !valid(c))
        return;
    float area = EdgeFunction({a.x, a.y}, {b.x, b.y}, {c.x, c.y});
    if (!std::isfinite(area) || std::abs(area) < 1e-7f)
        return;
    if (area < 0)
    {
        std::swap(b, c);
        area = -area;
    }
    // 先裁浮点包围盒，再转换 int，防止巨大坐标导致整数溢出。
    const float minX = std::max(0.f, std::min({a.x, b.x, c.x})),
                maxX = std::min(float(Raster::WIDTH - 1), std::max({a.x, b.x, c.x}));
    const float minY = std::max(0.f, std::min({a.y, b.y, c.y})),
                maxY = std::min(float(Raster::HEIGHT - 1), std::max({a.y, b.y, c.y}));
    if (minX > maxX || minY > maxY)
        return;
    const Vec2 p0{a.x, a.y}, p1{b.x, b.y}, p2{c.x, c.y};
    const auto same = [](const Vec3 &x, const Vec3 &y) { return x.x == y.x && x.y == y.y && x.z == y.z; };
    const bool flat = same(a.color, b.color) && same(a.color, c.color);
    const auto flatColor = flat ? Raster::EncodeLinearColor(a.color) : Raster::Color{};
    for (int y = int(std::floor(minY)); y <= int(std::ceil(maxY)); ++y)
        for (int x = int(std::floor(minX)); x <= int(std::ceil(maxX)); ++x)
        {
            const Vec2 sample{x + .5f, y + .5f};
            const float e0 = EdgeFunction(p1, p2, sample), e1 = EdgeFunction(p2, p0, sample),
                        e2 = EdgeFunction(p0, p1, sample);
            if (!(e0 > 0 || (e0 == 0 && IsTopLeftEdge(p1, p2))) || !(e1 > 0 || (e1 == 0 && IsTopLeftEdge(p2, p0))) ||
                !(e2 > 0 || (e2 == 0 && IsTopLeftEdge(p0, p1))))
                continue;
            const Barycentric w{e0 / area, e1 / area, e2 / area};
            const float z = a.ndcZ * w.alpha + b.ndcZ * w.beta + c.ndcZ * w.gamma;
            if (!std::isfinite(z) || z < -1.00001f || z > 1.00001f)
                continue;
            const float depth = std::clamp(z * .5f + .5f, 0.f, 1.f);
            if (Raster::DepthTest(x, y, depth))
                Raster::PutPixel(
                    x, y, flat ? flatColor : Raster::EncodeLinearColor(InterpolateAttributes(a, b, c, w, true).color));
        }
}
