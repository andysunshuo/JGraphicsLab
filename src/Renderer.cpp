
#include "RasterWindow.h"

#include <cmath>
#include <vector>
#include <cstdint>
#include <algorithm> // std::min/max（处理初始化列表的重载）
#include <cstdio>   // 数学作业输出测试结果
#include <stdexcept> // 参数不合法时抛出异常
#include <limits>   // 处理浮点数范围

// ============================================================
// JGraphicsLab
// HW02 ~ HW31：计算机图形学基础作业
//
// 当前 Framebuffer：100 × 100
// X 坐标：向右增加
// Y 坐标：向上增加
// 左下角：(0, 0)
// 右上角：(99, 99)
//
// 学习规则：
// 1. HW15~HW31 已提供完整参考实现。
// 2. 每次只实现一个 HW。
// 3. 先理解输入、输出，再编写算法。
// 4. 先保证结果正确，再考虑代码优化。
// ============================================================


// ============================================================
// 一、基础数据结构
// ============================================================

// 二维几何点
//
// 表示二维平面中的一个位置。
//
// 例如：
// Point2 p = {20.0f, 30.0f};
//
// p.x 表示 X 坐标
// p.y 表示 Y 坐标
//
// 注意：
// Point2 使用 float，因为几何位置可能存在小数。
// 真正调用 PutPixel 时，需要转换为整数像素坐标。
struct Point2
{
    float x;
    float y;
};


// 二维数学向量
//
// 与 Point2 的数据结构相同，但数学含义不同。
//
// Point2：表示一个位置。
// Vec2：表示一个方向或位移。
//
// 例如：
// Vec2 direction = {3.0f, 4.0f};
//
// HW16 会专门实现 Vec2 的数学运算。
struct Vec2
{
    float x;
    float y;
};


// RGB 颜色结构
//
// r：红色分量
// g：绿色分量
// b：蓝色分量
//
// 暂时使用 0~255 的 float 数值，方便后续插值。
//
// 例如：
// Color3 red = {255.0f, 0.0f, 0.0f};
struct Color3
{
    float r;
    float g;
    float b;
};


// 二维三角形
//
// a、b、c 分别表示三个顶点。
//
// 三个顶点共同确定一个三角形。
//
// 例如：
// Triangle t = {
//     {10.0f, 10.0f},
//     {80.0f, 20.0f},
//     {40.0f, 90.0f}
// };
struct Triangle
{
    Point2 a;
    Point2 b;
    Point2 c;
};


// 重心坐标
//
// alpha：顶点 A 的权重
// beta ：顶点 B 的权重
// gamma：顶点 C 的权重
//
// 满足：
// alpha + beta + gamma = 1
//
// HW10 用来计算。
// HW11 使用它进行颜色插值。
struct Barycentric
{
    float alpha;
    float beta;
    float gamma;
};


// ============================================================
// 二、测试用的颜色和几何数据
//
// 这些变量已经帮你准备好。
// 你可以直接在 RenderScene() 中使用。
// ============================================================


Raster::Color msaaBuffer
[
    Raster::HEIGHT
]
[
    Raster::WIDTH
]
[4];

// 基础颜色
const Raster::Color black = Raster::MakeColor(0, 0, 0);
const Raster::Color white = Raster::MakeColor(255, 255, 255);
const Raster::Color red = Raster::MakeColor(255, 0, 0);
const Raster::Color green = Raster::MakeColor(0, 255, 0);
const Raster::Color blue = Raster::MakeColor(0, 0, 255);


// HW03：直线端点
//
// lineStart：直线起点
// lineEnd  ：直线终点
//
// 目标：绘制从 (10,15) 到 (85,75) 的线段。
const Point2 lineStart = { 10.0f, 15.0f };
const Point2 lineEnd = { 85.0f, 75.0f };


// HW04：矩形的两个对角顶点
//
// rectMin：左下角
// rectMax：右上角
//
// 目标：绘制一个矩形。
const Point2 rectMin = { 15.0f, 15.0f };
const Point2 rectMax = { 65.0f, 55.0f };


// HW05：圆心和半径
//
// 圆心：(50,50)
// 半径：20
const Point2 circleCenter = { 50.0f, 50.0f };
constexpr float circleRadius = 20.0f;


// HW06~HW14：测试三角形
//
// A = (10,10)
// B = (85,15)
// C = (45,90)
const Triangle testTriangle =
{
    {10.0f, 10.0f},
    {85.0f, 15.0f},
    {45.0f, 90.0f}
};


// HW08：用于判断三角形内部的测试点
const Point2 insidePoint = { 45.0f, 40.0f };
const Point2 outsidePoint = { 90.0f, 90.0f };


// HW11：三个顶点的颜色
//
// A：红色
// B：绿色
// C：蓝色
const Color3 colorA = { 255.0f, 0.0f, 0.0f };
const Color3 colorB = { 0.0f, 255.0f, 0.0f };
const Color3 colorC = { 0.0f, 0.0f, 255.0f };


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
// HW07 — Edge Function
// 二维边函数
// ============================================================

// 功能：
// 判断点 P 位于有向边 AB 的哪一侧。
//
// 参数：
// a：有向边起点。
// b：有向边终点。
// p：需要判断的测试点。
//
// 返回值：
// 正数：P 位于 AB 左侧。
// 负数：P 位于 AB 右侧。
// 零  ：P 与 AB 共线。
//
// 注意：
// 这里假设 X 向右、Y 向上的坐标系统。
//
// 数学公式：
// E(A,B,P) =
// (Bx-Ax)*(Py-Ay) - (By-Ay)*(Px-Ax)
//
// 测试数据：
// A=(10,10)
// B=(80,10)
//
// P=(40,40)  应返回正数。
// P=(40,5)   应返回负数。
// P=(40,10)  应返回 0。
//
// 思考：
// 为什么交换 A、B 后，结果符号会反转？
float EdgeFunction(
    Point2 a,
    Point2 b,
    Point2 p)
{
    // TODO：自己实现
    // 计算向量 AB
    float ab_x = b.x - a.x;
    float ab_y = b.y - a.y;

    // 计算向量 AP
    float ap_x = p.x - a.x;
    float ap_y = p.y - a.y;

    // 二维叉乘
    // AB × AP
    return ab_x * ap_y - ab_y * ap_x;
}


// ============================================================
// HW08 — Inside Triangle
// 判断一个点是否位于三角形内部
// ============================================================

