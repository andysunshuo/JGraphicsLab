/*
【HW09 学习说明：三角形光栅化】
运行选择：Config.h 中 kActiveHomework=9，或启动参数 --hw 9。
当前状态：已有实现。以下调用说明对应当前代码，不自动实现缺失作业。
绘制调度：InitializeApplication -> RenderApplicationFrame -> RunHomework(9,false) -> RenderHW09。

【调用流程、数学原理与计算过程】
实际调用：RenderHW09 -> FillTriangle -> IsInsideTriangle -> EdgeFunction -> Raster::PutPixel。
数学原理：三角形内部一定落在顶点坐标的包围盒内；边函数判断每个像素中心是否被覆盖。
计算过程：默认包围盒 x=10～85、y=10～90，使用 floor(min)、ceil(max) 得到整数候选范围。
每次测试 P=(x+0.5,y+0.5)，通过三个半平面判断后写绿色，其他像素保留背景。
当前二维 FillTriangle 保留原包围盒循环；最终 PutPixel 做写入边界检查，不是 HW31 的裁剪包围盒版本。
在 Renderer/Rasterizer2D.cpp 观察 minX/maxX、xStart/xEnd、采样点 p 与覆盖结果。

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
void RenderHW09()
{

    // 进入包围盒循环，逐个测试中心采样点，再由 PutPixel 写绿色。
    FillTriangle(testTriangle, green);
}
