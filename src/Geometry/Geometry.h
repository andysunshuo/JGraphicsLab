#pragma once
#include "Geometry/Primitives.h"
#include "Renderer/Framebuffer.h"
#include <vector>

float EdgeFunction(
    Point2 a,
    Point2 b,
    Point2 p);

bool IsInsideTriangle(
    const Triangle& triangle,
    Point2 p);

bool ComputeBarycentric(
    const Triangle& triangle,
    Point2 p,
    Barycentric& result);

bool IsFrontFacing(
    const Triangle& triangle);

void DrawIndexedTriangles(
    const std::vector<Point2>& vertices,
    const std::vector<int>& indices,
    Raster::Color color);
