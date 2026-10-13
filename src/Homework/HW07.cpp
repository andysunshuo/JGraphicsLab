/*
【HW07 学习说明：边函数】
运行选择：Config.h 中 kActiveHomework=7，或启动参数 --hw 7。
当前状态：已有实现。以下调用说明对应当前代码，不自动实现缺失作业。
绘制调度：InitializeApplication -> RenderApplicationFrame -> RunHomework(7,false) -> RenderHW07。

【调用流程、数学原理与计算过程】
实际调用：RenderHW07 -> EdgeFunction；结果为正时 -> Raster::PutPixel(45,50,green)；
随后 -> DrawLine -> Raster::PutPixel，画出白色参考边 AB。
数学原理：E(A,B,P)=(Bx-Ax)*(Py-Ay)-(By-Ay)*(Px-Ax)，即二维 AB 与 AP 的叉积。
在 X 向右、Y 向上的约定下，E>0 表示 P 在有向边左侧，E<0 在右侧，E=0 共线。
计算过程：A=(10,30)、B=(90,30)、P=(45,50)，E=80*20-0*35=1600，所以标记绿色。
交换 A、B 会使符号反转；边函数实现位于 Geometry/Geometry.cpp，可观察 ab_x、ab_y、ap_x、ap_y。

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
void RenderHW07()
{

    // 用边函数显示一个点相对 AB 的位置。
    // 第 1 步：计算有向面积值 1600，判断测试点位于参考边左侧。
    if (EdgeFunction({10, 30}, {90, 30}, {45, 50}) > 0)
        Raster::PutPixel(45, 50, green);
    // 第 2 步：画白色参考边，用点和线的相对位置观察边函数符号。
    DrawLine({10, 30}, {90, 30}, white);
}
