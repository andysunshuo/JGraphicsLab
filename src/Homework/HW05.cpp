/*
【HW05 学习说明：圆形绘制】
运行选择：Config.h 中 kActiveHomework=5，或启动参数 --hw 5。
当前状态：已有实现。以下调用说明对应当前代码，不自动实现缺失作业。
绘制调度：InitializeApplication -> RenderApplicationFrame -> RunHomework(5,false) -> RenderHW05。

【调用流程、数学原理与计算过程】
实际调用：RenderHW05 -> DrawCircleOutline -> inCircleOutline -> Raster::PutPixel。
数学原理：圆周满足 (x-cx)^2+(y-cy)^2=r^2；离散像素采用 abs(distance-r)<=0.5 的圆环。
计算过程：默认圆心 (50,50)、半径 20；遍历候选整数位置，开平方得到距离后判断是否着色。
例如 (70,50) 的距离为 20，会写白色；(50,50) 距离为 0，不在圆周上。
FillCircle 虽有公共实现，但当前入口没有调用它；其整数平方判断为 distance<=R+1。
注意当前原循环 i 使用圆心 y 范围、j 使用 x 范围，却按 (i,j) 传入与写像素；
默认对称圆心掩盖了这一限制，本次注释不更改算法。断点放在 inCircleOutline 的 dx、dy、distance。

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
void RenderHW05()
{

    // 遍历圆周候选点，进入 inCircleOutline 查看距离和 0.5 像素厚度判断。
    DrawCircleOutline(circleCenter, circleRadius, white);
}
