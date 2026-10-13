#pragma once
#include "Geometry/Primitives.h"
#include "Renderer/Framebuffer.h"
#include <vector>

void RasterizeTriangleMSAA(
    const Triangle& triangle,
    Raster::Color color);

void ResolveMSAA();
