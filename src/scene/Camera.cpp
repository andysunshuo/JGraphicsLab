#include "Camera.h"

#include <cmath>
#include <stdexcept>

// ============================================================
// 1. ViewMatrix
// 获取观察矩阵（View Matrix）
// ============================================================
//
// 作用：
// 将世界坐标系中的顶点转换到相机坐标系。
//
// 数学公式：
//
// View = LookAt(eye, target, up)
//
// 参数说明：
// eye    : 相机在世界坐标系中的位置
// target : 相机观察的目标位置
// up     : 相机的参考上方向
//
// 示例：
// eye    = (0, 0, 3)
// target = (0, 0, 0)
// up     = (0, 1, 0)
//
// 预期：
// 世界原点 (0,0,0) 变换后为 (0,0,-3)。
//
// 实现步骤：
// 1. 调用 Transform::LookAt()。
// 2. 传入 eye、target、up。
// 3. 返回计算得到的 Mat4。
//
// 注意：
// 不要在 Camera.cpp 中重新实现 LookAt 的数学公式。
//
Mat4 Camera::ViewMatrix() const
{
    Mat4 view;

    // TODO 1:
    // 调用 Transform::LookAt()
    // 使用 eye、target、up 计算矩阵。
    //
    // view = ...

    // TODO 2:
    // 返回观察矩阵。
    return Transform::LookAt(eye, target, up);
}


// ============================================================
// 2. ProjectionMatrix
// 获取投影矩阵（Projection Matrix）
// ============================================================
//
// 作用：
// 根据投影类型，创建透视投影矩阵或正交投影矩阵。
//
// 投影类型：
//
// Perspective：透视投影
//   - 近大远小
//   - 常用于游戏场景、真实相机
//
// Orthographic：正交投影
//   - 不会近大远小
//   - 常用于 CAD、工程视图、UI
//
// 数学公式：
//
// Perspective:
// P = Perspective(fovRadians, aspect, nearPlane, farPlane)
//
// Orthographic:
// P = Orthographic(
//     orthoLeft,
//     orthoRight,
//     orthoBottom,
//     orthoTop,
//     nearPlane,
//     farPlane
// )
//
// 实现步骤：
// 1. 判断 projectionType。
// 2. 如果是 Perspective：
//    调用 Transform::Perspective()。
// 3. 如果是 Orthographic：
//    调用 Transform::Orthographic()。
// 4. 返回对应矩阵。
//
// 要求：
// 使用 switch 或 if 均可。
// 不要重复实现投影矩阵的数学公式。
// 不要把 fovRadians 转成角度。
//
Mat4 Camera::ProjectionMatrix() const
{
    switch (projectionType)
    {
    case ProjectionType::Perspective:
        return Transform::Perspective(
            fovRadians,
            aspect,
            nearPlane,
            farPlane
        );

    case ProjectionType::Orthographic:
        return Transform::Orthographic(
            orthoLeft,
            orthoRight,
            orthoBottom,
            orthoTop,
            nearPlane,
            farPlane
        );

    default:
        throw std::invalid_argument(
            "Camera: unknown projection type"
        );
    }
}


// ============================================================
// 3. ViewProjectionMatrix
// 获取视图投影矩阵（View Projection Matrix）
// ============================================================
//
// 作用：
// 将 View Matrix 与 Projection Matrix 合并。
//
// 世界坐标变换到裁剪空间：
//
// P_clip = Projection * View * P_world
//
// 因此：
//
// VP = Projection * View
//
// 实现步骤：
// 1. 获取 ViewMatrix()。
// 2. 获取 ProjectionMatrix()。
// 3. 计算 Projection * View。
// 4. 返回计算结果。
//
// 注意：
// 矩阵乘法不满足交换律。
//
// Projection * View  正确
// View * Projection  错误（当前列向量约定）
//
Mat4 Camera::ViewProjectionMatrix() const
{
    return ProjectionMatrix() * ViewMatrix();
}


// ============================================================
// 4. GenerateRay
// 根据屏幕像素生成世界空间射线
// ============================================================
// 给定屏幕上的一个像素，计算出一条从相机出发、穿过该像素、射向三维世界的 Ray（射线）。
Ray GenerateRay(int pixelX, int pixelY, int width, int height) 
{
    Ray result;
    return result;
}