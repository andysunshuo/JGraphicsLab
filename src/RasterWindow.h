#pragma once
#include "math/Vec3.h"
#include <cstdint>

// 窗口与测试共用 renderer/FrameBuffer.cpp 的真实颜色/深度缓冲。
namespace Raster
{
constexpr int WIDTH = 1000, HEIGHT = 1000;
using Color = std::uint32_t; // 0x00RRGGBB，与窗口的 32 位位图排列一致。

// 已有行为：通道与 255 按位与，不是浮点颜色，也不执行 Gamma 校正。
Color MakeColor(int r, int g, int b);
void Clear(Color color);
// 左下原点、Y 向上；存储行号为 HEIGHT-1-y，越界直接忽略。
void PutPixel(int x, int y, Color color);
void ClearDepth(float depth = 1.0f);
bool DepthTest(int x, int y, float depth);
float GetDepth(int x, int y);
Color GetPixel(int x, int y);
const Color *Pixels();
bool SavePPM(const char *utf8Path);
bool SavePNG(const char *utf8Path); // WIC 无损输出；保存真实 1000×1000 缓冲，不截取窗口缩放图。

// 在 renderer/FrameBuffer.cpp 集中实现颜色空间转换。
// Encode：有限检查、钳制 [0,1]，线性值 <=0.0031308 乘 12.92，否则用 sRGB 幂律。
// 再乘 255 四舍五入，最后调用 MakeColor；非有限颜色明确报错。
Color EncodeLinearColor(const Vec3 &linear);
// Decode：拆通道除 255；sRGB 值 <=0.04045 除 12.92，否则 ((c+0.055)/1.055)^2.4。
Vec3 DecodeSrgbColor(Color encoded);
} // namespace Raster
