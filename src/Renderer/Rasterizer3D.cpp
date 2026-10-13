#include "Renderer/Rasterizer3D.h"
#include "Renderer/DepthBuffer.h"
#include "Renderer/Interpolation.h"
#include "Camera/Transforms.h"
#include "Geometry/Geometry.h"
#include "Geometry/Surface.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <stdexcept>
#include <limits>

// HW31.1 — 从 ClipVertex 生成屏幕顶点
// 这里保留少量接口胶水：透视除法和视口变换是 HW27/28 的职责。
bool MakeScreenVertex3D(const ClipVertex& clipVertex,
    ScreenVertex3D& output)
{
    Vec3 ndc = {};
    if (!PerspectiveDivide(clipVertex.position, ndc))
        return false;
    output.position = ViewportTransform(ndc, Raster::WIDTH, Raster::HEIGHT);
    output.ndcZ = ndc.z;
    output.color = clipVertex.color;
    return true;
}

// ------------------------------------------------------------
// HW31.2 — 绘制一个带深度遮挡的屏幕空间三角形
// 参数：a/b/c 已经过 Model、View、Projection、Clip、Divide、Viewport。
// 实现流程：
// 1. 用三个 position 求 Bounding Box；裁到屏幕合法范围。
// 2. 对每个像素中心 (x+0.5,y+0.5)，用 EdgeFunction 测覆盖。
// 3. 计算 Barycentric(alpha,beta,gamma)。
// 4. 按重心坐标插值 NDC 深度：
//    ndcZ=alpha*a.ndcZ+beta*b.ndcZ+gamma*c.ndcZ。
// 5. 转成深度缓存值 depth=ndcZ*0.5+0.5，范围 [0,1]。
// 6. 调用 HW12 的 DepthTest(x,y,depth)；通过后才写颜色。
// 7. 用 HW11 的 InterpolateColor 计算 RGB，再 PutPixel。
// 注意：透视投影后，屏幕重心坐标线性插值 NDC z 是正确的；
//       但若以后插值 UV、世界空间位置等，必须做透视校正插值。
//       需要处理共享边覆盖规则和退化三角形。
// ------------------------------------------------------------
void RasterizeTriangle3D(const ScreenVertex3D& a,
    const ScreenVertex3D& b,
    const ScreenVertex3D& c)
{
    // 1. 将三角形规范为屏幕空间 CCW，便于使用正的有向边函数值。
    ScreenVertex3D v0 = a, v1 = b, v2 = c;
    float area = EdgeFunction(v0.position, v1.position, v2.position);
    if (std::abs(area) <= 1e-7f) return; // 退化三角形。
    if (area < 0.0f)
    {
        std::swap(v1, v2);
        area = -area;
    }

    // 2. 计算三角形包围盒，限制到 Framebuffer 中，避免越界访问。
    const float minX = std::min({ v0.position.x, v1.position.x, v2.position.x });
    const float maxX = std::max({ v0.position.x, v1.position.x, v2.position.x });
    const float minY = std::min({ v0.position.y, v1.position.y, v2.position.y });
    const float maxY = std::max({ v0.position.y, v1.position.y, v2.position.y });
    const int xMin = std::max(0, static_cast<int>(std::floor(minX)));
    const int xMax = std::min(Raster::WIDTH - 1, static_cast<int>(std::ceil(maxX)));
    const int yMin = std::max(0, static_cast<int>(std::floor(minY)));
    const int yMax = std::min(Raster::HEIGHT - 1, static_cast<int>(std::ceil(maxY)));
    if (xMax < xMin || yMax < yMin) return;

    // 3. Top-left Rule：共享边只由其中一个三角形覆盖，避免重复写入。
    // 注意这里坐标系 Y 向上：左边/上边满足 dy<0 或水平且 dx<0。
    auto isTopLeft = [](const Point2& start, const Point2& end) {
        return IsTopLeftEdge(start,end);
        };
    const bool edge0TopLeft = isTopLeft(v1.position, v2.position);
    const bool edge1TopLeft = isTopLeft(v2.position, v0.position);
    const bool edge2TopLeft = isTopLeft(v0.position, v1.position);

    for (int y = yMin; y <= yMax; ++y)
    {
        for (int x = xMin; x <= xMax; ++x)
        {
            const Point2 p = { x + 0.5f, y + 0.5f };
            const float e0 = EdgeFunction(v1.position, v2.position, p);
            const float e1 = EdgeFunction(v2.position, v0.position, p);
            const float e2 = EdgeFunction(v0.position, v1.position, p);
            // 4. 覆盖测试：内部为正；落在边上则按 Top-left Rule 判定。
            if (!(e0 > 0.0f || (e0 == 0.0f && edge0TopLeft)) ||
                !(e1 > 0.0f || (e1 == 0.0f && edge1TopLeft)) ||
                !(e2 > 0.0f || (e2 == 0.0f && edge2TopLeft)))
                continue;

            // 5. 重心权重：分别对应 v0/v1/v2。
            const Barycentric weights = { e0 / area, e1 / area, e2 / area };
            // NDC 深度已经做过透视除法；它对屏幕位置可线性插值。
            const float ndcZ = weights.alpha * v0.ndcZ +
                weights.beta * v1.ndcZ +
                weights.gamma * v2.ndcZ;
            if (ndcZ < -1.0001f || ndcZ > 1.0001f) continue;
            const float depth = Clamp(ndcZ * 0.5f + 0.5f, 0.0f, 1.0f);
            // 6. 先做深度测试，失败的片元不用再算颜色。
            if (!DepthTest(x, y, depth)) continue;
            const Raster::Color pixelColor = InterpolateColor(
                weights, v0.color, v1.color, v2.color);
            Raster::PutPixel(x, y, pixelColor);
        }
    }
}
