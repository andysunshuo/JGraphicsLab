#include "Renderer/DepthBuffer.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <stdexcept>
#include <limits>

// ============================================================
// HW12 — Depth Buffer
// 深度缓冲
// ============================================================

// 每个像素对应一个深度值。
//
// depthBuffer[y][x]
//
// 当前暂定：
// 深度值越小，距离观察者越近。
//
// 后续进入 MVP 时，需要重新明确深度空间与范围。
float depthBuffer[Raster::HEIGHT][Raster::WIDTH] = {};

// 功能：
// 清空整个 Depth Buffer。
//
// 与 Clear() 清空颜色缓冲区类似，
// 但这里清空的是深度，而不是 RGB 颜色。
//
// 思考：
// 如果深度越小越靠近观察者，
// 应该将深度缓冲初始化为什么值？
void ClearDepthBuffer()
{
    for (int y = 0; y < Raster::HEIGHT; y++)
    {
        for (int x = 0; x < Raster::WIDTH; x++)
        {
            depthBuffer[y][x] = 1.0f;
        }
    }
}

// 功能：
// 判断新的像素深度是否比已有深度更近。
//
// 参数：
// x    ：当前像素 X 坐标。
// y    ：当前像素 Y 坐标。
// depth：新绘制图元在这个像素处的深度。
//
// 返回值：
// true ：新深度更近，可以绘制。
// false：新深度更远，不应该覆盖旧颜色。
//
// 调用示例：
// DepthTest(50, 50, 0.3f);
//
// 思考：
// 1. 怎样访问 depthBuffer[y][x]？
// 2. 比较成功后是否需要更新深度值？
// 3. 同一个像素多次绘制，如何保持正确遮挡？
bool DepthTest(
    int x,
    int y,
    float depth)
{
    // 越界检查
    if (x < 0 || x >= Raster::WIDTH ||
        y < 0 || y >= Raster::HEIGHT)
    {
        return false;
    }


    // 新深度更近
    return DepthTestValue(depthBuffer[y][x],depth);
}

bool DepthTestValue(float &savedDepth,float incomingDepth)
{
    if(incomingDepth<savedDepth) { savedDepth=incomingDepth; return true; }
    return false;
}
