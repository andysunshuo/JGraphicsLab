#include "Homework/DemoData.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <stdexcept>
#include <limits>

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
const Point2 lineStart = {10.0f, 15.0f};

const Point2 lineEnd = {85.0f, 75.0f};

// HW04：矩形的两个对角顶点
//
// rectMin：左下角
// rectMax：右上角
//
// 目标：绘制一个矩形。
const Point2 rectMin = {15.0f, 15.0f};

const Point2 rectMax = {65.0f, 55.0f};

// HW05：圆心和半径
//
// 圆心：(50,50)
// 半径：20
const Point2 circleCenter = {50.0f, 50.0f};

constexpr float circleRadius = 20.0f;

// HW06~HW14：测试三角形
//
// A = (10,10)
// B = (85,15)
// C = (45,90)
const Triangle testTriangle = {{10.0f, 10.0f}, {85.0f, 15.0f}, {45.0f, 90.0f}};

// HW08：用于判断三角形内部的测试点
const Point2 insidePoint = {45.0f, 40.0f};

const Point2 outsidePoint = {90.0f, 90.0f};

// HW11：三个顶点的颜色
//
// A：红色
// B：绿色
// C：蓝色
const Color3 colorA = {255.0f, 0.0f, 0.0f};

const Color3 colorB = {0.0f, 255.0f, 0.0f};

const Color3 colorC = {0.0f, 0.0f, 255.0f};

// ============================================================
// 十三、HW29：MVP Pipeline — 旋转线框立方体
// ============================================================
// 一个测试用单位立方体：8 个顶点、12 条边。
// 位于物体局部空间，中心 (0,0,0)，边长 1。
const Vec3 hwCubeVertices[8] = {{-0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, -0.5f},
                                {-0.5f, 0.5f, -0.5f},  {-0.5f, -0.5f, 0.5f}, {0.5f, -0.5f, 0.5f},
                                {0.5f, 0.5f, 0.5f},    {-0.5f, 0.5f, 0.5f}};

// 每 2 个索引定义一条边；8 个顶点被边共享。
const int hwCubeEdges[24] = {0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6, 6, 7, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7};

// 12 个三角形（每三个索引一面）；每两个三角形对应立方体一个面。
// 绕序使用逆时针为外表面；与 HW13 的 CCW 正面一致。
const int hwCubeTriangleIndices[36] = {
    4, 5, 6, 4, 6, 7, // 正面 +Z
    1, 0, 3, 1, 3, 2, // 背面 -Z
    0, 4, 7, 0, 7, 3, // 左面 -X
    5, 1, 2, 5, 2, 6, // 右面 +X
    7, 6, 2, 7, 2, 3, // 上面 +Y
    0, 1, 5, 0, 5, 4  // 下面 -Y
};

const Color3 hwCubeFaceColors[6] = {
    {255.0f, 70.0f, 70.0f},   // 红
    {70.0f, 190.0f, 255.0f},  // 蓝
    {90.0f, 230.0f, 130.0f},  // 绿
    {255.0f, 195.0f, 70.0f},  // 黄
    {200.0f, 110.0f, 255.0f}, // 紫
    {90.0f, 220.0f, 210.0f}   // 青
};
