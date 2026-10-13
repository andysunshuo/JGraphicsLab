#include "Renderer/Rasterizer2D.h"
#include "Geometry/Geometry.h"
#include "Renderer/Interpolation.h"
#include "Core/Math.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <stdexcept>
#include <limits>

// ============================================================
// HW02 — Horizontal & Vertical Lines
// 水平线与垂直线
// ============================================================

// 功能：
// 在指定的 Y 坐标上绘制一条水平线。
//
// 参数：
// y      ：水平线所在的 Y 坐标。
// xStart ：线段起点的 X 坐标。
// xEnd   ：线段终点的 X 坐标。
// color  ：线段颜色。
//
// 调用示例：
// DrawHorizontalLine(20, 10, 90, red);
//
// 期望结果：
// 在 Y=20 这一行，将 X=10~90 的像素绘制为红色。
//
// 思考：
// 1. 哪个坐标保持不变？
// 2. 哪个坐标需要使用 for 循环？
// 3. 如果 xStart > xEnd 怎么办？
// 4. 是否需要绘制起点和终点？
//
// 验收：
// 支持从左到右，以及从右到左的输入。
void DrawHorizontalLine(
    int y,
    int xStart,
    int xEnd,
    Raster::Color color)
{
    // TODO：自己实现
    if (xStart > xEnd)
    {
        int temp = xStart;
        xStart = xEnd;
        xEnd = temp;
    }

    while (xStart <= xEnd)
    {
        Raster::PutPixel(xStart, y, color);
        xStart++;
    }
}

// 功能：
// 在指定的 X 坐标上绘制一条垂直线。
//
// 参数：
// x      ：垂直线所在的 X 坐标。
// yStart ：线段起点的 Y 坐标。
// yEnd   ：线段终点的 Y 坐标。
// color  ：线段颜色。
//
// 调用示例：
// DrawVerticalLine(30, 10, 90, blue);
//
// 期望结果：
// 在 X=30 这一列，将 Y=10~90 的像素绘制为蓝色。
//
// 思考：
// 1. 哪个坐标保持不变？
// 2. 哪个坐标需要遍历？
// 3. 如何处理反向输入？

void DrawVerticalLine(
    int x,
    int yStart,
    int yEnd,
    Raster::Color color)
{
    // TODO：自己实现
        // TODO：自己实现
    if (yStart > yEnd)
    {
        int temp = yStart;
        yStart = yEnd;
        yEnd = temp;
    }

    while (yStart <= yEnd)
    {
        Raster::PutPixel(x, yStart, color);
        yStart++;
    }
}

// ============================================================
// HW03 — Bresenham Line
// 任意方向直线光栅化
// ============================================================

// 功能：
// 给定两个端点，绘制连接它们的直线。
//
// 参数：
// start：起点，包含 x、y。
// end  ：终点，包含 x、y。
// color：直线颜色。
//
// 调用示例：
// DrawLine(lineStart, lineEnd, red);
//
// start = (10,15)
// end   = (85,75)
//
// 期望结果：
// 在两个端点之间绘制一条连续的红色线段。
//
// 数学知识：
// 1. dx、dy：坐标差。
// 2. 直线斜率。
// 3. Bresenham 整数误差。
// 4. X、Y 的步进方向。
//
// 思考：
// 1. 为什么不能只处理斜率为正的线？
// 2. 如果线段是垂直的怎么办？
// 3. 如果终点在起点左侧怎么办？
// 4. X 和 Y 什么时候需要同时移动？
//
// 验收：
// 支持水平、垂直、正斜率、负斜率、反向输入。
void DrawLine(
    Point2 start,
    Point2 end,
    Raster::Color color)
{
    // 与 HW21/HW29/HW30 集成需要处理变换后产生的浮点屏幕坐标。
    // Bresenham 应在整数像素网格运行：先四舍五入，否则浮点终点
    // 与每次 +1/-1 的当前位置可能永远不相等，导致死循环。
    int x0 = static_cast<int>(std::lround(start.x));
    int y0 = static_cast<int>(std::lround(start.y));
    const int x1 = static_cast<int>(std::lround(end.x));
    const int y1 = static_cast<int>(std::lround(end.y));
    const int dx = std::abs(x1 - x0);
    const int dy = std::abs(y1 - y0);
    const int sx = x0 < x1 ? 1 : -1;
    const int sy = y0 < y1 ? 1 : -1;
    int error = dx - dy;

    while (true)
    {
        if (x0 >= 0 && x0 < Raster::WIDTH && y0 >= 0 && y0 < Raster::HEIGHT)
            Raster::PutPixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        const int doubledError = error * 2;
        if (doubledError > -dy) { error -= dy; x0 += sx; }
        if (doubledError < dx) { error += dx; y0 += sy; }
    }
}

