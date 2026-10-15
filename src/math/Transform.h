#pragma once
#include "geometry/Vertex.h"
#include "math/Mat4.h"

// 在 Transform.cpp 实现命名空间内的变换函数；角度统一使用弧度。
// 约定：右手系、列向量、相机看向自身 -Z，裁剪/NDC z 为 [-1,1]。
// 单位矩阵必须接收返回值：Mat4 result=Mat4::Identity()；静态函数不会修改 result。
// cpp 定义统一写 Transform::函数名，或全部放入 namespace Transform 的代码块。
// 默认参数只写在头文件声明；所有非 void 定义都必须返回真实结果。
namespace Transform
{
// 单位矩阵最后一列写 tx、ty、tz；缩放写对角线 sx、sy、sz。
// Translation：m[0][3]=tx、m[1][3]=ty、m[2][3]=tz。
// Scale：m[0][0]=sx、m[1][1]=sy、m[2][2]=sz；m[3][3] 仍为 1。
Mat4 Translation(const Vec3 &offset);
Mat4 Scale(const Vec3 &scale);
// 按右手规则填写各轴旋转的 sin/cos，最后一行仍为 (0,0,0,1)。
// 在 Identity 上修改：X 的 yz 块为 [c,-s;s,c]；Y 的 xz 块为 [c,s;-s,c]；
// Z 的 xy 块为 [c,-s;s,c]。绕 Z 的 pi/2 应把 +X 转为 +Y。
Mat4 RotationX(float radians);
Mat4 RotationY(float radians);
Mat4 RotationZ(float radians);
// 明确旋转次序：T*Rz*Ry*Rx*S；顶点先缩放再依次绕 X/Y/Z 旋转再平移。
Mat4 Model(const Vec3 &translation, const Vec3 &rotationRadians, const Vec3 &scale);

// z=normalize(eye-target)，x=normalize(up×z)，y=z×x；各行与 eye 点积取负。
// eye==target 或 up 与视向平行时抛 std::invalid_argument。
// 三个轴分别填第 0/1/2 行，最后一列为 -x.Dot(eye)、-y.Dot(eye)、-z.Dot(eye)。
// 先验证输入和退化基底，不要把 Normalized 返回的零向量直接当相机轴。
Mat4 LookAt(const Vec3 &eye, const Vec3 &target, const Vec3 &up);
// f=1/tan(fov/2)，m00=f/aspect，m11=f，m32=-1；近远面映射到 -1/+1。
// 要求 0<fov<pi、aspect>0、0<near<far，且所有参数有限。
// 从零矩阵开始：m22=(far+near)/(near-far)，m23=2*far*near/(near-far)，m33=0。
// 用相机空间 z=-near/-far 的位置验证透视除法后得到 -1/+1。
Mat4 Perspective(float fovRadians, float aspect, float nearPlane, float farPlane);
// 填写平移与缩放，将给定盒子映射到 NDC；要求范围均非零，0<near<far。
// Identity 上 m00=2/(right-left)、m11=2/(top-bottom)、m22=-2/(far-near)。
// m03=-(right+left)/(right-left)、m13=-(top+bottom)/(top-bottom)、m23=-(far+near)/(far-near)。
Mat4 Orthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane);
// 取 Model 左上 A，返回 inverse(A).Transposed()，奇异时明确报错。
// 复用 Mat3::TryInverse，不另写一套求逆；平移不进入法线变换。
Mat3 NormalMatrix(const Mat4 &model);
// Clip -> NDC -> Screen，成功填充所有属性与 reciprocalW；w<=epsilon 时返回 false。
// 先校验尺寸/属性/epsilon；临时完成除法和视口映射，全部有限后再提交 out。
// worldPosition/normal/uv/color 原值复制，保存 1/w，不在这里提前把 UV 除一次 w。
// cpp 定义参数写 float epsilon 即可；再次写 =1e-8f 会出现 C2572。
bool ToScreen(const ClipVertex &clip, int width, int height, ScreenVertex &out, float epsilon = 1e-8f);
} // namespace Transform

// 原有二维演示的兼容重载已经实现，保留原算法与签名，不移走现有逻辑。
// 这里只映射 NDC x/y，不执行 MVP、裁剪或透视除法。
// [-1,1] 映射到 [0,width]/[0,height]，右上端点由光栅化包围盒限制。
inline ScreenVertex ToScreen(const Vec3 &position, int width, int height)
{
    ScreenVertex result;
    result.x = (position.x + 1.0f) * 0.5f * width;
    result.y = (position.y + 1.0f) * 0.5f * height;
    return result;
}
