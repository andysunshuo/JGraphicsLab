#include "Renderer/MSAA.h"
#include "Geometry/Geometry.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <stdexcept>
#include <limits>

// ============================================================
// 二、测试用的颜色和几何数据
//
// 这些变量已经帮你准备好。
// 你可以直接在 RenderScene() 中使用。
// ============================================================


Raster::Color msaaBuffer
[
    Raster::HEIGHT
]
[
    Raster::WIDTH
]
[4];

// ============================================================
// HW14 — MSAA 2×2
// 多重采样抗锯齿
// ============================================================

// 功能：
// 使用 2×2 的子采样点进行三角形覆盖测试。
//
// 参数：
// triangle：待光栅化的三角形。
// color   ：三角形颜色。
//
// 当前像素中心为 (x,y)。
//
// 可以使用四个采样偏移：
// (-0.25,-0.25)
// ( 0.25,-0.25)
// (-0.25, 0.25)
// ( 0.25, 0.25)
//
// 每个采样点分别判断是否被三角形覆盖。
//
// 注意：
// 后续需要自己设计：
// 1. 每个采样点的颜色。
// 2. 每个采样点的深度。
// 3. 多个三角形之间的覆盖关系。
//
// 思考：
// 为什么只在像素中心采样会产生锯齿？

void RasterizeTriangleMSAA(
    const Triangle& triangle,
    Raster::Color color)
{

    for (int y = 0; y < Raster::HEIGHT; y++)
    {
        for (int x = 0; x < Raster::WIDTH; x++)
        {

            Point2 samples[4] =
            {
                {x + 0.25f,y + 0.25f},
                {x + 0.75f,y + 0.25f},
                {x + 0.25f,y + 0.75f},
                {x + 0.75f,y + 0.75f}
            };


            for (int i = 0; i < 4; i++)
            {

                if (IsInsideTriangle(
                    triangle,
                    samples[i]))
                {

                    msaaBuffer[y][x][i]
                        = color;

                }
                else
                {

                    msaaBuffer[y][x][i]
                        = Raster::MakeColor(
                            0, 0, 0);

                }
            }
        }
    }
}

// 功能：
// 将一个像素内部的多个采样颜色合成为最终颜色。
//
// 例如：
// 四个子采样颜色：C0、C1、C2、C3。
//
// 最终像素颜色：
// C = (C0+C1+C2+C3)/4
//
// 注意：
// RGB 三个通道需要分别进行平均。
//
// 思考：
// 为什么不能直接对打包后的 uint32_t 颜色值求平均？
void ResolveMSAA()
{
    for (int y = 0; y < Raster::HEIGHT; y++)
    {
        for (int x = 0; x < Raster::WIDTH; x++)
        {

            int r = 0;
            int g = 0;
            int b = 0;


            // 四个采样点平均
            for (int sample = 0; sample < 4; sample++)
            {
                Raster::Color color =
                    msaaBuffer[y][x][sample];


                r += Raster::GetR(color);
                g += Raster::GetG(color);
                b += Raster::GetB(color);
            }


            r /= 4;
            g /= 4;
            b /= 4;


            Raster::PutPixel(
                x,
                y,
                Raster::MakeColor(
                    r,
                    g,
                    b));
        }
    }
}
