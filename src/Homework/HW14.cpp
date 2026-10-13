/*
【HW14 学习说明：二维 MSAA 演示】
运行选择：Config.h 中 kActiveHomework=14，或启动参数 --hw 14。
当前状态：仅演示或部分实现。以下调用说明对应当前代码，不自动实现缺失作业。
绘制调度：InitializeApplication -> RenderApplicationFrame -> RunHomework(14,false) -> RenderHW14。

【调用流程、数学原理与计算过程】
实际调用：RenderHW14 -> RasterizeTriangleMSAA -> IsInsideTriangle -> EdgeFunction；
随后 -> ResolveMSAA -> GetR/GetG/GetB -> MakeColor -> Raster::PutPixel。
数学原理：一个像素内使用四个覆盖采样，以平均颜色估计该像素的三角形覆盖率。
计算过程：采样偏移为 (0.25,0.25)、(0.75,0.25)、(0.25,0.75)、(0.75,0.75)。
覆盖样本写红色，未覆盖样本写黑色；Resolve 分通道求和后用整数除以 4。
例如两个样本覆盖时红通道为 (255+255+0+0)/4=127；四个样本覆盖时为 255。
当前只保存四份颜色，没有每样本深度；再次画别的三角形会重写未覆盖样本，不能当完整场景 MSAA。
在 Renderer/MSAA.cpp 观察 samples、msaaBuffer[y][x][i] 及 Resolve 的 r/g/b。

【阅读与调试】
F11 进入本章下述调用的公共实现；在本文件入口设置断点，可检查本章数据与变量。
公共算法不会被复制到每个 HW；本次只补说明，运算语句保持不变。
*/

#include "Homework/Entries.h"
#include "Geometry/Primitives.h"
#include "Homework/DemoData.h"
#include "Renderer/MSAA.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <stdexcept>
#include <limits>

// 作业绘制入口：Runner 已清背景和深度，随后执行下列调用。
// 具体调用链、采样/变换过程和观察结果见文件顶部学习说明。
void RenderHW14()
{

    // 第 1 步：四个偏移位置分别做覆盖检查，写入四份样本颜色。
    RasterizeTriangleMSAA(testTriangle, red);
    // 第 2 步：提取四份 RGB、分通道求和并整数平均，再写最终像素。
    ResolveMSAA();
}
