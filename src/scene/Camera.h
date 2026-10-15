#pragma once
#include "math/Transform.h"
#include "raytracing/Ray.h"

enum class ProjectionType
{
    Perspective,
    Orthographic
};

// 在 Camera.cpp 实现；相机是视图/投影参数，不持有 Mesh 或 FrameBuffer。
struct Camera
{
    Vec3 eye{0, 0, 3}, target{0, 0, 0}, up{0, 1, 0};
    ProjectionType projectionType = ProjectionType::Perspective;
    float fovRadians = 1.04719755f; // 60 度；不要把 60 当作弧度传入矩阵。
    float aspect = 1, nearPlane = 0.1f, farPlane = 100;
    float orthoLeft = -1, orthoRight = 1, orthoBottom = -1, orthoTop = 1;

    // 只转调 Transform::LookAt、Perspective/Orthographic，不重复写矩阵公式。
    // 下一步先实现下面三个包装；GenerateRay 等 Ray 与逆投影验证后再写。
    Mat4 ViewMatrix() const;
    Mat4 ProjectionMatrix() const;
    Mat4 ViewProjectionMatrix() const; // ProjectionMatrix()*ViewMatrix()。
    // 像素中心 (x+0.5,y+0.5) 映射到 NDC，逆 VP 得到近/远世界点。
    // 透视 origin=eye，方向指向远点；正交 origin=近点，方向=远点-近点。
    // 方向归一化；要求尺寸合法且 aspect 与 width/height 一致，奇异 VP 报错。
    // 参数 pixelY 沿屏幕 Y 向上，图像行号从上到下时调用处应先翻转。
    Ray GenerateRay(int pixelX, int pixelY, int width, int height) const;
};
