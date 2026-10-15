#pragma once
#include "math/Vec3.h"

// 在 Mat3.cpp 实现。m[行][列] 连续存储，每个向量按列向量参与 M*v。
// 默认初始化为零矩阵；需要单位矩阵时显式调用 Identity()。
class Mat3
{
  public:
    float m[3][3]{};

    // 检查：  矩阵里面有没有： - NaN，Inf
    // 很简单，是工程安全检查。
    bool IsFinite() const;

    static Mat3 Identity();
    // result[r][c]=sum_k m[r][k]*other.m[k][c]，不能按元素相乘。
    Mat3 operator*(const Mat3 &other) const;
    Vec3 operator*(const Vec3 &value) const;
    // 矩阵转置。
    Mat3 Transposed() const;

    // 矩阵行列式, 求三维矩阵行列式的值。
    float Determinant() const;

    // 现有余子式公式在常规输入下正确，不必重写。待补：
    // 1 输入矩阵有限且 epsilon 有限/非负；2 det 有限且 |det|>epsilon；
    // 3 在局部矩阵求逆；4 局部结果 IsFinite()；全部成功才写 out。
    // 任何失败返回 false、不改 out。det 有限不保证逆矩阵的每个元素都有限。
    // 与 cpp 定义匹配时，默认参数只写在此声明，不在定义中再次赋值。
    bool TryInverse(Mat3 &out, float epsilon = 1e-8f) const;
};
