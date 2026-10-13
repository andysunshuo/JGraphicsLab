#include "Camera/Transforms.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <stdexcept>
#include <limits>

// ------------------------------------------------------------
// HW18.1 — 三维位置 -> 齐次位置 (w=1)
// 测试：(2,3,4) -> (2,3,4,1)
// ------------------------------------------------------------
Vec4 MakePosition4(const Vec3& p)
{
    return { p.x, p.y, p.z, 1.0f };
}

// HW18.2 — 三维方向 -> 齐次方向 (w=0)
// 测试：(2,3,4) -> (2,3,4,0)
Vec4 MakeDirection4(const Vec3& direction)
{
    return { direction.x, direction.y, direction.z, 0.0f };
}

// ============================================================
// 六、HW21：2D Transformation（3x3 齐次矩阵）
// ============================================================
// 用 (x,y,1) 表示位置，统一由 Mat3 完成变换。
// 由于采用列向量，右边的矩阵先作用。
// 例如：T*R*S*p 表示先缩放，再旋转，最后平移。

// HW21.1 — 平移矩阵
// 输入 tx=10,ty=20；(2,3) -> (12,23)。
// 提示：把 tx、ty 放在 m[0][2]、m[1][2]。
Mat3 MakeTranslation2D(float tx, float ty)
{
    Mat3 m = IdentityMat3();
    m.m[0][2] = tx;
    m.m[1][2] = ty;
    return m;
}

// HW21.2 — 旋转矩阵（角度参数以弧度为单位）
// 正角度：从 +X 向 +Y 逆时针旋转。
// 测试：绕原点旋转 90°，(1,0)->约(0,1)。
// 提示：左上 2x2：[cos,-sin; sin,cos]。
Mat3 MakeRotation2D(float radians)
{
    const float c = std::cos(radians), s = std::sin(radians);
    Mat3 m = IdentityMat3();
    m.m[0][0] = c; m.m[0][1] = -s;
    m.m[1][0] = s; m.m[1][1] = c;
    return m;
}

// HW21.3 — 缩放矩阵
// 测试：sx=2,sy=3，(4,5)->(8,15)。
Mat3 MakeScale2D(float sx, float sy)
{
    Mat3 m = IdentityMat3();
    m.m[0][0] = sx; m.m[1][1] = sy;
    return m;
}

// HW21.4 — 关于 X 轴反射
// 测试：(2,3)->(2,-3)。
Mat3 MakeReflectionX2D()
{
    Mat3 m = IdentityMat3();
    m.m[1][1] = -1.0f; // 关于 X 轴对称，翻转 y。
    return m;
}

// HW21.5 — 关于 Y 轴反射
// 测试：(2,3)->(-2,3)。
Mat3 MakeReflectionY2D()
{
    Mat3 m = IdentityMat3();
    m.m[0][0] = -1.0f; // 关于 Y 轴对称，翻转 x。
    return m;
}

// HW21.6 — 错切（Shear）矩阵
// x' = x + shearX * y，y' = y + shearY * x
// 测试：shearX=1,shearY=0，(2,3)->(5,3)。
Mat3 MakeShear2D(float shearX, float shearY)
{
    Mat3 m = IdentityMat3();
    m.m[0][1] = shearX;
    m.m[1][0] = shearY;
    return m;
}

// HW21.7 — 用 Mat3 变换二维点（练习结果可直接在窗口里绘制）
// 这里只做三维齐次向量与 Point2 的接口适配。
Point2 TransformPoint2D(const Mat3& matrix, Point2 point)
{
    const Vec3 homogeneous = { point.x, point.y, 1.0f };
    const Vec3 transformed = Multiply(matrix, homogeneous);
    // 当前 2D Affine Matrix 的最后一行是 (0,0,1)，结果 w=1。
    return { transformed.x, transformed.y };
}

// ============================================================
// 七、HW22 / HW23：3D Transformation 和 Model Matrix
// ============================================================

// HW22.1 — 三维平移矩阵
// 测试：(1,2,3) 平移 (10,20,30) -> (11,22,33)。
// 提示：第四列存放 tx、ty、tz（注意从 0 编号）。
Mat4 MakeTranslation3D(float tx, float ty, float tz)
{
    Mat4 m = IdentityMat4();
    m.m[0][3] = tx;
    m.m[1][3] = ty;
    m.m[2][3] = tz;
    return m;
}

// HW22.2 — 三维缩放矩阵
// 测试：Scale=(2,3,4)，Point=(1,2,3,1)->(2,6,12,1)。
Mat4 MakeScale3D(float sx, float sy, float sz)
{
    Mat4 m = IdentityMat4();
    m.m[0][0] = sx; m.m[1][1] = sy; m.m[2][2] = sz;
    return m;
}

