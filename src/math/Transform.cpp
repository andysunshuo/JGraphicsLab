#include "math/Transform.h"

#include "geometry/Vertex.h"
#include "math/Mat4.h"
#include "math/Vec3.h"

#include <cmath>
#include <stdexcept>

// 单位矩阵最后一列写 tx、ty、tz；缩放写对角线 sx、sy、sz。
Mat4 Transform::Translation(const Vec3 &offset)
{
    Mat4 result = Mat4::Identity();
    result.m[0][3] = offset.x;
    result.m[1][3] = offset.y;
    result.m[2][3] = offset.z;

    return result;
}
Mat4 Transform::Scale(const Vec3 &scale)
{
    Mat4 result = Mat4::Identity();
    result.m[0][0] = scale.x;
    result.m[1][1] = scale.y;
    result.m[2][2] = scale.z;
    return result;
}

// 按右手规则填写各轴旋转的 sin/cos，最后一行仍为 (0,0,0,1)。
Mat4 Transform::RotationX(float radians)
{
    Mat4 result = Mat4::Identity();
    float c = std::cos(radians);
    float s = std::sin(radians);

    result.m[1][1] = c;
    result.m[1][2] = -s;
    result.m[2][1] = s;
    result.m[2][2] = c;

    return result;
}

Mat4 Transform::RotationY(float radians)
{
    Mat4 result = Mat4::Identity();

    float c = std::cos(radians);
    float s = std::sin(radians);

    result.m[0][0] = c;
    result.m[0][2] = s;
    result.m[2][0] = -s;
    result.m[2][2] = c;

    return result;
}
Mat4 Transform::RotationZ(float radians)
{
    Mat4 result = Mat4::Identity();
    float c = std::cos(radians);
    float s = std::sin(radians);

    result.m[0][0] = c;
    result.m[0][1] = -s;
    result.m[1][0] = s;
    result.m[1][1] = c;
    return result;
}

// 明确旋转次序：T*Rz*Ry*Rx*S；顶点先缩放再依次绕 X/Y/Z 旋转再平移。
// 为什么先缩放后旋转在平移。这个顺序换了会怎样？
Mat4 Transform::Model(const Vec3 &translation, const Vec3 &rotationRadians, const Vec3 &scale)
{
    return Translation(translation) *
        RotationZ(rotationRadians.z) * 
        RotationY(rotationRadians.y) * 
        RotationX(rotationRadians.x) * 
        Scale(scale);     
    // 研究 复杂的欧拉角问题  为什么 旋转先x后y后z；
}

// z=normalize(eye-target)，x=normalize(up×z)，y=z×x；各行与 eye 点积取负。
// eye==target 或 up 与视向平行时抛 std::invalid_argument。

// ============================================================
// LookAt — 构造右手坐标系观察矩阵
// ============================================================
//
// eye    : 相机在世界坐标系的位置
// target : 相机观察的目标位置
// up     : 参考上方向
//
// z = normalize(eye - target)
// x = normalize(up × z)
// y = z × x
//
// View =
// | x.x  x.y  x.z  -dot(x, eye) |
// | y.x  y.y  y.z  -dot(y, eye) |
// | z.x  z.y  z.z  -dot(z, eye) |
// |  0    0    0         1      |
//
//  View = R  *  T   坐标轴又有旋转 又有平移。

