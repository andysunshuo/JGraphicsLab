#include "TriangleRasterizer.h"
#include <cmath>

void RasterizeTriangle(ScreenVertex a, ScreenVertex b, ScreenVertex c)
{
    // 转换为整数之前拒绝非有限和屏幕外坐标，避免浮点转整数溢出。
    const auto valid = [](ScreenVertex v) {
        return std::isfinite(v.x) && std::isfinite(v.y) && v.x >= 0 && v.y >= 0 && v.x < Raster::WIDTH &&
               v.y < Raster::HEIGHT;
    };

    if (valid(a))
        Raster::PutPixel((int)a.x, (int)a.y, Raster::MakeColor(255, 0, 0));

    if (valid(b))
        Raster::PutPixel((int)b.x, (int)b.y, Raster::MakeColor(0, 255, 0));

    if (valid(c))
        Raster::PutPixel((int)c.x, (int)c.y, Raster::MakeColor(0, 0, 255));
}