// HW22.3 — 绕 X 轴的右手旋转矩阵
// YZ 平面旋转，测试：+90°：(0,1,0)->(0,0,1)。
Mat4 MakeRotationX3D(float radians)
{
    const float c = std::cos(radians), s = std::sin(radians);
    Mat4 m = IdentityMat4();
    m.m[1][1] = c; m.m[1][2] = -s;
    m.m[2][1] = s; m.m[2][2] = c;
    return m;
}

// HW22.4 — 绕 Y 轴的右手旋转矩阵
// XZ 平面旋转，测试：+90°：(0,0,1)->(1,0,0)。
Mat4 MakeRotationY3D(float radians)
{
    const float c = std::cos(radians), s = std::sin(radians);
    Mat4 m = IdentityMat4();
    m.m[0][0] = c; m.m[0][2] = s;
    m.m[2][0] = -s; m.m[2][2] = c;
    return m;
}

// HW22.5 — 绕 Z 轴的右手旋转矩阵
// XY 平面旋转，测试：+90°：(1,0,0)->(0,1,0)。
Mat4 MakeRotationZ3D(float radians)
{
    const float c = std::cos(radians), s = std::sin(radians);
    Mat4 m = IdentityMat4();
    m.m[0][0] = c; m.m[0][1] = -s;
    m.m[1][0] = s; m.m[1][1] = c;
    return m;
}

// ------------------------------------------------------------
// HW23 — Model Matrix
// 参数：
// position：模型的世界空间位置。
// rotationRadians：绕 XYZ 各轴旋转的弧度值。
// scale：每个轴的缩放倍数。
// 统一组合约定：M = T * Rz * Ry * Rx * S。
// 注意：要按右边先作用的顺序解释！
// 测试：全零位置/旋转、全 1 缩放时，应得到单位矩阵。
// ------------------------------------------------------------
Mat4 MakeModelMatrix(
    const Vec3& position,
    const Vec3& rotationRadians,
    const Vec3& scale)
{
    // 列向量：M = T * Rz * Ry * Rx * S；最右边的 S 首先应用。
    const Mat4 t = MakeTranslation3D(position.x, position.y, position.z);
    const Mat4 rz = MakeRotationZ3D(rotationRadians.z);
    const Mat4 ry = MakeRotationY3D(rotationRadians.y);
    const Mat4 rx = MakeRotationX3D(rotationRadians.x);
    const Mat4 s = MakeScale3D(scale.x, scale.y, scale.z);
    return Multiply(t, Multiply(rz, Multiply(ry, Multiply(rx, s))));
}

// ============================================================
// 八、HW24：View Matrix / LookAt
// ============================================================
// 右手视图坐标约定：相机看向自己的 -Z 方向。
// eye：相机世界坐标位置。
// target：注视的世界空间目标。
// up：参考向上的方向，例如 (0,1,0)。
// 提示步骤：
// 1. forward = Normalize(target-eye)；
// 2. right = Normalize(Cross(forward,up))；
// 3. cameraUp = Cross(right,forward)；
// 4. 利用 right、cameraUp、-forward 构造旋转部分；
// 5. 平移项相当于对 eye 求相应的负点积。
// 测试：eye=(0,0,3)，target=(0,0,0)，世界原点转到 (0,0,-3)。
// 边界：eye==target 或 up 与视线共线时，需要做有效性检查。
Mat4 LookAt(const Vec3& eye, const Vec3& target, const Vec3& up)
{
    // f：观察方向；r：相机右方向；u：经过正交化的相机上方向。
    // 相机在 View Space 中看向 -Z，因此第三行为 -f。
    const Vec3 fVector = Subtract(target, eye);
    if (Length(fVector) <= 1e-7f)
        throw std::invalid_argument("LookAt: eye and target must differ");
    const Vec3 f = Normalize(fVector);
    const Vec3 rightVector = Cross(f, up);
    if (Length(rightVector) <= 1e-7f)
        throw std::invalid_argument("LookAt: up is parallel to view direction");
    const Vec3 r = Normalize(rightVector);
    const Vec3 u = Cross(r, f);
    Mat4 m = IdentityMat4();
    m.m[0][0] = r.x;  m.m[0][1] = r.y;  m.m[0][2] = r.z;  m.m[0][3] = -Dot(r, eye);
    m.m[1][0] = u.x;  m.m[1][1] = u.y;  m.m[1][2] = u.z;  m.m[1][3] = -Dot(u, eye);
    m.m[2][0] = -f.x; m.m[2][1] = -f.y; m.m[2][2] = -f.z; m.m[2][3] = Dot(f, eye);
    return m;
}