void DrawLine1(
    Point2 start,
    Point2 end,
    Raster::Color color)
{
    // y = kx + b .
    // 计算直线斜率 k
    float dx = end.x - start.x;
    float dy = end.y - start.y;

    // 特殊情况：垂直直线，斜率不存在
    if (dx == 0.0f)
    {
        // TODO: 单独处理垂直直线
        return;
    }

    float k = dy / dx;

    // 计算直线在 Y 轴上的截距 b
    float b = start.y - k * start.x;
    for (int x = start.x; x < end.x; x++)
    {
        //  对当前x来说 y等于多少，然后做一个近似。
    // 计算当前 x 对应的浮点数 y
        float exactY = k * x + b;

        // 四舍五入，取最接近的整数
        int y = (exactY >= 0)
            ? int(exactY + 0.5f)
            : int(exactY - 0.5f);
        Raster::PutPixel(x, y, color);
    }
}

// ============================================================
// HW04 — Rectangle Rendering
// 矩形绘制
// ============================================================

// 功能：
// 绘制一个空心矩形，只绘制四条边。
//
// 参数：
// minPoint：矩形的一个对角顶点。
// maxPoint：矩形的另一个对角顶点。
// color   ：边框颜色。
//
// 调用示例：
// DrawRectangleOutline(rectMin, rectMax, white);
//
// 目标：
// 绘制左下角 (15,15)、右上角 (65,55) 的矩形。
//
// 数学知识：
// 1. 两个对角顶点确定一个轴对齐矩形。
// 2. 根据两个点计算另外两个顶点。
// 3. 连接四条边。
//
// 思考：
// 1. 为什么不需要输入四个顶点？
// 2. 哪两条边是水平的？
// 3. 哪两条边是垂直的？
// 4. 两个顶点交换后是否仍然成立？
void DrawRectangleOutline(
    Point2 minPoint,
    Point2 maxPoint,
    Raster::Color color)
{
    // TODO：自己实现
    DrawHorizontalLine(minPoint.y, minPoint.x, maxPoint.x, color);
    DrawHorizontalLine(maxPoint.y, minPoint.x, maxPoint.x, color);
    DrawVerticalLine(minPoint.x, minPoint.y, maxPoint.y, color);
    DrawVerticalLine(maxPoint.x, minPoint.y, maxPoint.y, color);
}

// 功能：
// 将矩形内部全部填充为指定颜色。
//
// 参数：
// minPoint：一个对角顶点。
// maxPoint：另一个对角顶点。
// color   ：填充颜色。
//
// 调用示例：
// FillRectangle(rectMin, rectMax, green);
//
// 期望结果：
// 矩形内部全部为绿色。
//
// 数学知识：
// xMin = min(x1,x2)
// xMax = max(x1,x2)
// yMin = min(y1,y2)
// yMax = max(y1,y2)
//
// 思考：
// 1. 如何使用两层 for 循环？
// 2. 内层循环遍历 X 还是 Y？
// 3. 如何支持两个点反向输入？
void FillRectangle(
    Point2 minPoint,
    Point2 maxPoint,
    Raster::Color color)
{
    // TODO：自己实现


    for (int i = minPoint.x; i <= maxPoint.x; i++)
    {
        for (int j = minPoint.y; j <= maxPoint.y; j++)
        {
            Raster::PutPixel(i, j, color);
        }
    }
}

// ============================================================
// HW05 — Circle Rendering
// 圆形绘制
// ============================================================

