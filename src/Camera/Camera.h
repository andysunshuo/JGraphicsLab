#pragma once
#include "Camera/Transforms.h"

struct Camera
{
    Vec3 eye{1.7f, 1.3f, 3.0f};
    Vec3 target{0, 0, 0};
    Vec3 up{0, 1, 0};
    float fovRadians = 1.04719755f;
    float aspect = 1.0f;
    float nearPlane = 0.1f;
    float farPlane = 20.0f;
    Mat4 View() const;
    Mat4 Projection() const;
};
