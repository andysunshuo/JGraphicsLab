/*
【HW03 学习说明：Bresenham 直线】
运行选择：Config.h 中 kActiveHomework=3，或启动参数 --hw 3。
当前状态：已有实现。以下调用说明对应当前代码，不自动实现缺失作业。
绘制调度：InitializeApplication -> RenderApplicationFrame -> RunHomework(3,false) -> RenderHW03。

【调用流程、数学原理与计算过程】
实际调用：RenderHW03 -> DrawLine(lineStart,lineEnd,red) -> Raster::PutPixel。
默认端点在 DemoData.cpp：从 (10,15) 到 (85,75)；入口没有调用 DrawLine1 斜率演示。
数学原理：Bresenham 用整数误差选择下一步改变 x、y 或同时改变两者，不逐点做浮点除法。
计算过程：端点先 lround；dx=75、dy=60、sx=sy=1，初始 error=dx-dy=15。
每轮保存 doubledError=2*error；大于 -dy 时移动 x 并减 dy，小于 dx 时移动 y 并加 dx。
两次判断都基于该轮保存的 doubledError；到达终点后退出，支持八个方向。
在 Renderer/Rasterizer2D.cpp 观察 x0、y0、error、doubledError。

【阅读与调试】
F11 进入本章下述调用的公共实现；在本文件入口设置断点，可检查本章数据与变量。
公共算法不会被复制到每个 HW；本次只补说明，运算语句保持不变。
*/

#include "Homework/Entries.h"
#include "Geometry/Primitives.h"
#include "Homework/DemoData.h"
#include "Renderer/Rasterizer2D.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <stdexcept>
#include <limits>

// 作业绘制入口：Runner 已清背景和深度，随后执行下列调用。
// 具体调用链、采样/变换过程和观察结果见文件顶部学习说明。
void RenderHW03()
{

    // 进入公共 Bresenham：在其中观察端点取整、误差累计和逐像素移动。
    DrawLine(lineStart, lineEnd, red);
}
