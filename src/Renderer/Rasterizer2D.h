#pragma once
#include "Geometry/Primitives.h"
#include "Renderer/Framebuffer.h"
#include <vector>

void DrawHorizontalLine(
    int y,
    int xStart,
    int xEnd,
    Raster::Color color);

void DrawVerticalLine(
    int x,
    int yStart,
    int yEnd,
    Raster::Color color);

void DrawLine(
    Point2 start,
    Point2 end,
    Raster::Color color);

void DrawLine1(
    Point2 start,
    Point2 end,
    Raster::Color color);

void DrawRectangleOutline(
    Point2 minPoint,
    Point2 maxPoint,
    Raster::Color color);

void FillRectangle(
    Point2 minPoint,
    Point2 maxPoint,
    Raster::Color color);

bool inCircleOutline(int i, int j,
    Point2 center, float radius);

void DrawCircleOutline(
    Point2 center,
    float radius,
    Raster::Color color);

bool inCircle(int i, int j, Point2 center, float radius);

void FillCircle(
    Point2 center,
    float radius,
    Raster::Color color);

void DrawTriangleWireframe(
    const Triangle& triangle,
    Raster::Color color);

void FillTriangle(
    const Triangle& triangle,
    Raster::Color color);

void FillTriangleColorInterpolation(
    const Triangle& triangle,
    const Color3& colorA,
    const Color3& colorB,
    const Color3& colorC);
