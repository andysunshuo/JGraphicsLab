/*
【HW13 学习说明：背面剔除】
运行选择：Config.h 中 kActiveHomework=13，或启动参数 --hw 13。
当前状态：已有实现。以下调用说明对应当前代码，不自动实现缺失作业。
绘制调度：InitializeApplication -> RenderApplicationFrame -> RunHomework(13,false) -> RenderHW13。

【调用流程、数学原理与计算过程】
实际调用：RenderHW13 -> IsFrontFacing -> EdgeFunction；为正面才 -> FillTriangle -> 覆盖检查 -> PutPixel。
数学原理：屏幕三角形的有向双倍面积 E(A,B,C) 决定绕序，本工程约定逆时针 CCW 为正面。
计算过程：左侧 ccw 的面积值为 35*60=2100，绘制绿色；右侧 cw 的面积值为 -2100，不绘制。
IsFrontFacing 在面积为 0 时返回 false，正面判定是 area>0，依赖当前 Y 向上约定。
本章展示剔除与绕序，尚未执行三维法线点积；背面剔除也不是深度测试的替代。
在 Geometry/Geometry.cpp 观察 area；在本文件比较两次 if 的分支是否进入。

【阅读与调试】
F11 进入本章下述调用的公共实现；在本文件入口设置断点，可检查本章数据与变量。
公共算法不会被复制到每个 HW；本次只补说明，运算语句保持不变。
*/

#include "Homework/Entries.h"
#include "Geometry/Primitives.h"
#include "Homework/DemoData.h"
#include "Renderer/Rasterizer2D.h"
#include "Geometry/Geometry.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <stdexcept>
#include <limits>

// 作业绘制入口：Runner 已清背景和深度，随后执行下列调用。
// 具体调用链、采样/变换过程和观察结果见文件顶部学习说明。
void RenderHW13()
{

    {
        const Triangle ccw = {{10, 10}, {45, 10}, {28, 70}};
        const Triangle cw = {{55, 10}, {72, 70}, {90, 10}};
        // 有向面积为 +2100，满足 CCW 正面约定，所以进入绿色填充。
        if (IsFrontFacing(ccw))
            FillTriangle(ccw, green);
        // 有向面积为 -2100，不满足正面约定，所以不会进入红色填充。
        if (IsFrontFacing(cw))
            FillTriangle(cw, red);
    }
}