// 功能：
// 绘制只有圆周、没有内部填充的空心圆。
//
// 参数：
// center：圆心坐标。
// radius：圆的半径。
// color ：圆周颜色。
//
// 调用示例：
// DrawCircleOutline(circleCenter, circleRadius, white);
//
// 期望结果：
// 圆心 (50,50)，半径 20 的白色空心圆。
//
// 数学原理：
// (x-a)^2 + (y-b)^2 = r^2
//
// 其中：
// a,b：圆心坐标
// x,y：需要测试的像素坐标
// r  ：半径
//
// 注意：
// 离散像素很难恰好满足等式。
// 可以使用一定厚度的圆环判断圆周附近的像素。
//
// 思考：
// 1. 如何判断一个像素接近圆周？
// 2. 为什么不能直接比较浮点距离是否等于半径？
// 3. 如何决定圆周的厚度？

bool inCircleOutline(int i, int j,
    Point2 center, float radius)
{
    // 判断 i j 是否在圆周上。
        // 1. 计算当前像素到圆心的 X、Y 距离
    float dx = i - center.x;
    float dy = j - center.y;

    // 2. 计算像素中心到圆心的实际距离
    float distance = std::sqrt(dx * dx + dy * dy);

    // 3. 判断距离是否接近圆的半径
    // 这里将圆周看作厚度为 1 像素的环
    return std::abs(distance - radius) <= 0.5f;
}

void DrawCircleOutline(
    Point2 center,
    float radius,
    Raster::Color color)
{
    // TODO：自己实现

    for (int i = center.y - radius; i <= center.y + radius; i++)
    {
        for (int j = center.x - radius; j <= center.x + radius; j++)
        {
            if (inCircleOutline(i, j, center, radius))
            {
                Raster::PutPixel(i, j, color);
            }
        }
    }
}

// 功能：
// 绘制一个实心圆。
//
// 参数：
// center：圆心。
// radius：半径。
// color ：内部填充颜色。
//
// 调用示例：
// FillCircle(circleCenter, circleRadius, green);
//
// 期望结果：
// 绘制圆心 (50,50)、半径 20 的绿色实心圆。
//
// 数学原理：
// (x-a)^2 + (y-b)^2 <= r^2
//
// 实现思路：
// 1. 计算圆的 Bounding Box。
// 2. 遍历包围盒内的像素。
// 3. 判断像素是否位于圆内。
// 4. 对满足条件的像素着色。
//
// 思考：
// 为什么使用距离平方即可完成判断？
//
// 验收：
// 圆形完整，能够处理位于画布边缘的圆。

bool inCircle(int i, int j, Point2 center, float radius)
{
    int dx = i - center.x;
    int dy = j - center.y;

    int distance = dx * dx + dy * dy;
    int R = radius * radius;
    return distance <= (R + 1);
    //  R + 1 和 R的区别 
}

void FillCircle(
    Point2 center,
    float radius,
    Raster::Color color)
{
    // TODO：自己实现
        // TODO：自己实现

    for (int i = center.y - radius; i <= center.y + radius; i++)
    {
        for (int j = center.x - radius; j <= center.x + radius; j++)
        {
            if (inCircle(i, j, center, radius))
            {
                Raster::PutPixel(i, j, color);
            }
        }
    }


}

// ============================================================
// HW06 — Triangle Wireframe
// 三角形线框
// ============================================================

// 功能：
// 使用三个顶点绘制三角形的三条边。
//
// 参数：
// triangle：包含三个顶点的三角形。
// color   ：线框颜色。
//
// 调用示例：
// DrawTriangleWireframe(testTriangle, red);
//
// testTriangle.a = (10,10)
// testTriangle.b = (85,15)
// testTriangle.c = (45,90)
//
// 期望结果：
// 绘制一个红色三角形线框。
//
// 提示：
// 可以复用之前实现的 DrawLine()。
//
// 思考：
// 1. 三角形有多少条边？
// 2. 哪些顶点应该依次连接？
// 3. 改变顶点顺序后会发生什么？
void DrawTriangleWireframe(
    const Triangle& triangle,
    Raster::Color color)
{
    // TODO：自己实现
    DrawLine(triangle.a, triangle.b, color);
    DrawLine(triangle.b, triangle.c, color);
    DrawLine(triangle.a, triangle.c, color);
}

