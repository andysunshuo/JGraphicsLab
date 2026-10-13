/*
【HW04 学习说明：矩形绘制】
运行选择：Config.h 中 kActiveHomework=4，或启动参数 --hw 4。
当前状态：已有实现。以下调用说明对应当前代码，不自动实现缺失作业。
绘制调度：InitializeApplication -> RenderApplicationFrame -> RunHomework(4,false) -> RenderHW04。

【调用流程、数学原理与计算过程】
实际调用：RenderHW04 -> DrawRectangleOutline -> 两次 DrawHorizontalLine、两次 DrawVerticalLine；
然后 -> FillRectangle -> 双重整数循环；两条路径最终都调用 Raster::PutPixel。
数学原理：轴对齐矩形的两条边固定 y，另两条边固定 x；内部是两个坐标区间的笛卡尔积。
计算过程：默认白框为 (15,15)～(65,55)，绿色填充区为 (20,20)～(40,40)。
填充循环包含两端，因此该区域是 21*21=441 个像素；此函数保持原实现的输入顺序约定。
在 Renderer/Rasterizer2D.cpp 单步查看四条边和填充循环的 i、j；本章不做深度测试。

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
void RenderHW04()
{

    // 第 1 步：公共函数按上下两条水平边、左右两条垂直边绘制白框。
    DrawRectangleOutline(rectMin, rectMax, white);
    // 第 2 步：在这个子矩形内逐列、逐行写绿色，两个端点均包含。
    FillRectangle({20, 20}, {40, 40}, green);
}
