/*
【HW12 学习说明：深度缓冲演示】
运行选择：Config.h 中 kActiveHomework=12，或启动参数 --hw 12。
当前状态：仅演示或部分实现。以下调用说明对应当前代码，不自动实现缺失作业。
绘制调度：InitializeApplication -> RenderApplicationFrame -> RunHomework(12,false) -> RenderHW12。

【调用流程、数学原理与计算过程】
实际调用：RenderHW12 -> ClearDepthBuffer；然后两次 DepthTest，通过才调用 Raster::PutPixel。
数学原理：每个像素保存目前最小深度；仅当新 depth<depthBuffer[y][x] 时更新并允许写颜色。
计算过程：先把所有深度初始化为 1；(50,50) 的 0.8<1，写蓝色并保存 0.8。
第二次 0.2<0.8，写红色并保存 0.2；最终只有一个红像素，所以窗口看起来大部分是黑色。
公共 Runner 在进入本函数前已清颜色/深度，本函数保留原演示中再次 ClearDepthBuffer 的调用。
这只是单像素遮挡演示，不会加载 OBJ、不画重叠三角形，也没有 TestHW12 自动测试入口。
在 Renderer/DepthBuffer.cpp 的比较语句上断点，观察 depthBuffer[50][50] 从 1 -> 0.8 -> 0.2。

【阅读与调试】
F11 进入本章下述调用的公共实现；在本文件入口设置断点，可检查本章数据与变量。
公共算法不会被复制到每个 HW；本次只补说明，运算语句保持不变。
*/

#include "Homework/Entries.h"
#include "Geometry/Primitives.h"
#include "Homework/DemoData.h"
#include "Renderer/DepthBuffer.h"
#include "Renderer/Framebuffer.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <stdexcept>
#include <limits>

// 作业绘制入口：Runner 已清背景和深度，随后执行下列调用。
// 具体调用链、采样/变换过程和观察结果见文件顶部学习说明。
void RenderHW12()
{

    // 第 1 步：初始化各像素深度为 1；当前颜色背景已由 Runner 清空。
    ClearDepthBuffer();
    // 第 2 步：0.8<1，接受远处蓝色样本，同时将此处深度更新为 0.8。
    if (DepthTest(50, 50, 0.8f))
        Raster::PutPixel(50, 50, blue);
    // 第 3 步：0.2<0.8，接受更近的红色样本并覆盖蓝色。
    if (DepthTest(50, 50, 0.2f))
        Raster::PutPixel(50, 50, red);
}