// ============================================================
// HW09 — Triangle Rasterization
// 实心三角形光栅化
// ============================================================

// 功能：
// 将三角形内部的像素填充为指定颜色。
//
// 参数：
// triangle：需要绘制的三角形。
// color   ：填充颜色。
//
// 调用示例：
// FillTriangle(testTriangle, green);
//
// 期望结果：
// 显示一个绿色实心三角形。
//
// 实现步骤：
// 1. 找出三个顶点 X 坐标的最小值、最大值。
// 2. 找出三个顶点 Y 坐标的最小值、最大值。
// 3. 得到三角形的 Bounding Box。
// 4. 遍历包围盒内的像素。
// 5. 使用 IsInsideTriangle() 判断覆盖关系。
// 6. 对内部像素调用 PutPixel()。
//
// 思考：
// 为什么不直接遍历整个 Framebuffer？
void FillTriangle(
    const Triangle& triangle,
    Raster::Color color)
{
    // 1. 获取三个顶点
    Point2 a = triangle.a;
    Point2 b = triangle.b;
    Point2 c = triangle.c;

    // 2. 计算 X 坐标的最小值和最大值
    float minX = std::min({ a.x, b.x, c.x });
    float maxX = std::max({ a.x, b.x, c.x });

    // 3. 计算 Y 坐标的最小值和最大值
    float minY = std::min({ a.y, b.y, c.y });
    float maxY = std::max({ a.y, b.y, c.y });

    // 4. 将浮点坐标转换为整数像素范围
    int xStart = static_cast<int>(std::floor(minX));
    int xEnd = static_cast<int>(std::ceil(maxX));

    int yStart = static_cast<int>(std::floor(minY));
    int yEnd = static_cast<int>(std::ceil(maxY));

    // 5. 遍历 Bounding Box 内的所有像素
    for (int y = yStart; y <= yEnd; ++y)
    {
        for (int x = xStart; x <= xEnd; ++x)
        {
            // 6. 取像素中心作为采样点
            Point2 p{
                x + 0.5f,
                y + 0.5f
            };

            // 7. 判断像素中心是否在三角形内部
            if (IsInsideTriangle(triangle, p))
            {
                // 8. 填充像素颜色
                Raster::PutPixel(x, y, color);
            }
        }
    }
}

// ============================================================
// HW11 Extended — Triangle Color Interpolation
// 三角形顶点颜色插值绘制
// ============================================================
//
// 功能：
// 给三角形三个顶点指定颜色，自动完成内部像素颜色插值。
//
// 参数：
// triangle : 三角形。
// colorA   : 顶点A颜色。
// colorB   : 顶点B颜色。
// colorC   : 顶点C颜色。
//
// 原理：
// 1. 判断像素是否在三角形内部。
// 2. 计算该像素重心坐标。
// 3. 根据 alpha beta gamma 插值颜色。
// 4. 写入 framebuffer。
//


void FillTriangleColorInterpolation(
    const Triangle& triangle,
    const Color3& colorA,
    const Color3& colorB,
    const Color3& colorC)
{

    // 为什么这里遍历整个屏幕而不是包围盒了呢？
    // 遍历整个屏幕

    for (int y = 0; y < Raster::HEIGHT; y++)
    {
        for (int x = 0; x < Raster::WIDTH; x++)
        {

            // 像素中心
            Point2 p =
            {
                x + 0.5f,
                y + 0.5f
            };


            // 判断像素是否属于三角形
            if (IsInsideTriangle(
                triangle,
                p))
            {

                Barycentric bary;


                // 计算重心坐标
                bool success =
                    ComputeBarycentric(
                        triangle,
                        p,
                        bary);


                if (success)
                {

                    // 根据三个顶点颜色插值
                    Raster::Color color =
                        InterpolateColor(
                            bary,
                            colorA,
                            colorB,
                            colorC);



                    // 写入像素
                    Raster::PutPixel(
                        x,
                        y,
                        color);
                }
            }
        }
    }
}
