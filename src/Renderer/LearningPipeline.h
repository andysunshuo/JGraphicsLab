#pragma once
#include "Renderer/ImageProcessing.h"
#include "Renderer/MeshRenderer.h"
#include "Renderer/Clipping.h"
#include "Geometry/Primitives.h"

namespace RenderLearning
{
struct Lighting
{
    // 同一世界空间中的点光源、相机与材质；此教学模型暂不包含距离衰减和阴影。
    Vec3 lightPosition{2, 2, 3}, eye{1.7f, 1.3f, 3};
    Vec3 baseColor{0.6f, 0.28f, 0.08f};
    float ambient = 0.08f, diffuse = 0.8f, specular = 0.65f, shininess = 48;
};
Vec3 Lambert(Vec3 position, Vec3 normal, const Lighting &light);
Vec3 BlinnPhong(Vec3 position, Vec3 normal, const Lighting &light);
enum class Shading
{
    Flat,
    Gouraud,
    Phong,
    FaceNormal,
    VertexNormal,
    UV,
    Texture
};
enum class TextureFilter
{
    Nearest,
    Bilinear,
    Trilinear
};
struct PipelineOptions
{
    // 三个开关用于正误对照：高光、透视属性校正、法线逆转置。
    Shading shading = Shading::Phong;
    Lighting light;
    bool specular = true, perspectiveCorrect = true, correctNormals = true;
    TextureFilter filter = TextureFilter::Bilinear;
    const Image *texture = nullptr;
    const std::vector<Image> *mipmaps = nullptr;
};
struct Fragment
{
    Vec3 position{}, normal{}, color{};
    Vec2 uv{};
};
struct PipelineVertex
{
    Point2 screen{};
    float ndcZ = 0, reciprocalW = 1;
    Fragment attributes;
};
Fragment InterpolateAttributes(const PipelineVertex &a, const PipelineVertex &b, const PipelineVertex &c,
                               Barycentric weights, bool perspectiveCorrect);
// 输入逆时针屏幕三角形；边值为零时按原 Top-left 判定接收，避免共享边重复覆盖。
bool CoveredSample(Point2 a, Point2 b, Point2 c, Point2 sample, Barycentric &weights);
class RenderTarget
{
  public:
    // 屏幕原点仍在左下；线性颜色、深度、覆盖次数按像素再按样本排列。
    // 单边最多 2048，全部样本不超过 4×1024×1024；Resolve 后图像行号改为从上到下。
    int width, height, samples;
    std::vector<Vec3> colors;
    std::vector<float> depths;
    std::vector<unsigned> coverage;
    RenderTarget(int width, int height, int samples = 1);
    void Clear(Vec3 color = {0.015f, 0.018f, 0.03f});
    Image Resolve() const;
    std::size_t SampleIndex(int x, int y, int sample) const;
};
struct PipelineStats
{
    std::size_t inputTriangles = 0, rasterTriangles = 0, coveredSamples = 0, depthWrites = 0;
};
void RasterizeAttributedTriangle(const PipelineVertex &a, const PipelineVertex &b, const PipelineVertex &c,
                                 RenderTarget &target, const PipelineOptions &options, PipelineStats &stats);
// 五参数接口，共用旧数学、齐次裁剪、深度比较和共享边规则，扩展属性、采样数与着色。
// 不清空目标，调用者可在同一目标叠加多个 Mesh；Model 应为仿射变换。
PipelineStats DrawMesh(const Mesh &mesh, const Mat4 &model, const Camera &camera,
                       const PipelineOptions &options, RenderTarget &target);
} // namespace RenderLearning
