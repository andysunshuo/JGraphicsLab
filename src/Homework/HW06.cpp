/*
【HW06 学习说明：三角形线框】
运行选择：Config.h 中 kActiveHomework=6，或启动参数 --hw 6。
当前状态：已有实现。以下调用说明对应当前代码，不自动实现缺失作业。
绘制调度：InitializeApplication -> RenderApplicationFrame -> RunHomework(6,false) -> RenderHW06。

【调用流程、数学原理与计算过程】
实际调用：RenderHW06 -> DrawTriangleWireframe -> DrawLine(A,B)、DrawLine(B,C)、DrawLine(A,C)；
每条线使用 HW03 的 Bresenham 过程，最终调用 Raster::PutPixel。
数学原理：三角形由三个顶点和三条边组成，线框只表示边界，不检查内部覆盖。
计算过程：默认 A=(10,10)、B=(85,15)、C=(45,90)，三条边全部绘为红色。
共享端点可能重复写同色，但不会改变结果；当前入口没有填充、背面剔除或深度测试。
在 Renderer/Rasterizer2D.cpp 依次观察三个 DrawLine 调用的起点、终点。

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
void RenderHW06()
{

    // 按 A-B、B-C、A-C 三条边进入 DrawLine；本章没有扫描三角形内部。
    DrawTriangleWireframe(testTriangle, red);
}
