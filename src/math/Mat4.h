#pragma once
#include "math/Mat3.h"
#include "math/Vec4.h"

// 在 Mat4.cpp 实现。m[行][列]、列向量、右手系；平移在最后一列。
// 从模型到裁剪空间是 Projection*View*Model*position，右侧先执行。
// 默认值为零矩阵，不是 Identity；保留原 Identity 和两种 operator* 签名。
class Mat4
{
  public:
    float m[4][4]{};
    static Mat4 Identity();
    // 三层循环：result[r][c]=sum_k m[r][k]*other.m[k][c]。
    Mat4 operator*(const Mat4 &other) const;
    // 每个输出分量等于该行与 Vec4 的点积，包括输出 w。
    Vec4 operator*(const Vec4 &value) const;
    Mat4 Transposed() const;
    Mat3 UpperLeft3x3() const;
    bool IsFinite() const;
    // 当前带主元的 Gauss-Jordan 主流程与失败保留 out 的实现已通过常规检查。
    // 待补第一步：epsilon 必须有限且非负，否则 false；fabs(NaN) 仍是 NaN。
    // 继续保持输入/输出有限性检查和局部计算，失败不改 out。
    // 供相机逆变换和生成射线复用，不必另写一份 4x4 求逆。
    bool TryInverse(Mat4 &out, float epsilon = 1e-8f) const;
};
