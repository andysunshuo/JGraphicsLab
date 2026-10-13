#pragma once
#include "Geometry/Primitives.h"
#include "Renderer/Clipping.h"
#include "Renderer/Framebuffer.h"
#include <vector>

Raster::Color InterpolateColor(
    const Barycentric& bary,
    const Color3& a,
    const Color3& b,
    const Color3& c);

ClipVertex InterpolateClipVertex(const ClipVertex& a,
    const ClipVertex& b,
    float t);
