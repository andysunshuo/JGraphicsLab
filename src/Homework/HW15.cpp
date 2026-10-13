/*
【HW15 学习说明：索引图元整合】
运行选择：Config.h 中 kActiveHomework=15，或启动参数 --hw 15。
当前状态：已有实现。以下调用说明对应当前代码，不自动实现缺失作业。
测试调度：InitializeApplication -> RunMathHomeworkTest(15) -> TestHW15。
绘制调度：InitializeApplication -> RenderApplicationFrame -> RunHomework(15,false) -> RenderHW15。
顺序：先运行数学检查，再清空检查可能写入的颜色/深度，最后调用本章绘制入口。

【调用流程、数学原理与计算过程】
测试调用：TestHW15 -> EdgeFunction 检查两面绕序；DrawIndexedTriangles -> FillTriangle；
随后 Raster::ReadPixel 验证有效图元仍可绘制，MathTestContext::check 统计结果。
渲染调用：RenderHW15 -> DrawIndexedTriangles -> FillTriangle -> IsInsideTriangle -> Raster::PutPixel。
数学原理：顶点数组保存位置，索引每三个组成一个三角形；共享顶点可以减少重复存储。
计算过程：四个点组成 80*80 矩形，索引 (0,1,2)、(0,2,3) 共享对角线，两个有向面积均为 6400。
测试额外传入 (999,2,3)；公共函数先验证索引，发现 999 越界后跳过该面，再检查 (50,30) 为白色。
在 Geometry/Geometry.cpp 观察 i、ia/ib/ic 和越界分支；此二维索引演示不是 OBJ Loader。

【阅读与调试】
F11 进入本章下述调用的公共实现；在本文件入口设置断点，可检查本章数据与变量。
公共算法不会被复制到每个 HW；本次只补说明，运算语句保持不变。
*/

#include "Homework/Entries.h"
#include "Geometry/Primitives.h"
#include "Homework/DemoData.h"
#include "Geometry/Geometry.h"
#include "Renderer/Framebuffer.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <stdexcept>
#include <limits>

// 数学入口：按下列顺序调用本章算法；near/nearN 输出实际与预期值，check 汇总 PASS/FAIL。
// 返回 TestResult；本章具体公式与逐步数值见文件顶部学习说明。
TestResult TestHW15()
{
    const int hw = 15;

    constexpr float halfPi = 3.14159265358979323846f * 0.5f;
    MathTestContext checks(hw);

    {
        // 顶点 V0/V1/V2/V3：两个三角形拼成矩形（共享 V0、V2）。
        const std::vector<Point2> vertices = {{10, 10}, {90, 10}, {90, 90}, {10, 90}};
        const std::vector<int> indices = {0, 1, 2, 0, 2, 3};
        checks.check(indices.size() == 6 && vertices.size() == 4, "indexed mesh: 4 vertices / 2 triangles");
        Triangle tri0 = {vertices[indices[0]], vertices[indices[1]], vertices[indices[2]]};
        Triangle tri1 = {vertices[indices[3]], vertices[indices[4]], vertices[indices[5]]};
        checks.check(checks.near(EdgeFunction(tri0.a, tri0.b, tri0.c), 6400.0f), "triangle 0 CCW area2=6400");
        checks.check(checks.near(EdgeFunction(tri1.a, tri1.b, tri1.c), 6400.0f), "triangle 1 CCW area2=6400");
        // 验证非法面被跳过：前两个有效面仍绘制，末尾含索引 999 的面不访问顶点。
        DrawIndexedTriangles(vertices, {0, 1, 2, 0, 2, 3, 999, 2, 3}, white);
        checks.check(Raster::ReadPixel(50, 30) == white, "索引绘制：非法索引不破坏有效三角形");
    }
    std::printf("[HW%02d] 结果：%d PASS, %d FAIL\n\n", hw, checks.result.passed, checks.result.failed);
    return {checks.result.passed, checks.result.failed};
}

// 作业绘制入口：Runner 已清背景和深度，随后执行下列调用。
// 具体调用链、采样/变换过程和观察结果见文件顶部学习说明。
void RenderHW15()
{

    {
        const std::vector<Point2> vertices = {{10, 10}, {90, 10}, {90, 90}, {10, 90}};
        const std::vector<int> indices = {0, 1, 2, 0, 2, 3};
        // 渲染入口每三个索引取一面；F11 可查看索引检查和公共 FillTriangle。
        DrawIndexedTriangles(vertices, indices, white);
    }
}
