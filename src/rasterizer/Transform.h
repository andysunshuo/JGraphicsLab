#pragma once

#include "../geometry/Mesh.h"
#include "Vertex.h"

inline ScreenVertex ToScreen(const Vec3 &position, int width, int height)
{

    ScreenVertex result;

    // X:
    // [-1,1] -> [0,width]

    result.x = (position.x + 1.0f) * 0.5f * width;

    // Y:
    // [-1,1] -> [0,height]

    result.y = (position.y + 1.0f) * 0.5f * height;

    return result;
}
