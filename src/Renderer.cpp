
#include "RasterWindow.h"

#include <cmath>
#include <vector>
#include <cstdint>

// ============================================================
// JGraphicsLab
// HW02 ~ HW16：计算机图形学基础作业
//
// 当前 Framebuffer：100 × 100
// X 坐标：向右增加
// Y 坐标：向上增加
// 左下角：(0, 0)
// 右上角：(99, 99)
//
// 学习规则：
// 1. 所有 TODO 都由自己完成。
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
        Raster::PutPixel(xStart,y,  color);
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
    // TODO：自己实现

    // 距离 
    float dx = std::abs( end.x - start.x ) ;
    float dy = std::abs( end.y - start.y ) ;
    
    int sx = (start.x < end.x)?1:-1;
    int sy = (start.y < end.y) ? 1 : -1;

    // 误差,误差用于判断下一步应该沿哪个坐标轴移动
    int err  = dx - dy;

    // panduan 走x还是走y/
    while (1)
    {
        Raster::PutPixel(start.x, start.y, color);
        if (start.x == end.x && start.y == end.y)
        {
            break;
        }
        
        int e2 = 2 * err;
        // 6. 判断是否需要沿 X 轴移动
        if (e2 > -dy)
        {
            err -= dy;
            start.x += sx;
        }

        // 7. 判断是否需要沿 Y 轴移动
        if (e2 < dx)
        {
            err += dx;
            start.y += sy;
        }
    }



    // 加一个 基于直线方程的直接绘制方法，与 DDA 算法也有联系
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

bool inCircleOutline(int i,int j , 
    Point2 center,float radius)
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
            if (inCircleOutline(i, j, center, radius) )
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
    DrawLine(triangle.a , triangle.b,color);
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
    return 0.0f;
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
    // TODO：自己实现
    return false;
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
    // TODO：自己实现
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
    return false;
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
    // TODO：自己实现
    return black;
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
    // TODO：自己实现
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
bool DepthTest(int x, int y, float depth)
{
    // TODO：自己实现
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
bool IsFrontFacing(const Triangle& triangle)
{
    // TODO：自己实现
    return false;
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
    // TODO：自己实现
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
    // TODO：自己实现
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
    // TODO：自己实现
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
    // TODO：自己实现
    return {};
}


// 功能：
// 计算两个向量之差。
//
// 测试：
// (5,7)-(2,3)=(3,4)
Vec2 Subtract(const Vec2& a, const Vec2& b)
{
    // TODO：自己实现
    return {};
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
    // TODO：自己实现
    return 0.0f;
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
    // TODO：自己实现
    return 0.0f;
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
    // TODO：自己实现
    return {};
}


// ============================================================
// 三、RenderScene
//
// 这是你平时唯一需要主动调用作业函数的地方。
//
// 规则：
// 1. 清空画布。
// 2. 选择一个作业。
// 3. 调用对应函数。
// 4. 观察运行结果。
// ============================================================

void RenderScene()
{
    // 首先清空为黑色。
    Raster::Clear(black);


    // ========================================================
    // HW02：水平线与垂直线
    // ========================================================

    // 在 Y=20 绘制一条从 X=10 到 X=90 的红线。
   //   DrawHorizontalLine(20, 10, 90, red);

    // 在 X=30 绘制一条从 Y=10 到 Y=90 的蓝线。
    //  DrawVerticalLine(30, 10, 90, blue);

     

    // ========================================================
    // HW03：任意直线
    // ========================================================

    //  DrawLine(lineStart, lineEnd, red);


    // ========================================================
    // HW04：矩形
    // ========================================================

    // DrawRectangleOutline(rectMin, rectMax, white);
    // FillRectangle(rectMin, rectMax, green);


    // ========================================================
    // HW05：圆形
    // ========================================================
    
    // DrawCircleOutline(circleCenter, circleRadius, white);
    // FillCircle(circleCenter, circleRadius, green);


    // ========================================================
    // HW06：三角形线框
    // ========================================================
     // main 

    // question 为什么用 draw line1就画不出来呢？
    DrawTriangleWireframe(testTriangle, red);


    // ========================================================
    // HW08：三角形内部判断
    // ========================================================

    // bool inside = IsInsideTriangle(
    //     testTriangle, insidePoint);


    // ========================================================
    // HW09：实心三角形
    // ========================================================

    // FillTriangle(testTriangle, green);


    // ========================================================
    // HW10：重心坐标
    // ========================================================

    // Barycentric result = {};
    // bool success = ComputeBarycentric(
    //     testTriangle, insidePoint, result);


    // ========================================================
    // 后续 HW11~HW16：
    //
    // 根据对应函数注释，自行添加测试。
    // 纯数学计算函数可以使用断点、assert
    // 或控制台测试程序检查计算结果。
    // ========================================================
}
