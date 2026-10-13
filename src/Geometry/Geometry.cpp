#include "Geometry/Geometry.h"
#include "Renderer/Rasterizer2D.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <stdexcept>
#include <limits>

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
