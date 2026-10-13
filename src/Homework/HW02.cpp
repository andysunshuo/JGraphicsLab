/*
【HW02 学习说明：水平线与垂直线】
运行选择：Config.h 中 kActiveHomework=2，或启动参数 --hw 2。
当前状态：已有实现。以下调用说明对应当前代码，不自动实现缺失作业。
绘制调度：InitializeApplication -> RenderApplicationFrame -> RunHomework(2,false) -> RenderHW02。

【调用流程、数学原理与计算过程】
实际调用：RenderHW02 -> DrawHorizontalLine(20,10,90,red) -> Raster::PutPixel；
随后 -> DrawVerticalLine(30,10,90,blue) -> Raster::PutPixel。
数学原理：水平线固定 y、递增 x；垂直线固定 x、递增 y，整数步长为 1。
计算过程：端点反序时公共函数先交换起止值，然后包含两个端点地遍历，共各 81 个像素。
交点是 (30,20)，先写红、后写蓝，所以最终为蓝色；本章没有深度测试。
在 Renderer/Rasterizer2D.cpp 观察当前坐标、起止值及颜色。

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
void RenderHW02()
{

    // 第 1 步：固定 y=20，依次写入 x=10～90 的红像素。
    DrawHorizontalLine(20, 10, 90, red);
    // 第 2 步：固定 x=30 写蓝像素；交点覆盖前面的红色。
    DrawVerticalLine(30, 10, 90, blue);
}