Mat4 Transform::LookAt(
    const Vec3& eye,
    const Vec3& target,
    const Vec3& up)
{
    // 1. 确定相机坐标轴
    Vec3 forward = (target - eye).Normalized();

    Vec3 z = -forward;
    Vec3 x = up.Cross(z).Normalized();
    Vec3 y = z.Cross(x);

    // 2. 平移整个世界，使相机位于原点
    Mat4 translation = Translation(-eye);

    // 3. 构造旋转矩阵
    Mat4 rotation;

    rotation.m[0][0] = x.x;
    rotation.m[0][1] = x.y;
    rotation.m[0][2] = x.z;
    rotation.m[0][3] = 0.0f;

    rotation.m[1][0] = y.x;
    rotation.m[1][1] = y.y;
    rotation.m[1][2] = y.z;
    rotation.m[1][3] = 0.0f;

    rotation.m[2][0] = z.x;
    rotation.m[2][1] = z.y;
    rotation.m[2][2] = z.z;
    rotation.m[2][3] = 0.0f;

    rotation.m[3][0] = 0.0f;
    rotation.m[3][1] = 0.0f;
    rotation.m[3][2] = 0.0f;
    rotation.m[3][3] = 1.0f;

    // 4. 先平移，再旋转
    return rotation * translation;
}
Mat4 Transform::Perspective(float fovRadians, float aspect, float nearPlane, float farPlane)
{
    // 1. 参数验证
    constexpr float PI = 3.14159265358979323846f;


    // 2. FOV 缩放系数
    const float s = 1.0f / std::tan(fovRadians * 0.5f);

    // 3. 近远平面距离差
    const float range = farPlane - nearPlane;

    // 4. 从零矩阵开始
    Mat4 result{};

    // 确保 Mat4{} 在你的实现中是零矩阵
    // 如果不是，需要显式清零。

    // 5. X 方向缩放
    result.m[0][0] = s / aspect;

    // 6. Y 方向缩放
    result.m[1][1] = s;

    // 7. Z 方向映射
    result.m[2][2] = -(farPlane + nearPlane) / range;
    result.m[2][3] = -(2.0f * farPlane * nearPlane) / range;

    // 8. 构造 w_clip = -z_view
    result.m[3][2] = -1.0f;

    return result;
}
// 填写平移与缩放，将给定盒子映射到 NDC；要求范围均非零，0<near<far。
Mat4 Transform::Orthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane)
{
    if (!std::isfinite(left) || !std::isfinite(right) || !std::isfinite(bottom) || !std::isfinite(top) ||
        !std::isfinite(nearPlane) || !std::isfinite(farPlane) || !(right > left) || !(top > bottom) ||
        !(nearPlane > 0 && farPlane > nearPlane))
        throw std::invalid_argument("正交投影范围无效");
    Mat4 result = Mat4::Identity();

    // X：缩放 + 平移
    result.m[0][0] = 2.0f / (right - left);
    result.m[0][3] = -(right + left) / (right - left);

    // Y：缩放 + 平移
    result.m[1][1] = 2.0f / (top - bottom);
    result.m[1][3] = -(top + bottom) / (top - bottom);

    // Z：缩放 + 平移，注意负号
    result.m[2][2] = -2.0f / (farPlane - nearPlane);
    result.m[2][3] = -(farPlane + nearPlane) / (farPlane - nearPlane);

    return result;
}
// 取 Model 左上 A，返回 inverse(A).Transposed()，奇异时明确报错。
// 复用 Mat3::TryInverse，不另写一套求逆；平移不进入法线变换。
// 在模型发生非均匀缩放时，保证表面法线仍然正确。
// 这个不懂做了什么? 为什么用它/
Mat3 Transform::NormalMatrix(const Mat4& model)
{
    // 1. 提取 Model 左上角 3×3
    Mat3 A;

    for (int row = 0; row < 3; ++row)
    {
        for (int col = 0; col < 3; ++col)
        {
            A.m[row][col] = model.m[row][col];
        }
    }

    // 2. 求逆矩阵
    Mat3 inverse;

    if (!A.TryInverse(inverse))
    {
        throw std::invalid_argument("NormalMatrix: model matrix is singular");
    }

    // 3. 求逆矩阵的转置
    Mat3 result = inverse.Transposed();

    return result;
}

// Clip -> NDC -> Screen，成功填充所有属性与 reciprocalW；w<=epsilon 时返回 false。
bool Transform::ToScreen(const ClipVertex &clip, int width, int height, ScreenVertex &out, float epsilon)
{
    // 读取裁剪坐标
    const float x = clip.position.x;
    const float y = clip.position.y;
    const float z = clip.position.z;
    const float w = clip.position.w;

    // 检查输入
    if (width <= 0 || height <= 0 || !std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z) || !std::isfinite(w) ||
        !std::isfinite(epsilon) || epsilon < 0.0f || w <= epsilon)
    {
        return false;
    }

    // 1. 透视除法
    const float invW = 1.0f / w;

    const float ndcX = x * invW;
    const float ndcY = y * invW;
    const float ndcZ = z * invW;

    // 屏幕 Y 继续向上，只在帧缓冲显示内存中翻转一次。
    ScreenVertex candidate{};
    candidate.x = (ndcX + 1) * .5f * width;
    candidate.y = (ndcY + 1) * .5f * height;
    candidate.ndcZ = ndcZ;
    candidate.reciprocalW = invW;
    candidate.worldPosition = clip.worldPosition;
    candidate.normal = clip.normal;
    candidate.uv = clip.uv;
    candidate.color = clip.color;
    if (!std::isfinite(candidate.x) || !std::isfinite(candidate.y) || !std::isfinite(ndcZ) || !std::isfinite(invW) ||
        !candidate.worldPosition.IsFinite() || !candidate.normal.IsFinite() || !candidate.uv.IsFinite() ||
        !candidate.color.IsFinite())
        return false;
    out = candidate;
    return true;
}

