/*
【HW01 学习说明：像素绘制】
运行选择：Config.h 中 kActiveHomework=1，或启动参数 --hw 1。
当前状态：未实现（Not Implemented）。以下调用说明对应当前代码，不自动实现缺失作业。
绘制调度：InitializeApplication -> RenderApplicationFrame -> RunHomework(1,false) -> RenderHW01。

【调用流程、数学原理与计算过程】
当前调用：RenderHW01 -> PrintNotImplemented(1) -> std::printf，随后 Runner 返回失败。
当前没有作业自己的像素演示，也不会调用 Raster::PutPixel；README 勾选不代表此入口已实现。
数学原理（学习目标）：整数像素 (x,y) 对应一个颜色槽，RGB 打包为 (r<<16)|(g<<8)|b。
公共 PutPixel 的计算：先检查 0<=x<100、0<=y<100，再用 (99-y)*100+x 写入显示内存。
这里 Y 向上，但显示内存从顶部开始排列；例如 (50,50) 对应内存下标 4950。
可学习 Core/Color.cpp 与 Renderer/Framebuffer.cpp；这些公共函数不等于本章已完成。

【阅读与调试】
F11 进入本章下述调用的公共实现；在本文件入口设置断点，可检查本章数据与变量。
公共算法不会被复制到每个 HW；本次只补说明，运算语句保持不变。
*/

#include "Homework/Entries.h"
#include "Geometry/Primitives.h"
#include "Homework/Registry.h"
// 当前源码未包含该作业；保留独立入口，便于逐章设置断点。
// 作业绘制入口：Runner 已清背景和深度，随后执行下列调用。
// 当前只输出未实现状态，不执行文件顶部列出的理论计算。
void RenderHW01()
{
    PrintNotImplemented(1);
}
