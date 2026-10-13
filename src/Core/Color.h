#pragma once
#include <vector>

// RGB 颜色结构
//
// r：红色分量
// g：绿色分量
// b：蓝色分量
//
// 暂时使用 0~255 的 float 数值，方便后续插值。
//
// 例如：
// Color3 red = {255.0f, 0.0f, 0.0f};
struct Color3
{
    float r;
    float g;
    float b;
};

#include <cstdint>
namespace Raster {
using Color = std::uint32_t;
int GetR(Color color);
int GetG(Color color);
int GetB(Color color);
Color MakeColor(int r, int g, int b);
}
