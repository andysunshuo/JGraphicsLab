/*
【HW08 学习说明：三角形内部判断】
运行选择：Config.h 中 kActiveHomework=8，或启动参数 --hw 8。
当前状态：已有实现。以下调用说明对应当前代码，不自动实现缺失作业。
绘制调度：InitializeApplication -> RenderApplicationFrame -> RunHomework(8,false) -> RenderHW08。

【调用流程、数学原理与计算过程】
实际调用：RenderHW08 -> DrawTriangleWireframe；然后 -> IsInsideTriangle -> 三次 EdgeFunction；
根据布尔结果 -> Raster::PutPixel，将测试点写为绿色或红色。
数学原理：点必须在三条边各自约定的内半平面中；三个边值全非负或全非正表示内部/边界。
计算过程：对默认 A、B、C 与 P=(45,40)，E(A,B,P)=2075、E(B,C,P)=2000、E(C,A,P)=1750。
三者同为正，且三角形有向双倍面积为 5825，非退化，因此测试点绿色。
IsInsideTriangle 另用 EdgeFunction(A,B,C) 排除共线三角形；不强制调用方先统一绕序。
在 Geometry/Geometry.cpp 观察 e0、e1、e2、area；当前入口没有使用 outsidePoint。

【阅读与调试】
F11 进入本章下述调用的公共实现；在本文件入口设置断点，可检查本章数据与变量。
公共算法不会被复制到每个 HW；本次只补说明，运算语句保持不变。
*/

#include "Homework/Entries.h"
#include "Geometry/Primitives.h"
#include "Homework/DemoData.h"
#include "Renderer/Rasterizer2D.h"
#include "Geometry/Geometry.h"
#include "Renderer/Framebuffer.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <stdexcept>
#include <limits>

// 作业绘制入口：Runner 已清背景和深度，随后执行下列调用。
// 具体调用链、采样/变换过程和观察结果见文件顶部学习说明。
void RenderHW08()
{

    // 第 1 步：显示三角形边界，作为内部判定的图形参考。
    DrawTriangleWireframe(testTriangle, white);
    // 第 2 步：三个边值同号且非退化时为内部；默认测试点应为绿色。
    Raster::PutPixel(45, 40, IsInsideTriangle(testTriangle, insidePoint) ? green : red);
}