// ============================================================
// 九、HW25：Orthographic Projection
// ============================================================
// 参数：left/right/bottom/top 是相机空间的可视矩形边界。
// zNear、zFar 为正的距离，沿相机前方 -Z 测量，0<near<far。
// 使用 OpenGL 风格 NDC z∈[-1,1]。
// 提示：x/y 通过平移+缩放映射到 [-1,1]；
//       z=-near 应映射到 -1，z=-far 应映射到 +1。
// 测试：left=-1,right=1,bottom=-1,top=1,near=1,far=11，
//       相机空间点 (0,0,-1,1) -> NDC (0,0,-1)。
Mat4 MakeOrthographic(
    float left, float right,
    float bottom, float top,
    float zNear, float zFar)
{
    if (!(right > left) || !(top > bottom) || !(zNear > 0.0f) || !(zFar > zNear))
        throw std::invalid_argument("MakeOrthographic: invalid bounds");
    Mat4 m = IdentityMat4();
    // OpenGL NDC：[-1,+1]，zView=-near 映射为 -1，zView=-far 映射为 +1。
    m.m[0][0] = 2.0f / (right - left);
    m.m[1][1] = 2.0f / (top - bottom);
    m.m[2][2] = -2.0f / (zFar - zNear);
    m.m[0][3] = -(right + left) / (right - left);
    m.m[1][3] = -(top + bottom) / (top - bottom);
    m.m[2][3] = -(zFar + zNear) / (zFar - zNear);
    return m;
}

// ============================================================
// 十、HW26：Perspective Projection
// ============================================================
// fovYRadians：垂直视场角，单位为弧度，不是角度。
// aspect：画面宽/高，例如 16.0f/9.0f；100x100 时为 1。
// zNear、zFar：沿相机前方 -Z 的正距离，0<near<far。
// 列向量 + OpenGL 风格：投影后 w_clip=-z_view。
// 核心关系：f = 1/tan(fovY/2)，X= f/aspect，Y=f。
// 测试：位于观察方向上的点 (0,0,-2) 投影后 x_ndc=y_ndc=0。
// 注意：必须在齐次裁剪之后，才做透视除法。
Mat4 MakePerspective(
    float fovYRadians, float aspect,
    float zNear, float zFar)
{
    const float pi = 3.14159265358979323846f;
    if (!(fovYRadians > 0.0f && fovYRadians < pi) ||
        !(aspect > 0.0f) || !(zNear > 0.0f) || !(zFar > zNear))
        throw std::invalid_argument("MakePerspective: invalid FOV/aspect/near/far");
    const float f = 1.0f / std::tan(fovYRadians * 0.5f);
    Mat4 m = {}; // 透视矩阵不是单位矩阵！
    m.m[0][0] = f / aspect;
    m.m[1][1] = f;
    m.m[2][2] = -(zFar + zNear) / (zFar - zNear);
    m.m[2][3] = -(2.0f * zNear * zFar) / (zFar - zNear);
    m.m[3][2] = -1.0f; // wClip=-zView，后续做透视除法。
    return m;
}

// ============================================================
// 十一、HW27：Perspective Divide
// ============================================================
// 输入：裁剪空间四维坐标 (x_clip,y_clip,z_clip,w_clip)。
// 输出：NDC 三维坐标 (x/w,y/w,z/w)。
// 返回 true 表示 w 有效且输出成功，false 表示无法安全除法。
// 思考：为什么不能在裁剪前，对整个三角形的顶点先做除法？
// 注意：这只是除法环节，不能代替 HW30 的齐次裁剪。
bool PerspectiveDivide(const Vec4& clip, Vec3& ndc)
{
    // 这里只负责除法；视锥裁剪必须在此之前完成。
    if (!std::isfinite(clip.w) || std::abs(clip.w) <= 1e-7f)
        return false;
    ndc = { clip.x / clip.w, clip.y / clip.w, clip.z / clip.w };
    return std::isfinite(ndc.x) && std::isfinite(ndc.y) && std::isfinite(ndc.z);
}

// ============================================================
// 十二、HW28：Viewport Transformation
// ============================================================
// 输入：NDC 中的 x/y，范围一般是 [-1,1]。
// 输出：Framebuffer 的连续坐标（不是必须取整数）。
// 当前坐标约定：Framebuffer X 向右、Y 向上。
// 公式提示：xScreen=(xNdc+1)*width/2，yScreen=(yNdc+1)*height/2。
// 测试：NDC (-1,-1)->(0,0)；(0,0)->(50,50)；(1,1)->(100,100)。
// 注意：(100,100) 是右上边界，不是合法像素中心；绘制时做边界裁剪。
Point2 ViewportTransform(const Vec3& ndc, int width, int height)
{
    // 映射到连续视口边界 [0,width]x[0,height]，像素中心用 x+0.5/y+0.5。
    // NDC=+1 对应右/上边界，像素访问时仍必须做裁剪。
    return { (ndc.x + 1.0f) * (static_cast<float>(width) * 0.5f),
            (ndc.y + 1.0f) * (static_cast<float>(height) * 0.5f) };
}
