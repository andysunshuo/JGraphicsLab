
#include "RasterWindow.h"

// ============================================================
// JGraphicsLab
//
// HW01 - Pixel Rendering
//
// Framebuffer: 100 × 100
//
// X: 0 ~ 99
// Y: 0 ~ 99
//
// Origin: Bottom-Left
//
// ============================================================


void RenderScene()
{
    // 清空画布
    Raster::Clear(
        Raster::MakeColor(0, 0, 0)
    );


    // HW01：绘制三个不同颜色的像素

    // 红色
    Raster::PutPixel(
        20, 20,
        Raster::MakeColor(255, 0, 0)
    );
    Raster::Color green = Raster::MakeColor(0, 255, 0);
    // 绿色
    Raster::PutPixel(
        50, 50,
        green
    );

    // 蓝色
    Raster::PutPixel(
        99, 99,
        Raster::MakeColor(0, 0, 255)
    );

    // ========================================================
    // HW01 - Pixel Rendering
    // ========================================================

    // 在这里完成你的 HW01

}