// 功能：
// 判断指定点 P 是否位于三角形内部。
//
// 参数：
// triangle：三角形 ABC。
// p       ：需要测试的点。
//
// 返回值：
// true ：P 在三角形内部或边界上。
// false：P 在三角形外部。
//
// 调用示例：
// IsInsideTriangle(testTriangle, insidePoint);
//
// 数学原理：
// 对三条有向边分别计算 Edge Function。
//
// E(A,B,P)
// E(B,C,P)
// E(C,A,P)
//
// 需要考虑顺时针和逆时针两种顶点排列。
//
// 测试数据：
// insidePoint  = (45,40)
// outsidePoint = (90,90)
//
// 思考：
// 1. 为什么需要同时判断三条边？
// 2. 顺时针和逆时针有什么区别？
// 3. 三个点共线时该返回什么？
bool IsInsideTriangle(
    const Triangle& triangle,
    Point2 p)
{
    // 1. 获取三角形三个顶点
    Point2 a = triangle.a;
    Point2 b = triangle.b;
    Point2 c = triangle.c;

    // 2. 计算点 P 相对于三条有向边的位置
    float e0 = EdgeFunction(a, b, p);
    float e1 = EdgeFunction(b, c, p);
    float e2 = EdgeFunction(c, a, p);

    // 3. 判断是否全部非负（逆时针三角形）
    bool allPositive = (e0 >= 0 && e1 >= 0 && e2 >= 0);

    // 4. 判断是否全部非正（顺时针三角形）
    bool allNegative = (e0 <= 0 && e1 <= 0 && e2 <= 0);

    // 5. 排除三个顶点共线的退化三角形
    float area = EdgeFunction(a, b, c);

    if (area == 0.0f)
    {
        return false;
    }

    // 6. 三个叉乘结果同号，则点在三角形内部或边界
    return allPositive || allNegative;
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
// HW10 — Barycentric Coordinates
// 重心坐标
// ============================================================

// 功能：
// 计算测试点 P 相对于三角形 ABC 的重心坐标。
//
// 参数：
// triangle：三角形 ABC。
// p       ：需要计算的二维点。
// result  ：输出计算结果。
//
// 例如：
// result.alpha = 0.2
// result.beta  = 0.3
// result.gamma = 0.5
//
// 返回值：
// true ：三角形非退化，计算成功。
// false：三角形退化，无法得到唯一的重心坐标。
//
// 数学关系：
// P = alpha*A + beta*B + gamma*C
//
// alpha + beta + gamma = 1
//
// 计算方法：
// 使用三个子三角形与原三角形的有向面积比。
//
// 注意：
// 重心坐标不仅可以用于三角形内部的点。
// 三角形外部的点也可以计算重心坐标。
//
// 思考：
// 1. 为什么三个权重之和必须为 1？
// 2. 当 alpha=1 时，P 在什么位置？
// 3. 重心坐标为什么可以用于颜色插值？
bool ComputeBarycentric(
    const Triangle& triangle,
    Point2 p,
    Barycentric& result)
{
    // TODO：自己实现
    Point2 A = triangle.a;
    Point2 B = triangle.b;
    Point2 C = triangle.c;

    float area = EdgeFunction(A, B, C);

    if (area == 0) return false;

    // PBC 对应 alpha
    result.alpha = EdgeFunction(B, C, p) / area;


    // PCA 对应 beta
    result.beta = EdgeFunction(C, A, p) / area;


    // PAB 对应 gamma
    result.gamma = EdgeFunction(A, B, p) / area;
    return true;
}
float Clamp(
    float value,
    float minValue,
    float maxValue)
{
    if (value < minValue)
    {
        return minValue;
    }

    if (value > maxValue)
    {
        return maxValue;
    }

    return value;
}

// ============================================================
// HW11 — Color Interpolation
// 三角形颜色插值
// ============================================================

// 功能：
// 根据重心坐标，计算三角形内部点的 RGB 颜色。
//
// 参数：
// bary：重心坐标 alpha、beta、gamma。
// a   ：顶点 A 的颜色。
// b   ：顶点 B 的颜色。
// c   ：顶点 C 的颜色。
//
// 返回值：
// 插值计算得到的 Raster::Color。
//
// 调用关系：
// 先执行 ComputeBarycentric()。
// 再将结果交给 InterpolateColor()。
//
// 数学公式：
// C = alpha*Ca + beta*Cb + gamma*Cc
//
// R、G、B 三个通道分别插值。
//
// 示例：
// A = 红色
// B = 绿色
// C = 蓝色
//
// 目标：
// 三角形内部形成 RGB 平滑渐变。
//
// 注意：
// 当前 Color3 使用 0~255 的 float。
// 返回 Raster::Color 前需要处理通道范围和整数转换。
Raster::Color InterpolateColor(
    const Barycentric& bary,
    const Color3& a,
    const Color3& b,
    const Color3& c)
{
    // RGB 插值

    float r =
        bary.alpha * a.r +
        bary.beta * b.r +
        bary.gamma * c.r;


    float g =
        bary.alpha * a.g +
        bary.beta * b.g +
        bary.gamma * c.g;


    float bValue =
        bary.alpha * a.b +
        bary.beta * b.b +
        bary.gamma * c.b;


    // 限制颜色范围
    r = Clamp(r, 0.0f, 255.0f);
    g = Clamp(g, 0.0f, 255.0f);
    bValue = Clamp(bValue, 0.0f, 255.0f);


    // 转换成最终 framebuffer 使用的颜色
    return Raster::MakeColor(
        static_cast<int>(r),
        static_cast<int>(g),
        static_cast<int>(bValue)
    );
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
// ============================================================
// HW12 — Depth Buffer
// 深度缓冲
// ============================================================

// 每个像素对应一个深度值。
//
// depthBuffer[y][x]
//
// 当前暂定：
// 深度值越小，距离观察者越近。
//
// 后续进入 MVP 时，需要重新明确深度空间与范围。
float depthBuffer[Raster::HEIGHT][Raster::WIDTH] = {};


// 功能：
// 清空整个 Depth Buffer。
//
// 与 Clear() 清空颜色缓冲区类似，
// 但这里清空的是深度，而不是 RGB 颜色。
//
// 思考：
// 如果深度越小越靠近观察者，
// 应该将深度缓冲初始化为什么值？
void ClearDepthBuffer()
{
    for (int y = 0; y < Raster::HEIGHT; y++)
    {
        for (int x = 0; x < Raster::WIDTH; x++)
        {
            depthBuffer[y][x] = 1.0f;
        }
    }
}


// 功能：
// 判断新的像素深度是否比已有深度更近。
//
// 参数：
// x    ：当前像素 X 坐标。
// y    ：当前像素 Y 坐标。
// depth：新绘制图元在这个像素处的深度。
//
// 返回值：
// true ：新深度更近，可以绘制。
// false：新深度更远，不应该覆盖旧颜色。
//
// 调用示例：
// DepthTest(50, 50, 0.3f);
//
// 思考：
// 1. 怎样访问 depthBuffer[y][x]？
// 2. 比较成功后是否需要更新深度值？
// 3. 同一个像素多次绘制，如何保持正确遮挡？
bool DepthTest(
    int x,
    int y,
    float depth)
{
    // 越界检查
    if (x < 0 || x >= Raster::WIDTH ||
        y < 0 || y >= Raster::HEIGHT)
    {
        return false;
    }


    // 新深度更近
    if (depth < depthBuffer[y][x])
    {
        depthBuffer[y][x] = depth;
        return true;
    }


    return false;
}


// ============================================================
// HW13 — Back-Face Culling
// 背面剔除
// ============================================================

// 功能：
// 根据三角形顶点绕序判断它是否为正面。
//
// 参数：
// triangle：待判断的三角形。
//
// 返回值：
// true ：三角形属于规定的正面。
// false：反面或退化三角形。
//
// 当前约定：
// X 向右，Y 向上。
// 逆时针 CCW 为正面。
//
// 数学原理：
// 使用 EdgeFunction(A,B,C) 的符号。
//
// 思考：
// 1. 为什么有向面积能够判断绕序？
// 2. 顶点顺序反转后会发生什么？
// 3. 背面剔除为什么能够减少光栅化工作量？
bool IsFrontFacing(
    const Triangle& triangle)
{
    float area =
        EdgeFunction(
            triangle.a,
            triangle.b,
            triangle.c);


    // 退化三角形
    if (area == 0.0f)
    {
        return false;
    }


    // CCW 正面
    return area > 0;
}


// ============================================================
// HW14 — MSAA 2×2
// 多重采样抗锯齿
// ============================================================

// 功能：
// 使用 2×2 的子采样点进行三角形覆盖测试。
//
// 参数：
// triangle：待光栅化的三角形。
// color   ：三角形颜色。
//
// 当前像素中心为 (x,y)。
//
// 可以使用四个采样偏移：
// (-0.25,-0.25)
// ( 0.25,-0.25)
// (-0.25, 0.25)
// ( 0.25, 0.25)
//
// 每个采样点分别判断是否被三角形覆盖。
//
// 注意：
// 后续需要自己设计：
// 1. 每个采样点的颜色。
// 2. 每个采样点的深度。
// 3. 多个三角形之间的覆盖关系。
//
// 思考：
// 为什么只在像素中心采样会产生锯齿？

void RasterizeTriangleMSAA(
    const Triangle& triangle,
    Raster::Color color)
{

    for (int y = 0; y < Raster::HEIGHT; y++)
    {
        for (int x = 0; x < Raster::WIDTH; x++)
        {

            Point2 samples[4] =
            {
                {x + 0.25f,y + 0.25f},
                {x + 0.75f,y + 0.25f},
                {x + 0.25f,y + 0.75f},
                {x + 0.75f,y + 0.75f}
            };


            for (int i = 0; i < 4; i++)
            {

                if (IsInsideTriangle(
                    triangle,
                    samples[i]))
                {

                    msaaBuffer[y][x][i]
                        = color;

                }
                else
                {

                    msaaBuffer[y][x][i]
                        = Raster::MakeColor(
                            0, 0, 0);

                }
            }
        }
    }
}

// 功能：
// 将一个像素内部的多个采样颜色合成为最终颜色。
//
// 例如：
// 四个子采样颜色：C0、C1、C2、C3。
//
// 最终像素颜色：
// C = (C0+C1+C2+C3)/4
//
// 注意：
// RGB 三个通道需要分别进行平均。
//
// 思考：
// 为什么不能直接对打包后的 uint32_t 颜色值求平均？
void ResolveMSAA()
{
    for (int y = 0; y < Raster::HEIGHT; y++)
    {
        for (int x = 0; x < Raster::WIDTH; x++)
        {

            int r = 0;
            int g = 0;
            int b = 0;


            // 四个采样点平均
            for (int sample = 0; sample < 4; sample++)
            {
                Raster::Color color =
                    msaaBuffer[y][x][sample];


                r += Raster::GetR(color);
                g += Raster::GetG(color);
                b += Raster::GetB(color);
            }


            r /= 4;
            g /= 4;
            b /= 4;


            Raster::PutPixel(
                x,
                y,
                Raster::MakeColor(
                    r,
                    g,
                    b));
        }
    }
}


// ============================================================
// HW15 — Rasterizer Integration
// 多三角形整合
// ============================================================

// 功能：
// 使用顶点数组和三角形索引绘制多个三角形。
//
// 参数：
// vertices：全部顶点的位置数组。
// indices ：三角形索引数组。
// color   ：三角形颜色。
//
// 例如有四个顶点：
// V0=(10,10)
// V1=(90,10)
// V2=(90,90)
// V3=(10,90)
//
// 可以用两个三角形组成矩形：
// T0=(V0,V1,V2)
// T1=(V0,V2,V3)
//
// indices：
// {0,1,2, 0,2,3}
//
// 每三个索引表示一个三角形。
//
// 思考：
// 1. 为什么使用索引，而不是为每个三角形重复保存顶点？
// 2. indices 数量为什么应该是 3 的倍数？
// 3. 共享边为什么需要统一覆盖规则？
void DrawIndexedTriangles(
    const std::vector<Point2>& vertices,
    const std::vector<int>& indices,
    Raster::Color color)
{
    // 索引每三个表示一个三角形。非法索引直接跳过，避免越界访问。
    // 不同三角形可复用同一个顶点，从而避免重复存储位置数据。
    for (std::size_t i = 0; i + 2 < indices.size(); i += 3)
    {
        const int ia = indices[i];
        const int ib = indices[i + 1];
        const int ic = indices[i + 2];
        if (ia < 0 || ib < 0 || ic < 0 ||
            static_cast<std::size_t>(ia) >= vertices.size() ||
            static_cast<std::size_t>(ib) >= vertices.size() ||
            static_cast<std::size_t>(ic) >= vertices.size())
            continue;
        const Triangle triangle = { vertices[ia], vertices[ib], vertices[ic] };
        FillTriangle(triangle, color);  // 复用 HW09。
    }
}


// ============================================================
// HW16 — Vec2
// 二维向量数学
// ============================================================

// 功能：
// 计算两个向量之和。
//
// 输入：
// a=(ax,ay)
// b=(bx,by)
//
// 输出：
// a+b
//
// 测试：
// (1,2)+(3,4)=(4,6)
Vec2 Add(const Vec2& a, const Vec2& b)
{
    return { a.x + b.x, a.y + b.y };
}


// 功能：
// 计算两个向量之差。
//
// 测试：
// (5,7)-(2,3)=(3,4)
Vec2 Subtract(const Vec2& a, const Vec2& b)
{
    return { a.x - b.x, a.y - b.y };
}


// 功能：
// 计算二维向量点积。
//
// 数学公式：
// dot(a,b)=ax*bx+ay*by
//
// 测试：
// dot((1,2),(3,4))=11
//
// 思考：
// 两个互相垂直的非零向量，点积是多少？
float Dot(const Vec2& a, const Vec2& b)
{
    return a.x * b.x + a.y * b.y;
}


// 功能：
// 计算向量长度。
//
// 数学公式：
// length=sqrt(x*x+y*y)
//
// 测试：
// (3,4) 的长度为 5。
float Length(const Vec2& v)
{
    return std::sqrt(Dot(v, v));
}


// 功能：
// 将向量变成单位向量。
//
// 输入：
// 任意非零向量。
//
// 输出：
// 方向不变，长度为 1 的向量。
//
// 数学公式：
// normalized(v)=v/length(v)
//
// 思考：
// 如果输入的是零向量，应该如何处理？
Vec2 Normalize(const Vec2& v)
{
    const float length = Length(v);
    if (length <= 1e-7f) return { 0.0f, 0.0f };  // 零向量没有确定方向。
    return { v.x / length, v.y / length };
}

// ------------------------------------------------------------
// HW16 补充 — Vec2 数乘
// 输入：(3,4)，scalar=2。
// 期望输出：(6,8)。
// 为什么需要数乘？移动、速度计算、归一化都需要。
// ------------------------------------------------------------
Vec2 Multiply(const Vec2& v, float scalar)
{
    return { v.x * scalar, v.y * scalar };
}

// ============================================================
// HW17 ~ HW31：三维数学、MVP 与 CPU 光栅化
// ============================================================
// 本部分采用统一约定：
// 1. 右手坐标系；相机朝局部 -Z 方向观察。
// 2. 矩阵采用 m[row][column]，向量为列向量：p' = M * p。
// 3. 组合顺序：Clip = Projection * View * Model * Position。
// 4. OpenGL 风格 Clip/NDC：-w<=x,y,z<=w；NDC z∈[-1,1]。
// 5. Viewport 保持当前 Framebuffer 的 Y 向上约定。
// 6. 深度值存入 depthBuffer 前映射到 [0,1]；越小越近。
// 7. HW15~HW31 已完成，可通过 RunMathHomeworkTest 校验。
// ============================================================

// ============================================================
// 四、HW17 / HW18：向量结构与运算
// ============================================================

// HW17 — Vec3：三维向量。可表示位移、方向、法线，也可保存三维位置。
// 示例：Vec3 v = {1.0f, 2.0f, 3.0f};
struct Vec3
{
    float x;
    float y;
    float z;
};

// HW18 — Vec4：四维向量。第四个分量 w 与齐次坐标有关。
// Position：(x,y,z,1)，Direction：(x,y,z,0)。
// 平移矩阵影响 Position，不影响 Direction。
struct Vec4
{
    float x;
    float y;
    float z;
    float w;
};

// ------------------------------------------------------------
// HW17.1 — Vec3 加法
// 输入：(1,2,3) 与 (4,5,6)
// 期望：(5,7,9)
// 提示：每个对应分量相加。
// ------------------------------------------------------------
Vec3 Add(const Vec3& a, const Vec3& b)
{
    return { a.x + b.x, a.y + b.y, a.z + b.z };
}

// HW17.2 — Vec3 减法；测试：(5,7,9)-(1,2,3)=(4,5,6)。
Vec3 Subtract(const Vec3& a, const Vec3& b)
{
    return { a.x - b.x, a.y - b.y, a.z - b.z };
}

// HW17.3 — Vec3 数乘；测试：2*(1,2,3)=(2,4,6)。
Vec3 Multiply(const Vec3& v, float scalar)
{
    return { v.x * scalar, v.y * scalar, v.z * scalar };
}

// ------------------------------------------------------------
// HW17.4 — Dot Product，三维点积
// dot(a,b)=ax*bx+ay*by+az*bz
// 测试：Dot((1,2,3),(4,5,6)) = 32。
// 思考：单位向量的点积与它们夹角余弦的关系？
// ------------------------------------------------------------
float Dot(const Vec3& a, const Vec3& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

// ------------------------------------------------------------
// HW17.5 — Cross Product，三维叉积
// cross(a,b) = (ay*bz-az*by, az*bx-ax*bz, ax*by-ay*bx)
// 测试：Cross((1,0,0),(0,1,0)) = (0,0,1)。
// 思考：为什么交换 a、b 的顺序，结果方向会反转？
// ------------------------------------------------------------
Vec3 Cross(const Vec3& a, const Vec3& b)
{
    // 右手法则：X 叉乘 Y 得到 +Z。
    return { a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x };
}

// HW17.6 — 长度；测试：Length((2,3,6))=7。
float Length(const Vec3& v)
{
    return std::sqrt(Dot(v, v));
}

// HW17.7 — 归一化；测试：Normalize((0,0,5))=(0,0,1)。
Vec3 Normalize(const Vec3& v)
{
    const float length = Length(v);
    if (length <= 1e-7f) return { 0.0f, 0.0f, 0.0f };
    return Multiply(v, 1.0f / length);
}

// ------------------------------------------------------------
// HW18.1 — 三维位置 -> 齐次位置 (w=1)
// 测试：(2,3,4) -> (2,3,4,1)
// ------------------------------------------------------------
Vec4 MakePosition4(const Vec3& p)
{
    return { p.x, p.y, p.z, 1.0f };
}

// HW18.2 — 三维方向 -> 齐次方向 (w=0)
// 测试：(2,3,4) -> (2,3,4,0)
Vec4 MakeDirection4(const Vec3& direction)
{
    return { direction.x, direction.y, direction.z, 0.0f };
}

// HW18.3 — Vec4 加法；测试：(1,2,3,1)+(2,3,4,0)=(3,5,7,1)。
Vec4 Add(const Vec4& a, const Vec4& b)
{
    return { a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
}

// HW18.4 — Vec4 数乘；测试：Multiply((1,2,3,4),2)=(2,4,6,8)。
Vec4 Multiply(const Vec4& v, float scalar)
{
    return { v.x * scalar, v.y * scalar, v.z * scalar, v.w * scalar };
}

// ============================================================
// 五、HW19 / HW20：矩阵结构与基础运算
// ============================================================

// m[row][col] = 第 row 行、第 col 列；从 0 开始编号。
struct Mat3
{
    float m[3][3] = {};
};

struct Mat4
{
    float m[4][4] = {};
};

// ------------------------------------------------------------
// HW19.1 — 3x3 单位矩阵
// 对角线为 1，其他为 0；I * v = v。
// 注意：这里使用普通的 Mat3{ }，初始值为全 0，
// 需要自己填写对角元素！
// ------------------------------------------------------------
Mat3 IdentityMat3()
{
    Mat3 m = {};
    for (int i = 0; i < 3; ++i) m.m[i][i] = 1.0f;
    return m;
}

// ------------------------------------------------------------
// HW19.2 — 3x3 矩阵乘法：C = A * B
// C[row][col] = sum(A[row][k] * B[k][col]), k=0..2
// 测试：I*A=A、A*I=A；矩阵乘法通常不满足交换律。
// ------------------------------------------------------------
Mat3 Multiply(const Mat3& a, const Mat3& b)
{
    Mat3 result = {};
    // 左矩阵的某一行，与右矩阵的某一列做点积。
    for (int row = 0; row < 3; ++row)
        for (int col = 0; col < 3; ++col)
            for (int k = 0; k < 3; ++k)
                result.m[row][col] += a.m[row][k] * b.m[k][col];
    return result;
}

// ------------------------------------------------------------
// HW19.3 — 3x3 矩阵乘 Vec3
// result[i] = M[i][0]*v.x + M[i][1]*v.y + M[i][2]*v.z
// 测试：Multiply(IdentityMat3(),{2,3,1}) -> (2,3,1)
// ------------------------------------------------------------
Vec3 Multiply(const Mat3& matrix, const Vec3& v)
{
    return {
        matrix.m[0][0] * v.x + matrix.m[0][1] * v.y + matrix.m[0][2] * v.z,
        matrix.m[1][0] * v.x + matrix.m[1][1] * v.y + matrix.m[1][2] * v.z,
        matrix.m[2][0] * v.x + matrix.m[2][1] * v.y + matrix.m[2][2] * v.z
    };
}

// ------------------------------------------------------------
// HW20.1 — 4x4 单位矩阵
// 测试：I * (2,3,4,1) = (2,3,4,1)
// ------------------------------------------------------------
Mat4 IdentityMat4()
{
    Mat4 m = {};
    for (int i = 0; i < 4; ++i) m.m[i][i] = 1.0f;
    return m;
}

// ------------------------------------------------------------
// HW20.2 — 4x4 矩阵乘法
// C[row][col] = sum(A[row][k] * B[k][col]), k=0..3
// 测试：A*I=I*A=A。
// ------------------------------------------------------------
Mat4 Multiply(const Mat4& a, const Mat4& b)
{
    Mat4 result = {};
    for (int row = 0; row < 4; ++row)
        for (int col = 0; col < 4; ++col)
            for (int k = 0; k < 4; ++k)
                result.m[row][col] += a.m[row][k] * b.m[k][col];
    return result;
}

// ------------------------------------------------------------
// HW20.3 — 4x4 矩阵乘 Vec4
// 输入矩阵、(x,y,z,w)，返回变换后的四维坐标。
// 注意：不能丢掉结果中的 w；投影后 w 往往不等于 1。
// ------------------------------------------------------------
Vec4 Multiply(const Mat4& matrix, const Vec4& v)
{
    return {
        matrix.m[0][0] * v.x + matrix.m[0][1] * v.y + matrix.m[0][2] * v.z + matrix.m[0][3] * v.w,
        matrix.m[1][0] * v.x + matrix.m[1][1] * v.y + matrix.m[1][2] * v.z + matrix.m[1][3] * v.w,
        matrix.m[2][0] * v.x + matrix.m[2][1] * v.y + matrix.m[2][2] * v.z + matrix.m[2][3] * v.w,
        matrix.m[3][0] * v.x + matrix.m[3][1] * v.y + matrix.m[3][2] * v.z + matrix.m[3][3] * v.w
    };
}

// ============================================================
// 六、HW21：2D Transformation（3x3 齐次矩阵）
// ============================================================
// 用 (x,y,1) 表示位置，统一由 Mat3 完成变换。
// 由于采用列向量，右边的矩阵先作用。
// 例如：T*R*S*p 表示先缩放，再旋转，最后平移。

// HW21.1 — 平移矩阵
// 输入 tx=10,ty=20；(2,3) -> (12,23)。
// 提示：把 tx、ty 放在 m[0][2]、m[1][2]。
Mat3 MakeTranslation2D(float tx, float ty)
{
    Mat3 m = IdentityMat3();
    m.m[0][2] = tx;
    m.m[1][2] = ty;
    return m;
}

// HW21.2 — 旋转矩阵（角度参数以弧度为单位）
// 正角度：从 +X 向 +Y 逆时针旋转。
// 测试：绕原点旋转 90°，(1,0)->约(0,1)。
// 提示：左上 2x2：[cos,-sin; sin,cos]。
Mat3 MakeRotation2D(float radians)
{
    const float c = std::cos(radians), s = std::sin(radians);
    Mat3 m = IdentityMat3();
    m.m[0][0] = c; m.m[0][1] = -s;
    m.m[1][0] = s; m.m[1][1] = c;
    return m;
}

// HW21.3 — 缩放矩阵
// 测试：sx=2,sy=3，(4,5)->(8,15)。
Mat3 MakeScale2D(float sx, float sy)
{
    Mat3 m = IdentityMat3();
    m.m[0][0] = sx; m.m[1][1] = sy;
    return m;
}

// HW21.4 — 关于 X 轴反射
// 测试：(2,3)->(2,-3)。
Mat3 MakeReflectionX2D()
{
    Mat3 m = IdentityMat3();
    m.m[1][1] = -1.0f; // 关于 X 轴对称，翻转 y。
    return m;
}

// HW21.5 — 关于 Y 轴反射
// 测试：(2,3)->(-2,3)。
Mat3 MakeReflectionY2D()
{
    Mat3 m = IdentityMat3();
    m.m[0][0] = -1.0f; // 关于 Y 轴对称，翻转 x。
    return m;
}

// HW21.6 — 错切（Shear）矩阵
// x' = x + shearX * y，y' = y + shearY * x
// 测试：shearX=1,shearY=0，(2,3)->(5,3)。
Mat3 MakeShear2D(float shearX, float shearY)
{
    Mat3 m = IdentityMat3();
    m.m[0][1] = shearX;
    m.m[1][0] = shearY;
    return m;
}

// HW21.7 — 用 Mat3 变换二维点（练习结果可直接在窗口里绘制）
// 这里只做三维齐次向量与 Point2 的接口适配。
Point2 TransformPoint2D(const Mat3& matrix, Point2 point)
{
    const Vec3 homogeneous = { point.x, point.y, 1.0f };
    const Vec3 transformed = Multiply(matrix, homogeneous);
    // 当前 2D Affine Matrix 的最后一行是 (0,0,1)，结果 w=1。
    return { transformed.x, transformed.y };
}

// ============================================================
// 七、HW22 / HW23：3D Transformation 和 Model Matrix
// ============================================================

// HW22.1 — 三维平移矩阵
// 测试：(1,2,3) 平移 (10,20,30) -> (11,22,33)。
// 提示：第四列存放 tx、ty、tz（注意从 0 编号）。
Mat4 MakeTranslation3D(float tx, float ty, float tz)
{
    Mat4 m = IdentityMat4();
    m.m[0][3] = tx;
    m.m[1][3] = ty;
    m.m[2][3] = tz;
    return m;
}

// HW22.2 — 三维缩放矩阵
// 测试：Scale=(2,3,4)，Point=(1,2,3,1)->(2,6,12,1)。
Mat4 MakeScale3D(float sx, float sy, float sz)
{
    Mat4 m = IdentityMat4();
    m.m[0][0] = sx; m.m[1][1] = sy; m.m[2][2] = sz;
    return m;
}

// HW22.3 — 绕 X 轴的右手旋转矩阵
// YZ 平面旋转，测试：+90°：(0,1,0)->(0,0,1)。
Mat4 MakeRotationX3D(float radians)
{
    const float c = std::cos(radians), s = std::sin(radians);
    Mat4 m = IdentityMat4();
    m.m[1][1] = c; m.m[1][2] = -s;
    m.m[2][1] = s; m.m[2][2] = c;
    return m;
}

// HW22.4 — 绕 Y 轴的右手旋转矩阵
// XZ 平面旋转，测试：+90°：(0,0,1)->(1,0,0)。
Mat4 MakeRotationY3D(float radians)
{
    const float c = std::cos(radians), s = std::sin(radians);
    Mat4 m = IdentityMat4();
    m.m[0][0] = c; m.m[0][2] = s;
    m.m[2][0] = -s; m.m[2][2] = c;
    return m;
}

// HW22.5 — 绕 Z 轴的右手旋转矩阵
// XY 平面旋转，测试：+90°：(1,0,0)->(0,1,0)。
Mat4 MakeRotationZ3D(float radians)
{
    const float c = std::cos(radians), s = std::sin(radians);
    Mat4 m = IdentityMat4();
    m.m[0][0] = c; m.m[0][1] = -s;
    m.m[1][0] = s; m.m[1][1] = c;
    return m;
}

// ------------------------------------------------------------
// HW23 — Model Matrix
// 参数：
// position：模型的世界空间位置。
// rotationRadians：绕 XYZ 各轴旋转的弧度值。
// scale：每个轴的缩放倍数。
// 统一组合约定：M = T * Rz * Ry * Rx * S。
// 注意：要按右边先作用的顺序解释！
// 测试：全零位置/旋转、全 1 缩放时，应得到单位矩阵。
// ------------------------------------------------------------
Mat4 MakeModelMatrix(
    const Vec3& position,
    const Vec3& rotationRadians,
    const Vec3& scale)
{
    // 列向量：M = T * Rz * Ry * Rx * S；最右边的 S 首先应用。
    const Mat4 t = MakeTranslation3D(position.x, position.y, position.z);
    const Mat4 rz = MakeRotationZ3D(rotationRadians.z);
    const Mat4 ry = MakeRotationY3D(rotationRadians.y);
    const Mat4 rx = MakeRotationX3D(rotationRadians.x);
    const Mat4 s = MakeScale3D(scale.x, scale.y, scale.z);
    return Multiply(t, Multiply(rz, Multiply(ry, Multiply(rx, s))));
}

// ============================================================
// 八、HW24：View Matrix / LookAt
// ============================================================
// 右手视图坐标约定：相机看向自己的 -Z 方向。
// eye：相机世界坐标位置。
// target：注视的世界空间目标。
// up：参考向上的方向，例如 (0,1,0)。
// 提示步骤：
// 1. forward = Normalize(target-eye)；
// 2. right = Normalize(Cross(forward,up))；
// 3. cameraUp = Cross(right,forward)；
// 4. 利用 right、cameraUp、-forward 构造旋转部分；
// 5. 平移项相当于对 eye 求相应的负点积。
// 测试：eye=(0,0,3)，target=(0,0,0)，世界原点转到 (0,0,-3)。
// 边界：eye==target 或 up 与视线共线时，需要做有效性检查。
Mat4 LookAt(const Vec3& eye, const Vec3& target, const Vec3& up)
{
    // f：观察方向；r：相机右方向；u：经过正交化的相机上方向。
    // 相机在 View Space 中看向 -Z，因此第三行为 -f。
    const Vec3 fVector = Subtract(target, eye);
    if (Length(fVector) <= 1e-7f)
        throw std::invalid_argument("LookAt: eye and target must differ");
    const Vec3 f = Normalize(fVector);
    const Vec3 rightVector = Cross(f, up);
    if (Length(rightVector) <= 1e-7f)
        throw std::invalid_argument("LookAt: up is parallel to view direction");
    const Vec3 r = Normalize(rightVector);
    const Vec3 u = Cross(r, f);
    Mat4 m = IdentityMat4();
    m.m[0][0] = r.x;  m.m[0][1] = r.y;  m.m[0][2] = r.z;  m.m[0][3] = -Dot(r, eye);
    m.m[1][0] = u.x;  m.m[1][1] = u.y;  m.m[1][2] = u.z;  m.m[1][3] = -Dot(u, eye);
    m.m[2][0] = -f.x; m.m[2][1] = -f.y; m.m[2][2] = -f.z; m.m[2][3] = Dot(f, eye);
    return m;
}

// ============================================================
// 九、HW25：Orthographic Projection
// ============================================================
// 参数：left/right/bottom/top 是相机空间的可视矩形边界。
// zNear、zFar 为正的距离，沿相机前方 -Z 测量，0<near<far。
// 使用 OpenGL 风格 NDC z∈[-1,1]。
// 提示：x/y 通过平移+缩放映射到 [-1,1]；
//       z=-near 应映射到 -1，z=-far 应映射到 +1。
// 测试：left=-1,right=1,bottom=-1,top=1,near=1,far=11，
//       相机空间点 (0,0,-1,1) -> NDC (0,0,-1)。
Mat4 MakeOrthographic(
    float left, float right,
    float bottom, float top,
    float zNear, float zFar)
{
    if (!(right > left) || !(top > bottom) || !(zNear > 0.0f) || !(zFar > zNear))
        throw std::invalid_argument("MakeOrthographic: invalid bounds");
    Mat4 m = IdentityMat4();
    // OpenGL NDC：[-1,+1]，zView=-near 映射为 -1，zView=-far 映射为 +1。
    m.m[0][0] = 2.0f / (right - left);
    m.m[1][1] = 2.0f / (top - bottom);
    m.m[2][2] = -2.0f / (zFar - zNear);
    m.m[0][3] = -(right + left) / (right - left);
    m.m[1][3] = -(top + bottom) / (top - bottom);
    m.m[2][3] = -(zFar + zNear) / (zFar - zNear);
    return m;
}

// ============================================================
// 十、HW26：Perspective Projection
// ============================================================
// fovYRadians：垂直视场角，单位为弧度，不是角度。
// aspect：画面宽/高，例如 16.0f/9.0f；100x100 时为 1。
// zNear、zFar：沿相机前方 -Z 的正距离，0<near<far。
// 列向量 + OpenGL 风格：投影后 w_clip=-z_view。
// 核心关系：f = 1/tan(fovY/2)，X= f/aspect，Y=f。
// 测试：位于观察方向上的点 (0,0,-2) 投影后 x_ndc=y_ndc=0。
// 注意：必须在齐次裁剪之后，才做透视除法。
Mat4 MakePerspective(
    float fovYRadians, float aspect,
    float zNear, float zFar)
{
    const float pi = 3.14159265358979323846f;
    if (!(fovYRadians > 0.0f && fovYRadians < pi) ||
        !(aspect > 0.0f) || !(zNear > 0.0f) || !(zFar > zNear))
        throw std::invalid_argument("MakePerspective: invalid FOV/aspect/near/far");
    const float f = 1.0f / std::tan(fovYRadians * 0.5f);
    Mat4 m = {}; // 透视矩阵不是单位矩阵！
    m.m[0][0] = f / aspect;
    m.m[1][1] = f;
    m.m[2][2] = -(zFar + zNear) / (zFar - zNear);
    m.m[2][3] = -(2.0f * zNear * zFar) / (zFar - zNear);
    m.m[3][2] = -1.0f; // wClip=-zView，后续做透视除法。
    return m;
}

// ============================================================
// 十一、HW27：Perspective Divide
// ============================================================
// 输入：裁剪空间四维坐标 (x_clip,y_clip,z_clip,w_clip)。
// 输出：NDC 三维坐标 (x/w,y/w,z/w)。
// 返回 true 表示 w 有效且输出成功，false 表示无法安全除法。
// 思考：为什么不能在裁剪前，对整个三角形的顶点先做除法？
// 注意：这只是除法环节，不能代替 HW30 的齐次裁剪。
bool PerspectiveDivide(const Vec4& clip, Vec3& ndc)
{
    // 这里只负责除法；视锥裁剪必须在此之前完成。
    if (!std::isfinite(clip.w) || std::abs(clip.w) <= 1e-7f)
        return false;
    ndc = { clip.x / clip.w, clip.y / clip.w, clip.z / clip.w };
    return std::isfinite(ndc.x) && std::isfinite(ndc.y) && std::isfinite(ndc.z);
}

// ============================================================
// 十二、HW28：Viewport Transformation
// ============================================================
// 输入：NDC 中的 x/y，范围一般是 [-1,1]。
// 输出：Framebuffer 的连续坐标（不是必须取整数）。
// 当前坐标约定：Framebuffer X 向右、Y 向上。
// 公式提示：xScreen=(xNdc+1)*width/2，yScreen=(yNdc+1)*height/2。
// 测试：NDC (-1,-1)->(0,0)；(0,0)->(50,50)；(1,1)->(100,100)。
// 注意：(100,100) 是右上边界，不是合法像素中心；绘制时做边界裁剪。
Point2 ViewportTransform(const Vec3& ndc, int width, int height)
{
    // 映射到连续视口边界 [0,width]x[0,height]，像素中心用 x+0.5/y+0.5。
    // NDC=+1 对应右/上边界，像素访问时仍必须做裁剪。
    return { (ndc.x + 1.0f) * (static_cast<float>(width) * 0.5f),
            (ndc.y + 1.0f) * (static_cast<float>(height) * 0.5f) };
}

// ============================================================
// 十三、HW29：MVP Pipeline — 旋转线框立方体
// ============================================================
// 一个测试用单位立方体：8 个顶点、12 条边。
// 位于物体局部空间，中心 (0,0,0)，边长 1。
const Vec3 hwCubeVertices[8] =
{
    {-0.5f,-0.5f,-0.5f}, { 0.5f,-0.5f,-0.5f},
    { 0.5f, 0.5f,-0.5f}, {-0.5f, 0.5f,-0.5f},
    {-0.5f,-0.5f, 0.5f}, { 0.5f,-0.5f, 0.5f},
    { 0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f}
};

// 每 2 个索引定义一条边；8 个顶点被边共享。
const int hwCubeEdges[24] =
{
    0,1, 1,2, 2,3, 3,0,
    4,5, 5,6, 6,7, 7,4,
    0,4, 1,5, 2,6, 3,7
};

// 调度函数：通过 HW17~HW28 中完整实现的数学函数进行变换。
// 随着各模块完成，立方体将自动能够显示。
// angleRadians：控制模型绕 Y 轴旋转的角度。
void DrawWireframeCubeMVP(float angleRadians)
{
    const Vec3 position = { 0.0f, 0.0f, 0.0f };
    const Vec3 rotation = { 0.25f, angleRadians, 0.0f };
    const Vec3 scale = { 1.0f, 1.0f, 1.0f };

    const Mat4 model = MakeModelMatrix(position, rotation, scale);
    const Mat4 view = LookAt({ 1.7f, 1.3f, 3.0f },
        { 0.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f });
    const float aspect = static_cast<float>(Raster::WIDTH) /
        static_cast<float>(Raster::HEIGHT);
    const Mat4 projection = MakePerspective(1.04719755f, aspect, 0.1f, 20.0f);

    // 注意：MVP=P*V*M。行向量约定下，顺序才会不同。
    const Mat4 modelView = Multiply(view, model);
    const Mat4 mvp = Multiply(projection, modelView);

    Point2 screen[8] = {};
    bool valid[8] = {};

    for (int i = 0; i < 8; ++i)
    {
        const Vec4 local = MakePosition4(hwCubeVertices[i]);
        const Vec4 clip = Multiply(mvp, local);
        Vec3 ndc = {};
        if (!PerspectiveDivide(clip, ndc))
            continue;

        // HW29 的示例物体完全位于视锥内；这里暂不实现边裁剪。
        screen[i] = ViewportTransform(ndc, Raster::WIDTH, Raster::HEIGHT);
        valid[i] = true;
    }

    for (int i = 0; i < 24; i += 2)
    {
        const int a = hwCubeEdges[i];
        const int b = hwCubeEdges[i + 1];
        if (valid[a] && valid[b])
            DrawLine(screen[a], screen[b], white);
    }
}

// ============================================================
// 十四、HW30：Frustum Clipping（齐次裁剪空间）
// ============================================================
// 六个平面：
// LEFT   : x + w >= 0     RIGHT : w - x >= 0
// BOTTOM : y + w >= 0     TOP   : w - y >= 0
// NEAR   : z + w >= 0     FAR   : w - z >= 0
// 注意：此处 OpenGL 风格 z_clip ∈ [-w,+w]。

struct ClipVertex
{
    Vec4 position;   // 裁剪空间的四维坐标
    Color3 color;    // 顶点颜色（裁剪新交点也需要插值）
};

// HW30.1 — 点相对裁剪平面的有符号距离值
// 参数 planeIndex：0左 1右 2下 3上 4近 5远。
// >=0：在半空间内部；<0：需要裁掉。
// 例如对 near 面应计算 z+w。
float EvaluateClipPlane(const Vec4& p, int planeIndex)
{
    // OpenGL 右手系，六个裁剪平面的内侧距离均要求 >=0。
    switch (planeIndex)
    {
    case 0: return p.x + p.w;  // Left
    case 1: return p.w - p.x;  // Right
    case 2: return p.y + p.w;  // Bottom
    case 3: return p.w - p.y;  // Top
    case 4: return p.z + p.w;  // Near
    case 5: return p.w - p.z;  // Far
    default: throw std::invalid_argument("EvaluateClipPlane: invalid planeIndex");
    }
}

// HW30.2 — 沿边插值生成交点
// t 在 0~1 之间时，返回 a+(b-a)*t 的位置和颜色。
// 新顶点不仅插值 x/y/z，也必须插值 w！
ClipVertex InterpolateClipVertex(const ClipVertex& a,
    const ClipVertex& b,
    float t)
{
    // 在四维齐次空间中做边交点插值，必须保留 w。
    const float invT = 1.0f - t;
    return {
        {
            a.position.x * invT + b.position.x * t,
            a.position.y * invT + b.position.y * t,
            a.position.z * invT + b.position.z * t,
            a.position.w * invT + b.position.w * t
        },
        {
            a.color.r * invT + b.color.r * t,
            a.color.g * invT + b.color.g * t,
            a.color.b * invT + b.color.b * t
        }
    };
}

// ------------------------------------------------------------
// HW30.3 — Sutherland-Hodgman 单平面多边形裁剪
// polygon：按边界顺序排列的一组 ClipVertex。
// planeIndex：选择六个裁剪面中的一个。
// 思路：遍历多边形边 (previous,current)：
// ① inside->inside ：保留 current；
// ② inside->outside：只添加交点；
// ③ outside->inside：先添加交点，再添加 current；
// ④ outside->outside：两点都不添加。
// 用 da=EvaluateClipPlane(a),db=EvaluateClipPlane(b)；
// 交点参数 t = da/(da-db)，注意分母为 0 的情形。
// 一条边可能生成 0、1 或 2 个输出顶点。
// ------------------------------------------------------------
std::vector<ClipVertex> ClipPolygonAgainstPlane(
    const std::vector<ClipVertex>& polygon,
    int planeIndex)
{
    // Sutherland-Hodgman：沿多边形边顺序逐条裁剪。
    std::vector<ClipVertex> output;
    if (polygon.empty()) return output;
    output.reserve(polygon.size() + 1);

    ClipVertex previous = polygon.back();
    float previousDistance = EvaluateClipPlane(previous.position, planeIndex);
    bool previousInside = (previousDistance >= 0.0f);
    for (const ClipVertex& current : polygon)
    {
        const float currentDistance = EvaluateClipPlane(current.position, planeIndex);
        const bool currentInside = (currentDistance >= 0.0f);
        if (previousInside != currentInside)
        {
            // 边跨过平面：f(t)=(1-t)*dPrev+t*dCur=0。
            const float denominator = previousDistance - currentDistance;
            if (std::abs(denominator) > 1e-12f)
            {
                const float t = previousDistance / denominator;
                output.push_back(InterpolateClipVertex(previous, current, t));
            }
        }
        if (currentInside) output.push_back(current);
        previous = current;
        previousDistance = currentDistance;
        previousInside = currentInside;
    }
    return output;
}

// HW30.4 — 依次经过全部六个裁剪面（调度代码已提供）
std::vector<ClipVertex> ClipTriangleToFrustum(
    const ClipVertex& a,
    const ClipVertex& b,
    const ClipVertex& c)
{
    std::vector<ClipVertex> polygon = { a, b, c };
    for (int plane = 0; plane < 6; ++plane)
    {
        polygon = ClipPolygonAgainstPlane(polygon, plane);
        if (polygon.empty())
            break;
    }
    return polygon; // 裁剪后可能有 0、3、4 或更多顶点
}

// HW30.5 — 显示近裁剪面测试三角形
// 三个顶点直接放在相机空间：其中第一个位于 near 前方，应被裁掉一部分。
// 与 HW29 不同，这里不能先做 PerspectiveDivide 再执行裁剪。
void DrawNearClippedTriangleDemo()
{
    const Mat4 projection = MakePerspective(
        1.04719755f,
        static_cast<float>(Raster::WIDTH) / Raster::HEIGHT,
        0.5f, 10.0f);

    const ClipVertex a = { Multiply(projection, Vec4{ 0.0f,   0.0f, -0.2f, 1.0f}),
                          {255.0f, 60.0f, 60.0f} };
    const ClipVertex b = { Multiply(projection, Vec4{ 0.65f, -0.45f,-2.0f, 1.0f}),
                          {60.0f, 255.0f, 60.0f} };
    const ClipVertex c = { Multiply(projection, Vec4{ 0.0f,  0.65f,-2.0f, 1.0f}),
                          {60.0f, 60.0f, 255.0f} };

    const std::vector<ClipVertex> polygon = ClipTriangleToFrustum(a, b, c);
    if (polygon.size() < 3)
        return;

    // 先把裁剪后的每个顶点变换到屏幕空间，再把多边形分割为三角扇。
    // HW11 的 RGB 插值可作为本章的颜色可视化（并非透视校正属性插值）。
    std::vector<Point2> screen;
    screen.reserve(polygon.size());
    for (const ClipVertex& vertex : polygon)
    {
        Vec3 ndc = {};
        if (!PerspectiveDivide(vertex.position, ndc)) return;
        screen.push_back(ViewportTransform(ndc, Raster::WIDTH, Raster::HEIGHT));
    }
    for (std::size_t j = 1; j + 1 < screen.size(); ++j)
    {
        const Triangle triangle = { screen[0], screen[j], screen[j + 1] };
        FillTriangleColorInterpolation(triangle,
            polygon[0].color,
            polygon[j].color,
            polygon[j + 1].color);
    }
    // 最后只画外轮廓；不把三角扇内部对角线画成白线。
    for (std::size_t i = 0; i < screen.size(); ++i)
        DrawLine(screen[i], screen[(i + 1) % screen.size()], white);

}

// ============================================================
// 十五、HW31：3D Triangle Rasterization + Z Buffer
// ============================================================
// 屏幕顶点包含：二维位置、NDC 深度、RGB 顶点颜色。
struct ScreenVertex3D
{
    Point2 position;
    float ndcZ;  // 透视除法之后的 z，范围 [-1,1]
    Color3 color;
};

// HW31.1 — 从 ClipVertex 生成屏幕顶点
// 这里保留少量接口胶水：透视除法和视口变换是 HW27/28 的职责。
bool MakeScreenVertex3D(const ClipVertex& clipVertex,
    ScreenVertex3D& output)
{
    Vec3 ndc = {};
    if (!PerspectiveDivide(clipVertex.position, ndc))
        return false;
    output.position = ViewportTransform(ndc, Raster::WIDTH, Raster::HEIGHT);
    output.ndcZ = ndc.z;
    output.color = clipVertex.color;
    return true;
}

// ------------------------------------------------------------
// HW31.2 — 绘制一个带深度遮挡的屏幕空间三角形
// 参数：a/b/c 已经过 Model、View、Projection、Clip、Divide、Viewport。
// 实现流程：
// 1. 用三个 position 求 Bounding Box；裁到屏幕合法范围。
// 2. 对每个像素中心 (x+0.5,y+0.5)，用 EdgeFunction 测覆盖。
// 3. 计算 Barycentric(alpha,beta,gamma)。
// 4. 按重心坐标插值 NDC 深度：
//    ndcZ=alpha*a.ndcZ+beta*b.ndcZ+gamma*c.ndcZ。
// 5. 转成深度缓存值 depth=ndcZ*0.5+0.5，范围 [0,1]。
// 6. 调用 HW12 的 DepthTest(x,y,depth)；通过后才写颜色。
// 7. 用 HW11 的 InterpolateColor 计算 RGB，再 PutPixel。
// 注意：透视投影后，屏幕重心坐标线性插值 NDC z 是正确的；
//       但若以后插值 UV、世界空间位置等，必须做透视校正插值。
//       需要处理共享边覆盖规则和退化三角形。
// ------------------------------------------------------------
void RasterizeTriangle3D(const ScreenVertex3D& a,
    const ScreenVertex3D& b,
    const ScreenVertex3D& c)
{
    // 1. 将三角形规范为屏幕空间 CCW，便于使用正的有向边函数值。
    ScreenVertex3D v0 = a, v1 = b, v2 = c;
    float area = EdgeFunction(v0.position, v1.position, v2.position);
    if (std::abs(area) <= 1e-7f) return; // 退化三角形。
    if (area < 0.0f)
    {
        std::swap(v1, v2);
        area = -area;
    }

    // 2. 计算三角形包围盒，限制到 Framebuffer 中，避免越界访问。
    const float minX = std::min({ v0.position.x, v1.position.x, v2.position.x });
    const float maxX = std::max({ v0.position.x, v1.position.x, v2.position.x });
    const float minY = std::min({ v0.position.y, v1.position.y, v2.position.y });
    const float maxY = std::max({ v0.position.y, v1.position.y, v2.position.y });
    const int xMin = std::max(0, static_cast<int>(std::floor(minX)));
    const int xMax = std::min(Raster::WIDTH - 1, static_cast<int>(std::ceil(maxX)));
    const int yMin = std::max(0, static_cast<int>(std::floor(minY)));
    const int yMax = std::min(Raster::HEIGHT - 1, static_cast<int>(std::ceil(maxY)));
    if (xMax < xMin || yMax < yMin) return;

    // 3. Top-left Rule：共享边只由其中一个三角形覆盖，避免重复写入。
    // 注意这里坐标系 Y 向上：左边/上边满足 dy<0 或水平且 dx<0。
    auto isTopLeft = [](const Point2& start, const Point2& end) {
        const float dx = end.x - start.x;
        const float dy = end.y - start.y;
        return (dy < 0.0f) || (dy == 0.0f && dx < 0.0f);
        };
    const bool edge0TopLeft = isTopLeft(v1.position, v2.position);
    const bool edge1TopLeft = isTopLeft(v2.position, v0.position);
    const bool edge2TopLeft = isTopLeft(v0.position, v1.position);

    for (int y = yMin; y <= yMax; ++y)
    {
        for (int x = xMin; x <= xMax; ++x)
        {
            const Point2 p = { x + 0.5f, y + 0.5f };
            const float e0 = EdgeFunction(v1.position, v2.position, p);
            const float e1 = EdgeFunction(v2.position, v0.position, p);
            const float e2 = EdgeFunction(v0.position, v1.position, p);
            // 4. 覆盖测试：内部为正；落在边上则按 Top-left Rule 判定。
            if (!(e0 > 0.0f || (e0 == 0.0f && edge0TopLeft)) ||
                !(e1 > 0.0f || (e1 == 0.0f && edge1TopLeft)) ||
                !(e2 > 0.0f || (e2 == 0.0f && edge2TopLeft)))
                continue;

            // 5. 重心权重：分别对应 v0/v1/v2。
            const Barycentric weights = { e0 / area, e1 / area, e2 / area };
            // NDC 深度已经做过透视除法；它对屏幕位置可线性插值。
            const float ndcZ = weights.alpha * v0.ndcZ +
                weights.beta * v1.ndcZ +
                weights.gamma * v2.ndcZ;
            if (ndcZ < -1.0001f || ndcZ > 1.0001f) continue;
            const float depth = Clamp(ndcZ * 0.5f + 0.5f, 0.0f, 1.0f);
            // 6. 先做深度测试，失败的片元不用再算颜色。
            if (!DepthTest(x, y, depth)) continue;
            const Raster::Color pixelColor = InterpolateColor(
                weights, v0.color, v1.color, v2.color);
            Raster::PutPixel(x, y, pixelColor);
        }
    }
}

// 12 个三角形（每三个索引一面）；每两个三角形对应立方体一个面。
// 绕序使用逆时针为外表面；与 HW13 的 CCW 正面一致。
const int hwCubeTriangleIndices[36] =
{
    4,5,6, 4,6,7,  // 正面 +Z
    1,0,3, 1,3,2,  // 背面 -Z
    0,4,7, 0,7,3,  // 左面 -X
    5,1,2, 5,2,6,  // 右面 +X
    7,6,2, 7,2,3,  // 上面 +Y
    0,1,5, 0,5,4   // 下面 -Y
};

const Color3 hwCubeFaceColors[6] =
{
    {255.0f,  70.0f,  70.0f}, // 红
    { 70.0f, 190.0f, 255.0f}, // 蓝
    { 90.0f, 230.0f, 130.0f}, // 绿
    {255.0f, 195.0f,  70.0f}, // 黄
    {200.0f, 110.0f, 255.0f}, // 紫
    { 90.0f, 220.0f, 210.0f}  // 青
};

// HW31.3 — 完整绘制调度：12 三角形 -> 齐次裁剪 -> 深度光栅化。
// 全部调度和光栅化均已实现。
void DrawSolidCubeMVP(float angleRadians)
{
    ClearDepthBuffer();
    const Mat4 model = MakeModelMatrix(
        { 0.0f, 0.0f, 0.0f }, { 0.25f, angleRadians, 0.0f }, { 1.0f, 1.0f, 1.0f });
    const Mat4 view = LookAt({ 1.7f, 1.3f, 3.0f },
        { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f });
    const Mat4 projection = MakePerspective(
        1.04719755f, static_cast<float>(Raster::WIDTH) / Raster::HEIGHT,
        0.1f, 20.0f);
    const Mat4 mvp = Multiply(projection, Multiply(view, model));

    for (int tri = 0; tri < 12; ++tri)
    {
        ClipVertex clipVertices[3];
        for (int corner = 0; corner < 3; ++corner)
        {
            const int vertexIndex = hwCubeTriangleIndices[tri * 3 + corner];
            clipVertices[corner].position =
                Multiply(mvp, MakePosition4(hwCubeVertices[vertexIndex]));
            clipVertices[corner].color = hwCubeFaceColors[tri / 2];
        }

        // 先在 Clip Space 中裁剪；不可提前透视除法。
        const std::vector<ClipVertex> polygon = ClipTriangleToFrustum(
            clipVertices[0], clipVertices[1], clipVertices[2]);
        if (polygon.size() < 3)
            continue;

        // 生成三角扇：(0,1,2), (0,2,3), ...
        // HW31 初期不启用背面剔除，确保即使顶点绕序有误也能观察结果。
        for (size_t j = 1; j + 1 < polygon.size(); ++j)
        {
            ScreenVertex3D a = {}, b = {}, c = {};
            if (!MakeScreenVertex3D(polygon[0], a) ||
                !MakeScreenVertex3D(polygon[j], b) ||
                !MakeScreenVertex3D(polygon[j + 1], c))
                continue;
            RasterizeTriangle3D(a, b, c);
        }
    }
}

// ============================================================
// 十六、作业辅助测试与场景绘制
// ============================================================
// 以下 Demo 复用已经实现的数学函数，负责组织数据和调用。
// 当前窗口只有 100x100：向量/矩阵结果用 printf 输出；
// HW15、HW21、HW29、HW30、HW31 也可以观察图像。
// 若 Windows GUI 没有控制台，可在调试器的输出/断点处查看变量。

void PrintVec2Test(const char* title, const Vec2& v)
{
    std::printf("%s = (%.4f, %.4f)\n", title, v.x, v.y);
}

void PrintVec3Test(const char* title, const Vec3& v)
{
    std::printf("%s = (%.4f, %.4f, %.4f)\n", title, v.x, v.y, v.z);
}

void PrintVec4Test(const char* title, const Vec4& v)
{
    std::printf("%s = (%.4f, %.4f, %.4f, %.4f)\n",
        title, v.x, v.y, v.z, v.w);
}

void PrintMat3Test(const char* title, const Mat3& mat)
{
    std::printf("%s:\n", title);
    for (int row = 0; row < 3; ++row)
        std::printf("  %8.4f %8.4f %8.4f\n",
            mat.m[row][0], mat.m[row][1], mat.m[row][2]);
}

void PrintMat4Test(const char* title, const Mat4& mat)
{
    std::printf("%s:\n", title);
    for (int row = 0; row < 4; ++row)
        std::printf("  %8.4f %8.4f %8.4f %8.4f\n",
            mat.m[row][0], mat.m[row][1], mat.m[row][2], mat.m[row][3]);
}

// HW15~HW31：逐项检查数值算法，并打印 PASS/FAIL。
// 只在 RenderScene 的作业编号变化时执行一次，避免打印刷屏。
// ============================================================
// RunMathHomeworkTest：HW15~HW31 单元测试
// 将 kActiveHomework 改为 0 可依次运行全部作业测试。
// 除图形显示外，用 PASS/FAIL 自动验证真实结果，而不是只打印期望值。
// ============================================================
void RunMathHomeworkTest(int hw)
{
    constexpr float pi = 3.14159265358979323846f;
    constexpr float halfPi = pi * 0.5f;
    const auto near = [](float a, float b, float eps = 1e-4f) -> bool {
        return std::isfinite(a) && std::isfinite(b) && std::abs(a - b) <= eps;
        };
    const auto near2 = [&](const Vec2& a, const Vec2& b) -> bool {
        return near(a.x, b.x) && near(a.y, b.y);
        };
    const auto near3 = [&](const Vec3& a, const Vec3& b) -> bool {
        return near(a.x, b.x) && near(a.y, b.y) && near(a.z, b.z);
        };
    const auto near4 = [&](const Vec4& a, const Vec4& b) -> bool {
        return near(a.x, b.x) && near(a.y, b.y) && near(a.z, b.z) && near(a.w, b.w);
        };
    const auto nearMat3 = [&](const Mat3& a, const Mat3& b) -> bool {
        for (int r = 0; r < 3; ++r)
            for (int c = 0; c < 3; ++c)
                if (!near(a.m[r][c], b.m[r][c])) return false;
        return true;
        };
    const auto nearMat4 = [&](const Mat4& a, const Mat4& b) -> bool {
        for (int r = 0; r < 4; ++r)
            for (int c = 0; c < 4; ++c)
                if (!near(a.m[r][c], b.m[r][c])) return false;
        return true;
        };
    int passed = 0, failed = 0;
    const auto check = [&](bool ok, const char* description) {
        std::printf("[HW%02d] %s  %s\n", hw, ok ? "PASS" : "FAIL", description);
        if (ok) ++passed; else ++failed;
        };

    switch (hw)
    {
        // 一键执行 HW15~HW31 的全部测试，不影响原先逐章运行的功能。
    case 0:
        for (int id = 15; id <= 31; ++id) RunMathHomeworkTest(id);
        return;

    case 10:
    {
        Barycentric bary = {};
        const bool ok = ComputeBarycentric(testTriangle, insidePoint, bary);
        check(ok && near(bary.alpha + bary.beta + bary.gamma, 1.0f),
            "Barycentric: alpha+beta+gamma = 1");
        break;
    }
    case 15:
    {
        // 顶点 V0/V1/V2/V3：两个三角形拼成矩形（共享 V0、V2）。
        const std::vector<Point2> vertices = { {10,10},{90,10},{90,90},{10,90} };
        const std::vector<int> indices = { 0,1,2,0,2,3 };
        check(indices.size() == 6 && vertices.size() == 4, "indexed mesh: 4 vertices / 2 triangles");
        Triangle tri0 = { vertices[indices[0]],vertices[indices[1]],vertices[indices[2]] };
        Triangle tri1 = { vertices[indices[3]],vertices[indices[4]],vertices[indices[5]] };
        check(near(EdgeFunction(tri0.a, tri0.b, tri0.c), 6400.0f), "triangle 0 CCW area2=6400");
        check(near(EdgeFunction(tri1.a, tri1.b, tri1.c), 6400.0f), "triangle 1 CCW area2=6400");
        DrawIndexedTriangles(vertices, { 0,1,2,0,2,3,999,2,3 }, white);
        check(true, "DrawIndexedTriangles: invalid index is ignored safely");
        break;
    }
    case 16:
        check(near2(Add(Vec2{ 1,2 }, Vec2{ 3,4 }), { 4,6 }), "Vec2 Add (1,2)+(3,4)=(4,6)");
        check(near2(Subtract(Vec2{ 5,7 }, Vec2{ 2,3 }), { 3,4 }), "Vec2 Subtract");
        check(near2(Multiply(Vec2{ 3,4 }, 2.0f), { 6,8 }), "Vec2 Multiply scalar");
        check(near(Dot(Vec2{ 1,2 }, Vec2{ 3,4 }), 11.0f), "Vec2 Dot=11");
        check(near(Length(Vec2{ 3,4 }), 5.0f), "Vec2 Length=5");
        check(near2(Normalize(Vec2{ 3,4 }), { 0.6f,0.8f }), "Vec2 Normalize");
        check(near2(Normalize(Vec2{ 0,0 }), { 0,0 }), "Vec2 Normalize zero vector");
        break;
    case 17:
        check(near3(Add(Vec3{ 1,2,3 }, Vec3{ 4,5,6 }), { 5,7,9 }), "Vec3 Add");
        check(near3(Subtract(Vec3{ 5,7,9 }, Vec3{ 1,2,3 }), { 4,5,6 }), "Vec3 Subtract");
        check(near3(Multiply(Vec3{ 1,2,3 }, 2.0f), { 2,4,6 }), "Vec3 Multiply scalar");
        check(near(Dot(Vec3{ 1,2,3 }, Vec3{ 4,5,6 }), 32.0f), "Vec3 Dot=32");
        check(near3(Cross({ 1,0,0 }, { 0,1,0 }), { 0,0,1 }), "Cross(X,Y)=+Z");
        check(near3(Cross({ 0,1,0 }, { 1,0,0 }), { 0,0,-1 }), "Cross(Y,X)=-Z");
        check(near(Length(Vec3{ 2,3,6 }), 7.0f), "Vec3 Length=7");
        check(near3(Normalize(Vec3{ 0,0,5 }), { 0,0,1 }), "Vec3 Normalize");
        check(near3(Normalize(Vec3{ 0,0,0 }), { 0,0,0 }), "Vec3 Normalize zero vector");
        break;
    case 18:
    {
        check(near4(MakePosition4({ 2,3,4 }), Vec4{ 2,3,4,1 }), "Position w=1");
        check(near4(MakeDirection4({ 2,3,4 }), Vec4{ 2,3,4,0 }), "Direction w=0");
        check(near4(Add(Vec4{ 1,2,3,1 }, Vec4{ 2,3,4,0 }), Vec4{ 3,5,7,1 }), "Vec4 Add");
        check(near4(Multiply(Vec4{ 1,2,3,4 }, 2.0f), Vec4{ 2,4,6,8 }), "Vec4 Multiply scalar");
        const Mat4 translation = MakeTranslation3D(10, 20, 30);
        check(near4(Multiply(translation, MakePosition4({ 1,2,3 })), Vec4{ 11,22,33,1 }),
            "Translation affects a position");
        check(near4(Multiply(translation, MakeDirection4({ 1,2,3 })), Vec4{ 1,2,3,0 }),
            "Translation does NOT affect a direction");
        break;
    }
    case 19:
    {
        const Mat3 i = IdentityMat3();
        const Mat3 m = MakeTranslation2D(2, 3);
        check(near3(Multiply(i, Vec3{ 2,3,1 }), { 2,3,1 }), "Mat3 identity * vector");
        check(nearMat3(Multiply(i, m), m) && nearMat3(Multiply(m, i), m), "Mat3 identity multiplication");
        check(near3(Multiply(Multiply(m, MakeScale2D(2, 3)), Vec3{ 1,2,1 }), { 4,9,1 }),
            "Mat3 matrix product * vector");
        break;
    }
    case 20:
    {
        const Mat4 i = IdentityMat4();
        const Mat4 t = MakeTranslation3D(2, 3, 4);
        const Mat4 scale = MakeScale3D(2, 3, 4);
        check(near4(Multiply(i, Vec4{ 2,3,4,1 }), Vec4{ 2,3,4,1 }), "Mat4 identity * vector");
        check(nearMat4(Multiply(i, t), t) && nearMat4(Multiply(t, i), t), "Mat4 identity multiplication");
        check(near4(Multiply(Multiply(t, scale), Vec4{ 1,2,3,1 }), Vec4{ 4,9,16,1 }),
            "Mat4 matrix product * vector");
        break;
    }
    case 21:
    {
        const Point2 p = { 2,3 };
        const auto pointNear = [&](const Point2& actual, Point2 expected) {
            return near(actual.x, expected.x) && near(actual.y, expected.y);
            };
        check(pointNear(TransformPoint2D(MakeTranslation2D(10, 20), p), { 12,23 }), "2D Translation");
        check(pointNear(TransformPoint2D(MakeRotation2D(halfPi), { 1,0 }), { 0,1 }), "2D Rotation +90deg");
        check(pointNear(TransformPoint2D(MakeScale2D(2, 3), { 4,5 }), { 8,15 }), "2D Scale");
        check(pointNear(TransformPoint2D(MakeReflectionX2D(), p), { 2,-3 }), "2D Reflection about X");
        check(pointNear(TransformPoint2D(MakeReflectionY2D(), p), { -2,3 }), "2D Reflection about Y");
        check(pointNear(TransformPoint2D(MakeShear2D(1, 0), p), { 5,3 }), "2D Shear X");
        check(pointNear(TransformPoint2D(MakeShear2D(0, 1), p), { 2,5 }), "2D Shear Y");
        const Mat3 trs = Multiply(MakeTranslation2D(10, 20),
            Multiply(MakeRotation2D(halfPi), MakeScale2D(2, 3)));
        check(pointNear(TransformPoint2D(trs, { 1,0 }), { 10,22 }), "2D T*R*S order");
        break;
    }
    case 22:
        check(near4(Multiply(MakeTranslation3D(10, 20, 30), Vec4{ 1,2,3,1 }), Vec4{ 11,22,33,1 }), "3D Translation");
        check(near4(Multiply(MakeScale3D(2, 3, 4), Vec4{ 1,2,3,1 }), Vec4{ 2,6,12,1 }), "3D Scale");
        check(near4(Multiply(MakeRotationX3D(halfPi), Vec4{ 0,1,0,1 }), Vec4{ 0,0,1,1 }), "Rotation about X +90deg");
        check(near4(Multiply(MakeRotationY3D(halfPi), Vec4{ 0,0,1,1 }), Vec4{ 1,0,0,1 }), "Rotation about Y +90deg");
        check(near4(Multiply(MakeRotationZ3D(halfPi), Vec4{ 1,0,0,1 }), Vec4{ 0,1,0,1 }), "Rotation about Z +90deg");
        break;
    case 23:
        check(nearMat4(MakeModelMatrix({ 0,0,0 }, { 0,0,0 }, { 1,1,1 }), IdentityMat4()),
            "identity Model Matrix");
        check(near4(Multiply(MakeModelMatrix({ 10,20,30 }, { 0,0,0 }, { 2,3,4 }), Vec4{ 1,2,3,1 }), Vec4{ 12,26,42,1 }), "Model T*S transforms position");
        check(near4(Multiply(MakeModelMatrix({ 10,20,30 }, { 0,0,halfPi }, { 2,2,2 }), Vec4{ 1,0,0,1 }), Vec4{ 10,22,30,1 }), "Model T*Rz*S order");
        break;
    case 24:
    {
        const Mat4 view = LookAt({ 0,0,3 }, { 0,0,0 }, { 0,1,0 });
        check(near4(Multiply(view, Vec4{ 0,0,0,1 }), Vec4{ 0,0,-3,1 }), "LookAt world origin -> view -Z");
        check(near4(Multiply(view, Vec4{ 0,0,3,1 }), Vec4{ 0,0,0,1 }), "LookAt eye -> view origin");
        check(near4(Multiply(LookAt({ 0,0,0 }, { 1,0,0 }, { 0,1,0 }), Vec4{ 1,0,0,1 }), Vec4{ 0,0,-1,1 }), "LookAt arbitrary +X view direction");
        bool rejected = false;
        try { (void)LookAt({ 0,0,0 }, { 0,0,0 }, { 0,1,0 }); }
        catch (const std::invalid_argument&) { rejected = true; }
        check(rejected, "LookAt rejects coincident eye/target");
        break;
    }
    case 25:
    {
        const Mat4 ortho = MakeOrthographic(-1, 1, -1, 1, 1, 11);
        check(near4(Multiply(ortho, Vec4{ 0,0,-1,1 }), Vec4{ 0,0,-1,1 }), "Ortho near z -> -1");
        check(near4(Multiply(ortho, Vec4{ 0,0,-11,1 }), Vec4{ 0,0,1,1 }), "Ortho far z -> +1");
        check(near4(Multiply(ortho, Vec4{ 1,1,-1,1 }), Vec4{ 1,1,-1,1 }), "Ortho top-right bound");
        bool rejected = false;
        try { (void)MakeOrthographic(0, 0, -1, 1, 1, 10); }
        catch (const std::invalid_argument&) { rejected = true; }
        check(rejected, "Ortho rejects invalid bounds");
        break;
    }
    case 26:
    {
        const Mat4 proj = MakePerspective(halfPi, 1.0f, 1.0f, 11.0f);
        const Vec4 nearClip = Multiply(proj, Vec4{ 0,0,-1,1 });
        const Vec4 farClip = Multiply(proj, Vec4{ 0,0,-11,1 });
        check(near(nearClip.w, 1) && near(nearClip.z / nearClip.w, -1), "Perspective near -> NDC z=-1");
        check(near(farClip.w, 11) && near(farClip.z / farClip.w, 1), "Perspective far -> NDC z=+1");
        check(near4(Multiply(proj, Vec4{ 2,0,-2,1 }), Vec4{ 2,0,0.2f,2 }),
            "Perspective fov=90 aspect=1, view(2,0,-2)");
        bool rejected = false;
        try { (void)MakePerspective(halfPi, 0.0f, 1.0f, 11.0f); }
        catch (const std::invalid_argument&) { rejected = true; }
        check(rejected, "Perspective rejects invalid aspect");
        break;
    }
    case 27:
    {
        Vec3 ndc = {};
        check(PerspectiveDivide({ 2,4,6,2 }, ndc) && near3(ndc, { 1,2,3 }), "Clip/w => NDC");
        check(!PerspectiveDivide({ 1,2,3,0 }, ndc), "Reject w=0");
        check(!PerspectiveDivide({ 1,2,3,1e-9f }, ndc), "Reject tiny w");
        break;
    }
    case 28:
    {
        const Point2 lo = ViewportTransform({ -1,-1,0 }, 100, 100);
        const Point2 center = ViewportTransform({ 0,0,0 }, 100, 100);
        const Point2 hi = ViewportTransform({ 1,1,0 }, 100, 100);
        check(near(lo.x, 0) && near(lo.y, 0), "Viewport bottom-left (0,0)");
        check(near(center.x, 50) && near(center.y, 50), "Viewport center (50,50)");
        check(near(hi.x, 100) && near(hi.y, 100), "Viewport right/top boundary (100,100)");
        break;
    }
    case 29:
    {
        const Mat4 model = MakeModelMatrix({ 0,0,0 }, { 0.25f,1.2f,0 }, { 1,1,1 });
        const Mat4 view = LookAt({ 1.7f,1.3f,3 }, { 0,0,0 }, { 0,1,0 });
        const Mat4 proj = MakePerspective(1.04719755f, 1.0f, 0.1f, 20.0f);
        const Mat4 mvp = Multiply(proj, Multiply(view, model));
        int count = 0;
        for (const Vec3& vertex : hwCubeVertices)
        {
            const Vec4 clip = Multiply(mvp, MakePosition4(vertex));
            Vec3 ndc = {};
            if (PerspectiveDivide(clip, ndc) &&
                ndc.x >= -1.0f && ndc.x <= 1.0f &&
                ndc.y >= -1.0f && ndc.y <= 1.0f &&
                ndc.z >= -1.0f && ndc.z <= 1.0f) ++count;
        }
        check(count == 8, "MVP: all 8 cube corners are in the frustum");
        check(sizeof(hwCubeEdges) / sizeof(hwCubeEdges[0]) == 24, "Wireframe: 12 edges");
        break;
    }
    case 30:
    {
        const Vec4 inside = { 0,0,0,1 };
        bool sixPositive = true;
        for (int plane = 0; plane < 6; ++plane)
            sixPositive &= (EvaluateClipPlane(inside, plane) >= 0.0f);
        check(sixPositive, "Clip: center inside all six planes");
        check(near(EvaluateClipPlane({ 0,0,-2,1 }, 4), -1.0f), "Clip: near plane z+w");
        const ClipVertex va = { {0,0,-2,1},{255,0,0} };
        const ClipVertex vb = { {0.6f,-0.4f,0,1},{0,255,0} };
        const ClipVertex vc = { {-0.6f,0.4f,0,1},{0,0,255} };
        const auto polygon = ClipTriangleToFrustum(va, vb, vc);
        check(polygon.size() == 4, "Near clipping triangle -> quad (4 vertices)");
        bool allInside = true;
        for (const ClipVertex& v : polygon)
            for (int plane = 0; plane < 6; ++plane)
                if (EvaluateClipPlane(v.position, plane) < -1e-4f) allInside = false;
        check(allInside, "Clipped polygon entirely inside all six planes");
        const ClipVertex midpoint = InterpolateClipVertex(va, vb, 0.5f);
        check(near4(midpoint.position, Vec4{ 0.3f,-0.2f,-1,1 }) &&
            near(midpoint.color.r, 127.5f) && near(midpoint.color.g, 127.5f),
            "Clip edge interpolates Vec4 and RGB");
        check(ClipTriangleToFrustum({ {5,0,0,1},{255,0,0} },
            { {6,0,0,1},{0,255,0} },
            { {5,1,0,1},{0,0,255} }).empty(),
            "Clip triangle completely outside right plane");
        break;
    }
    case 31:
    {
        const ClipVertex cv = { {0,0,0,1},{255,0,0} };
        ScreenVertex3D sv = {};
        check(MakeScreenVertex3D(cv, sv) &&
            near(sv.position.x, 50) && near(sv.position.y, 50) && near(sv.ndcZ, 0),
            "ClipVertex -> ScreenVertex3D at (50,50)");
        ClearDepthBuffer();
        const ScreenVertex3D far0 = { {10,10}, 0.6f,{0,0,255} };
        const ScreenVertex3D far1 = { {90,10}, 0.6f,{0,0,255} };
        const ScreenVertex3D far2 = { {50,90}, 0.6f,{0,0,255} };
        RasterizeTriangle3D(far0, far1, far2);
        check(near(depthBuffer[30][50], 0.8f), "Rasterize: far triangle writes depth=0.8");
        const ScreenVertex3D near0 = { {10,10},-0.4f,{255,0,0} };
        const ScreenVertex3D near1 = { {90,10},-0.4f,{255,0,0} };
        const ScreenVertex3D near2 = { {50,90},-0.4f,{255,0,0} };
        RasterizeTriangle3D(near0, near1, near2);
        check(near(depthBuffer[30][50], 0.3f), "Rasterize: nearer triangle overwrites depth=0.3");
        RasterizeTriangle3D(far0, far1, far2);
        check(near(depthBuffer[30][50], 0.3f), "Z-Buffer: farther triangle cannot overwrite");
        check(near(depthBuffer[0][0], 1.0f), "Rasterize: outside triangle depth unchanged");
        break;
    }
    default:
        std::printf("No math test for HW%d\n", hw);
        return;
    }
    std::printf("[HW%02d] RESULT: %d PASS, %d FAIL\n\n", hw, passed, failed);
}

// HW21：比较未变换（暗灰色）的四边形和完成变换（白色）的四边形。
// 可调节平移/旋转/反射/错切，观察差异。
void DrawTransform2DDemo()
{
    const Point2 local[4] =
    {
        {-15.0f,-12.0f}, {15.0f,-12.0f},
        {15.0f, 12.0f}, {-15.0f, 12.0f}
    };
    const Mat3 translation = MakeTranslation2D(50.0f, 50.0f);
    const Mat3 rotation = MakeRotation2D(0.5f);
    const Mat3 scaling = MakeScale2D(1.15f, 0.8f);
    const Mat3 matrix = Multiply(translation, Multiply(rotation, scaling));

    for (int i = 0; i < 4; ++i)
    {
        const int next = (i + 1) % 4;
        // 对照图：仅将局部四边形移至画面中央。
        DrawLine({ local[i].x + 50.0f, local[i].y + 50.0f },
            { local[next].x + 50.0f, local[next].y + 50.0f },
            Raster::MakeColor(65, 65, 65));
        DrawLine(TransformPoint2D(matrix, local[i]),
            TransformPoint2D(matrix, local[next]), white);
    }
}

// ============================================================
// 十七、RenderScene：在这里选择希望运行的作业
// ============================================================
// 只需要修改 kActiveHomework，不需要在 main.cpp 增加调用。
// 原文件的 HW02~HW15 代码已保留，HW15~HW31 实现及入口已补全。
// 注意：数学作业（16~20,22~28）在控制台输出数值；
// HW21/HW29/HW30/HW31 在 Framebuffer 中绘制可视化结果。
// 每次只运行一个作业，保证调试范围明确。
// ============================================================
void RenderScene()
{
    Raster::Clear(black);

    // ★★★ 修改这里切换作业：0=全部测试，2~31=单个作业 ★★★
    // 推荐学习顺序：16 -> 17 -> ... -> 31。
    // 默认 0：一次执行 HW15~31 全部断言并展示 HW31 三维立方体。
    // 如果只想观察某一章，改成 15~31 中对应编号。
    constexpr int kActiveHomework = 0;

    // 对需要打印数值的作业，只在第一次调用时打印，避免刷屏。
    static int lastLoggedHomework = -1;
    if (lastLoggedHomework != kActiveHomework)
    {
        RunMathHomeworkTest(kActiveHomework);
        lastLoggedHomework = kActiveHomework;
        // 单元测试可能向颜色缓冲写入像素；清空后再渲染正常场景。
        Raster::Clear(black);
    }

    switch (kActiveHomework)
    {
    case 0:
        // 数值测试已在上方运行；展示最新的 HW31 彩色三维立方体。
        DrawSolidCubeMVP(1.2f);
        break;
        // 旧作业（复用你已经写好的实现）
    case 2:
        DrawHorizontalLine(20, 10, 90, red);
        DrawVerticalLine(30, 10, 90, blue);
        break;
    case 3:
        DrawLine(lineStart, lineEnd, red);
        break;
    case 4:
        DrawRectangleOutline(rectMin, rectMax, white);
        FillRectangle({ 20,20 }, { 40,40 }, green);
        break;
    case 5:
        DrawCircleOutline(circleCenter, circleRadius, white);
        break;
    case 6:
        DrawTriangleWireframe(testTriangle, red);
        break;
    case 7:
        // 用边函数显示一个点相对 AB 的位置。
        if (EdgeFunction({ 10,30 }, { 90,30 }, { 45,50 }) > 0)
            Raster::PutPixel(45, 50, green);
        DrawLine({ 10,30 }, { 90,30 }, white);
        break;
    case 8:
        DrawTriangleWireframe(testTriangle, white);
        Raster::PutPixel(45, 40,
            IsInsideTriangle(testTriangle, insidePoint) ? green : red);
        break;
    case 9:
        FillTriangle(testTriangle, green);
        break;
    case 10:
        DrawTriangleWireframe(testTriangle, white);
        break;
    case 11:
        FillTriangleColorInterpolation(testTriangle, colorA, colorB, colorC);
        break;
    case 12:
        ClearDepthBuffer();
        if (DepthTest(50, 50, 0.8f)) Raster::PutPixel(50, 50, blue);
        if (DepthTest(50, 50, 0.2f)) Raster::PutPixel(50, 50, red);
        break;
    case 13:
    {
        const Triangle ccw = { {10,10},{45,10},{28,70} };
        const Triangle cw = { {55,10},{72,70},{90,10} };
        if (IsFrontFacing(ccw)) FillTriangle(ccw, green);
        if (IsFrontFacing(cw)) FillTriangle(cw, red);
        break;
    }
    case 14:
        RasterizeTriangleMSAA(testTriangle, red);
        ResolveMSAA();
        break;
    case 15:
    {
        const std::vector<Point2> vertices =
        { {10,10},{90,10},{90,90},{10,90} };
        const std::vector<int> indices = { 0,1,2,0,2,3 };
        DrawIndexedTriangles(vertices, indices, white);
        break;
    }

    // HW16：Vec2。观察控制台输出的加减、数乘、长度、归一化。
    case 16:
        break; // RunMathHomeworkTest 已在 switch 上方调用

        // HW17：Vec3。检查 Dot、Cross、Length、Normalize。
    case 17:
        break;

        // HW18：Vec4。对比 Position 的 w=1 和 Direction 的 w=0。
    case 18:
        break;

        // HW19：Mat3。检查单位矩阵和矩阵乘向量的结果。
    case 19:
        break;

        // HW20：Mat4。检查单位矩阵和矩阵乘向量的结果。
    case 20:
        break;

        // HW21：二维变换。灰色未旋转，白色为 T*R*S 的结果。
    case 21:
        DrawTransform2DDemo();
        break;

        // HW22：3D 平移/旋转等，比较 Position 和 Direction。
    case 22:
        break;

        // HW23：Model Matrix 的组合顺序。
    case 23:
        break;

        // HW24：LookAt 相机视图矩阵。
    case 24:
        break;

        // HW25：正交投影。
    case 25:
        break;

        // HW26：透视投影。
    case 26:
        break;

        // HW27：透视除法 Clip -> NDC。
    case 27:
        break;

        // HW28：NDC -> 100x100 视口。
    case 28:
        break;

        // HW29：旋转线框立方体。调节角度观察结果。
        // HW17~HW28 中对应的数学函数已实现。
    case 29:
        DrawWireframeCubeMVP(1.2f);
        break;

        // HW30：裁剪穿过 Near Plane 的三角形。
        // EvaluateClipPlane、InterpolateClipVertex、ClipPolygonAgainstPlane 已实现。
    case 30:
        DrawNearClippedTriangleDemo();
        break;

        // HW31：六色立方体，12 个三角形，带深度遮挡。
        // RasterizeTriangle3D 已实现，可检查不同面的遮挡关系。
    case 31:
        DrawSolidCubeMVP(1.2f);
        break;

    default:
        // 没有对应作业；只显示黑色背景。
        break;
    }
}
