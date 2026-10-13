#include "Renderer/LearningPipeline.h"
#include "Geometry/Geometry.h"
#include "Geometry/Surface.h"
#include "Renderer/DepthBuffer.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace RenderLearning
{
namespace
{
bool FiniteAttributes(const Fragment &f)
{
    return IsFinite(f.position) && IsFinite(f.normal) && IsFinite(f.color) && std::isfinite(f.uv.x) &&
           std::isfinite(f.uv.y);
}
Vec3 Shade(const Fragment &fragment, const PipelineOptions &options, Vec3 faceColor)
{
    if (options.shading == Shading::Flat || options.shading == Shading::Gouraud)
        return fragment.color;
    if (options.shading == Shading::FaceNormal || options.shading == Shading::VertexNormal)
    {
        const auto n = Normalize(fragment.normal);
        return {(n.x + 1) * 0.5f, (n.y + 1) * 0.5f, (n.z + 1) * 0.5f};
    }
    if (options.shading == Shading::UV)
        return {fragment.uv.x - std::floor(fragment.uv.x), fragment.uv.y - std::floor(fragment.uv.y), 0.08f};
    if (options.shading == Shading::Texture)
        return faceColor;
    return options.specular ? BlinnPhong(fragment.position, fragment.normal, options.light)
                            : Lambert(fragment.position, fragment.normal, options.light);
}
Barycentric WeightsAt(const PipelineVertex &a, const PipelineVertex &b, const PipelineVertex &c, Point2 p)
{
    Barycentric w{};
    if (!ComputeBarycentric({a.screen, b.screen, c.screen}, p, w))
        throw std::invalid_argument("无法计算退化三角形的属性");
    return w;
}
Vec3 TextureAt(const Fragment &fragment, const PipelineOptions &o, float lod)
{
    if (!o.texture)
        throw std::invalid_argument("纹理着色缺少图像");
    if (o.filter == TextureFilter::Nearest)
        return SampleNearest(*o.texture, fragment.uv);
    if (o.filter == TextureFilter::Bilinear)
        return SampleBilinear(*o.texture, fragment.uv);
    if (!o.mipmaps)
        throw std::invalid_argument("三线性采样缺少 Mipmap");
    return SampleTrilinear(*o.mipmaps, fragment.uv, lod);
}
} // namespace
Vec3 Lambert(Vec3 position, Vec3 normal, const Lighting &light)
{
    const auto n = Normalize(normal), l = Normalize(Subtract(light.lightPosition, position));
    const float intensity = light.ambient + light.diffuse * std::max(0.0f, Dot(n, l));
    return Multiply(light.baseColor, intensity);
}
Vec3 BlinnPhong(Vec3 position, Vec3 normal, const Lighting &light)
{
    if (light.shininess < 0 || !std::isfinite(light.shininess))
        throw std::invalid_argument("高光指数无效");
    const auto n = Normalize(normal), l = Normalize(Subtract(light.lightPosition, position)),
               v = Normalize(Subtract(light.eye, position));
    const auto h = Normalize(Add(l, v));
    const float spec = Dot(n, l) > 0 && Length(h) > 1e-7f
                           ? light.specular * std::pow(std::max(0.0f, Dot(n, h)), light.shininess)
                           : 0;
    return Add(Lambert(position, normal, light), Vec3{spec, spec, spec});
}
Fragment InterpolateAttributes(const PipelineVertex &a, const PipelineVertex &b, const PipelineVertex &c,
                               Barycentric w, bool corrected)
{
    if (corrected)
    {
        w = {w.alpha * a.reciprocalW, w.beta * b.reciprocalW, w.gamma * c.reciprocalW};
        const float sum = w.alpha + w.beta + w.gamma;
        if (!std::isfinite(sum) || std::abs(sum) < 1e-8f)
            throw std::invalid_argument("透视属性插值分母无效");
        w = {w.alpha / sum, w.beta / sum, w.gamma / sum};
    }
    const auto vec3 = [&](Vec3 x, Vec3 y, Vec3 z) {
        return Add(Add(Multiply(x, w.alpha), Multiply(y, w.beta)), Multiply(z, w.gamma));
    };
    const auto uv = Add(Add(Multiply(a.attributes.uv, w.alpha), Multiply(b.attributes.uv, w.beta)),
                        Multiply(c.attributes.uv, w.gamma));
    return {vec3(a.attributes.position, b.attributes.position, c.attributes.position),
            vec3(a.attributes.normal, b.attributes.normal, c.attributes.normal),
            vec3(a.attributes.color, b.attributes.color, c.attributes.color), uv};
}
bool CoveredSample(Point2 a, Point2 b, Point2 c, Point2 p, Barycentric &w)
{
    const float area = EdgeFunction(a, b, c);
    if (!std::isfinite(area) || area <= 1e-7f)
        return false;
    const float e0 = EdgeFunction(b, c, p), e1 = EdgeFunction(c, a, p), e2 = EdgeFunction(a, b, p);
    if (!(e0 > 0 || (e0 == 0 && IsTopLeftEdge(b, c))) || !(e1 > 0 || (e1 == 0 && IsTopLeftEdge(c, a))) ||
        !(e2 > 0 || (e2 == 0 && IsTopLeftEdge(a, b))))
        return false;
    w = {e0 / area, e1 / area, e2 / area};
    return true;
}
RenderTarget::RenderTarget(int w, int h, int count) : width(w), height(h), samples(count)
{
    if (w <= 0 || h <= 0 || w > 2048 || h > 2048 || (count != 1 && count != 4) ||
        static_cast<std::size_t>(w) * h * count > 4 * 1024 * 1024)
        throw std::invalid_argument("渲染目标尺寸或样本数超过预算");
    const auto size = static_cast<std::size_t>(w) * h * count;
    colors.resize(size);
    depths.resize(size);
    coverage.resize(size);
    Clear();
}
void RenderTarget::Clear(Vec3 color)
{
    std::fill(colors.begin(), colors.end(), color);
    std::fill(depths.begin(), depths.end(), 1.0f);
    std::fill(coverage.begin(), coverage.end(), 0);
}
std::size_t RenderTarget::SampleIndex(int x, int y, int sample) const
{
    if (x < 0 || y < 0 || x >= width || y >= height || sample < 0 || sample >= samples)
        throw std::out_of_range("多采样缓冲索引越界");
    return (static_cast<std::size_t>(y) * width + x) * samples + sample;
}
Image RenderTarget::Resolve() const
{
    Image image(width, height);
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
        {
            Vec3 sum{};
            for (int s = 0; s < samples; ++s)
                sum = Add(sum, colors[SampleIndex(x, y, s)]);
            image.At(x, height - 1 - y) = Multiply(sum, 1.0f / samples);
        }
    return image;
}
void RasterizeAttributedTriangle(const PipelineVertex &a, const PipelineVertex &b, const PipelineVertex &c,
                                 RenderTarget &target, const PipelineOptions &options, PipelineStats &stats)
{
    PipelineVertex v0 = a, v1 = b, v2 = c;
    for (const auto &v : {a, b, c})
        if (!std::isfinite(v.screen.x) || !std::isfinite(v.screen.y) || !std::isfinite(v.ndcZ) ||
            !std::isfinite(v.reciprocalW) || v.reciprocalW <= 0 || !FiniteAttributes(v.attributes))
            return;
    float area = EdgeFunction(v0.screen, v1.screen, v2.screen);
    if (!std::isfinite(area) || std::abs(area) < 1e-7f)
        return;
    if (area < 0)
        std::swap(v1, v2);
    const float minX = std::min({a.screen.x, b.screen.x, c.screen.x}),
                maxX = std::max({a.screen.x, b.screen.x, c.screen.x}),
                minY = std::min({a.screen.y, b.screen.y, c.screen.y}),
                maxY = std::max({a.screen.y, b.screen.y, c.screen.y});
    if (maxX < 0 || maxY < 0 || minX >= target.width || minY >= target.height)
        return;
    const int x0 = static_cast<int>(std::floor(std::max(0.0f, minX))),
              x1 = static_cast<int>(std::ceil(std::min(maxX, static_cast<float>(target.width - 1)))),
              y0 = static_cast<int>(std::floor(std::max(0.0f, minY))),
              y1 = static_cast<int>(std::ceil(std::min(maxY, static_cast<float>(target.height - 1))));
    ++stats.rasterTriangles;
    constexpr Point2 offsets[4] = {{0.25f, 0.25f}, {0.75f, 0.25f}, {0.25f, 0.75f}, {0.75f, 0.75f}};
    for (int y = y0; y <= y1; ++y)
        for (int x = x0; x <= x1; ++x)
            for (int s = 0; s < target.samples; ++s)
            {
                const Point2 offset = target.samples == 1 ? Point2{0.5f, 0.5f} : offsets[s],
                             p{x + offset.x, y + offset.y};
                Barycentric w{};
                if (!CoveredSample(v0.screen, v1.screen, v2.screen, p, w))
                    continue;
                const auto index = target.SampleIndex(x, y, s);
                ++target.coverage[index];
                ++stats.coveredSamples;
                const float z = w.alpha * v0.ndcZ + w.beta * v1.ndcZ + w.gamma * v2.ndcZ;
                if (!std::isfinite(z) || z < -1.0001f || z > 1.0001f)
                    continue;
                const float depth = std::clamp(z * 0.5f + 0.5f, 0.0f, 1.0f);
                if (!(depth < target.depths[index]))
                    continue;
                const auto fragment = InterpolateAttributes(v0, v1, v2, w, options.perspectiveCorrect);
                Vec3 textureColor{};
                if (options.shading == Shading::Texture)
                {
                    float lod = 0;
                    if (options.filter == TextureFilter::Trilinear)
                    {
                        const auto fx = InterpolateAttributes(
                            v0, v1, v2, WeightsAt(v0, v1, v2, {p.x + 1, p.y}), options.perspectiveCorrect);
                        const auto fy = InterpolateAttributes(
                            v0, v1, v2, WeightsAt(v0, v1, v2, {p.x, p.y + 1}), options.perspectiveCorrect);
                        if (!options.texture)
                            throw std::invalid_argument("LOD 估计缺少基础纹理");
                        lod = EstimateLod(*options.texture, Subtract(fx.uv, fragment.uv),
                                          Subtract(fy.uv, fragment.uv));
                    }
                    textureColor = TextureAt(fragment, options, lod);
                }
                const Vec3 color = Shade(fragment, options, textureColor);
                if (!IsFinite(color))
                    continue;
                if (DepthTestValue(target.depths[index], depth))
                {
                    target.colors[index] = color;
                    ++stats.depthWrites;
                }
            }
}
PipelineStats DrawMesh(const Mesh &mesh, const Mat4 &model, const Camera &camera,
                       const PipelineOptions &options, RenderTarget &target)
{
    if (mesh.indices.size() % 3 != 0)
        throw std::invalid_argument("学习 Mesh 的索引数量不是三的倍数");
    PipelineStats stats;
    stats.inputTriangles = mesh.indices.size() / 3;
    const auto mvp = Multiply(camera.Projection(), Multiply(camera.View(), model));
    Mat3 normalMatrix{};
    if (options.correctNormals)
        normalMatrix = MakeNormalMatrix(model);
    else
        for (int r = 0; r < 3; ++r)
            for (int c = 0; c < 3; ++c)
                normalMatrix.m[r][c] = model.m[r][c];
    for (std::size_t tri = 0; tri < stats.inputTriangles; ++tri)
    {
        const Index ids[3] = {mesh.indices[tri * 3], mesh.indices[tri * 3 + 1], mesh.indices[tri * 3 + 2]};
        if (ids[0] >= mesh.vertices.size() || ids[1] >= mesh.vertices.size() ||
            ids[2] >= mesh.vertices.size())
            throw std::invalid_argument("学习 Mesh 存在非法索引");
        Vec3 positions[3];
        bool valid = true;
        for (int k = 0; k < 3; ++k)
        {
            const auto p = Multiply(model, MakePosition4(mesh.vertices[ids[k]].position));
            positions[k] = {p.x, p.y, p.z};
            valid &= IsFinite(positions[k]);
        }
        if (!valid)
            continue;
        const Vec3 face = FaceNormal(positions[0], positions[1], positions[2]);
        if (Length(face) < 1e-7f)
            continue;
        const auto center = Multiply(Add(Add(positions[0], positions[1]), positions[2]), 1 / 3.0f);
        const auto flat =
            options.specular ? BlinnPhong(center, face, options.light) : Lambert(center, face, options.light);
        ClipVertex clip[3];
        for (int k = 0; k < 3; ++k)
        {
            const auto &vertex = mesh.vertices[ids[k]];
            const auto n = options.shading == Shading::FaceNormal
                               ? face
                               : Normalize(Multiply(normalMatrix, vertex.normal));
            const auto color = options.shading == Shading::Flat
                                   ? flat
                                   : (options.specular ? BlinnPhong(positions[k], n, options.light)
                                                       : Lambert(positions[k], n, options.light));
            clip[k] = {Multiply(mvp, MakePosition4(vertex.position)),
                       {color.x * 255, color.y * 255, color.z * 255},
                       positions[k],
                       n,
                       vertex.uv};
            const auto p = clip[k].position;
            valid &= std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z) && std::isfinite(p.w) &&
                     IsFinite(n) && IsFinite(color) && std::isfinite(vertex.uv.x) &&
                     std::isfinite(vertex.uv.y);
        }
        if (!valid)
            continue;
        const auto polygon = ClipTriangleToFrustum(clip[0], clip[1], clip[2]);
        for (std::size_t j = 1; j + 1 < polygon.size(); ++j)
        {
            PipelineVertex screen[3];
            const ClipVertex p[3] = {polygon[0], polygon[j], polygon[j + 1]};
            bool ok = true;
            for (int k = 0; k < 3; ++k)
            {
                Vec3 ndc{};
                if (!PerspectiveDivide(p[k].position, ndc) || p[k].position.w <= 1e-7f)
                {
                    ok = false;
                    break;
                }
                screen[k] = {ViewportTransform(ndc, target.width, target.height),
                             ndc.z,
                             1 / p[k].position.w,
                             {p[k].worldPosition,
                              p[k].normal,
                              {p[k].color.r / 255, p[k].color.g / 255, p[k].color.b / 255},
                              p[k].uv}};
            }
            if (ok)
                RasterizeAttributedTriangle(screen[0], screen[1], screen[2], target, options, stats);
        }
    }
    return stats;
}
} // namespace RenderLearning
