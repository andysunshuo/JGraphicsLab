#pragma once
#include "Geometry/Primitives.h"
#include <vector>

Vec4 MakePosition4(const Vec3& p);

Vec4 MakeDirection4(const Vec3& direction);

Mat3 MakeTranslation2D(float tx, float ty);

Mat3 MakeRotation2D(float radians);

Mat3 MakeScale2D(float sx, float sy);

Mat3 MakeReflectionX2D();

Mat3 MakeReflectionY2D();

Mat3 MakeShear2D(float shearX, float shearY);

Point2 TransformPoint2D(const Mat3& matrix, Point2 point);

Mat4 MakeTranslation3D(float tx, float ty, float tz);

Mat4 MakeScale3D(float sx, float sy, float sz);

Mat4 MakeRotationX3D(float radians);

Mat4 MakeRotationY3D(float radians);

Mat4 MakeRotationZ3D(float radians);

Mat4 MakeModelMatrix(
    const Vec3& position,
    const Vec3& rotationRadians,
    const Vec3& scale);

Mat4 LookAt(const Vec3& eye, const Vec3& target, const Vec3& up);

Mat4 MakeOrthographic(
    float left, float right,
    float bottom, float top,
    float zNear, float zFar);

Mat4 MakePerspective(
    float fovYRadians, float aspect,
    float zNear, float zFar);

bool PerspectiveDivide(const Vec4& clip, Vec3& ndc);

Point2 ViewportTransform(const Vec3& ndc, int width, int height);
