/*
【HW11 学习说明：颜色插值】
运行选择：Config.h 中 kActiveHomework=11，或启动参数 --hw 11。
当前状态：已有实现。以下调用说明对应当前代码，不自动实现缺失作业。
绘制调度：InitializeApplication -> RenderApplicationFrame -> RunHomework(11,false) -> RenderHW11。

【调用流程、数学原理与计算过程】
实际调用：RenderHW11 -> FillTriangleColorInterpolation -> IsInsideTriangle -> ComputeBarycentric；
随后 -> InterpolateColor -> Clamp -> Raster::MakeColor -> Raster::PutPixel。
数学原理：每个颜色通道使用 C=alpha*Ca+beta*Cb+gamma*Cc，重心权重随屏幕位置变化。
计算过程：遍历 100*100 像素，以 (x+0.5,y+0.5) 测覆盖；内部像素才计算权重与 RGB。
默认 A 红、B 绿、C 蓝，因此 R=255*alpha、G=255*beta、B=255*gamma。
例如三权重均为 1/3 时三个通道各约 85；Clamp 将结果限制在 0～255 后转换为整数颜色。
在 Renderer/Rasterizer2D.cpp 与 Interpolation.cpp 观察 p、bary、r/g/bValue；本章未做深度或透视校正。

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
void RenderHW11()
{

    // 逐像素覆盖检查 -> 重心坐标 -> 三通道加权 -> 颜色打包 -> 写帧缓冲。
    FillTriangleColorInterpolation(testTriangle, colorA, colorB, colorC);
}
