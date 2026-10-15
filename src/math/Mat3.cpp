#include "math/Vec3.h"
#include "math/Mat3.h"

Mat3 Mat3::Identity()
{
    Mat3 result;
    for (int i = 0; i < 3; i++)
    {
        result.m[i][i] = 1;
    }
    return result;
}

// result[r][c]=sum_k m[r][k]*other.m[k][c]，不能按元素相乘。
Mat3 Mat3::operator*(const Mat3& other) const
{
    Mat3 result;
    
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            result.m[i][j] = 
                m[i][0] * other.m[0][j] +
                m[i][1] * other.m[1][j] +
                m[i][2] * other.m[2][j];
        }
        
    }

    return result;

}
Vec3 Mat3::operator*(const Vec3& value) const
{
    Vec3 result;

    result.x = m[0][0] * value.x + m[0][1] * value.y + m[0][2] * value.z;
    result.y = m[1][0] * value.x + m[1][1] * value.y + m[1][2] * value.z;
    result.z = m[2][0] * value.x + m[2][1] * value.y + m[2][2] * value.z;

    return result;
}

Mat3 Mat3::Transposed() const
{
    Mat3 result;

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            result.m[i][j] = m[j][i];

        }

    }

    return result;

}
#include <cmath>

float Mat3::Determinant() const
{
    // 3x3 行列式，按第一行展开
    return m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
        - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0])
        + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
}

bool Mat3::IsFinite() const
{
    for (int r = 0; r < 3; ++r)
    {
        for (int c = 0; c < 3; ++c)
        {
            if (!std::isfinite(m[r][c]))
                return false;
        }
    }
    return true;
}

// 注意：如果默认参数已经在类声明里写过：
// bool TryInverse(Mat3& out, float epsilon = 1e-8f) const;
// 那么这里定义时不要再写 = 1e-8f，否则会重复默认参数。
bool Mat3::TryInverse(Mat3& out, float epsilon) const
{
    const float det = Determinant();

    if (!std::isfinite(det) || std::fabs(det) <= epsilon)
        return false;

    const float invDet = 1.0f / det;

    // 先算到局部变量，失败时不修改 out
    Mat3 inv;

    // 伴随矩阵 / det，行主序
    inv.m[0][0] = (m[1][1] * m[2][2] - m[1][2] * m[2][1]) * invDet;
    inv.m[0][1] = -(m[0][1] * m[2][2] - m[0][2] * m[2][1]) * invDet;
    inv.m[0][2] = (m[0][1] * m[1][2] - m[0][2] * m[1][1]) * invDet;

    inv.m[1][0] = -(m[1][0] * m[2][2] - m[1][2] * m[2][0]) * invDet;
    inv.m[1][1] = (m[0][0] * m[2][2] - m[0][2] * m[2][0]) * invDet;
    inv.m[1][2] = -(m[0][0] * m[1][2] - m[0][2] * m[1][0]) * invDet;

    inv.m[2][0] = (m[1][0] * m[2][1] - m[1][1] * m[2][0]) * invDet;
    inv.m[2][1] = -(m[0][0] * m[2][1] - m[0][1] * m[2][0]) * invDet;
    inv.m[2][2] = (m[0][0] * m[1][1] - m[0][1] * m[1][0]) * invDet;

    out = inv;
    return true;
}