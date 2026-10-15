#include "Mat4.h"

#include <cmath>
#include <stdexcept>



Mat4 Mat4::Identity()
{
    Mat4 result;
    for (int i = 0; i < 4; i++)
    {
        result.m[i][i] = 1;
    }
    return result;
}



Mat4 Mat4::operator*(const Mat4& other) const
{
    Mat4 result;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            result.m[i][j] =
                m[i][0] * other.m[0][j] +
                m[i][1] * other.m[1][j] +
                m[i][2] * other.m[2][j] +
                m[i][3] * other.m[3][j];
        }
        
    }



    return result;
}





Vec4 Mat4::operator*(const Vec4& value) const
{
    Vec4 result;
    result.x =
        m[0][0] * value.x +
        m[0][1] * value.y +
        m[0][2] * value.z +
        m[0][3] * value.w;


    result.y =
        m[1][0] * value.x +
        m[1][1] * value.y +
        m[1][2] * value.z +
        m[1][3] * value.w;


    result.z =
        m[2][0] * value.x +
        m[2][1] * value.y +
        m[2][2] * value.z +
        m[2][3] * value.w;


    result.w =
        m[3][0] * value.x +
        m[3][1] * value.y +
        m[3][2] * value.z +
        m[3][3] * value.w;

    return result;
}



Mat4 Mat4::Transposed() const
{
    Mat4 result;


    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            result.m[i][j] = m[j][i];
        }
    }


    return result;
}



// ============================================================
// 提取左上角3x3矩阵
//
// 已实现
//
// Mat4:
//
// R R R Tx
// R R R Ty
// R R R Tz
// 0 0 0 1
//
// 返回:
//
// R R R
// R R R
// R R R
//
// 用于:
// Normal Matrix
// ============================================================

Mat3 Mat4::UpperLeft3x3() const
{
    Mat3 result;


    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            result.m[i][j] = m[i][j];
        }
    }


    return result;
}

bool Mat4::IsFinite() const
{

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {

            if (!std::isfinite(m[i][j]))
            {
                return false;
            }

        }
    }


    return true;
}


bool Mat4::TryInverse(Mat4& out, float epsilon) const
{
    if (!IsFinite())
        return false;

    const float eps = std::fabs(epsilon);

    float a[4][4];
    float inv[4][4];

    // 构造增广矩阵 [a | inv]
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            a[i][j] = m[i][j];
            inv[i][j] = (i == j) ? 1.0f : 0.0f;
        }
    }

    for (int col = 0; col < 4; ++col)
    {
        // 部分主元：在当前列找绝对值最大的行
        int pivot = col;
        float maxAbs = std::fabs(a[col][col]);

        for (int r = col + 1; r < 4; ++r)
        {
            float v = std::fabs(a[r][col]);
            if (v > maxAbs)
            {
                maxAbs = v;
                pivot = r;
            }
        }

        // 主元太小，认为矩阵奇异
        if (maxAbs <= eps)
            return false;

        // 交换行
        if (pivot != col)
        {
            for (int c = 0; c < 4; ++c)
            {
                float tmp = a[col][c];
                a[col][c] = a[pivot][c];
                a[pivot][c] = tmp;

                tmp = inv[col][c];
                inv[col][c] = inv[pivot][c];
                inv[pivot][c] = tmp;
            }
        }

        // 归一化主元行
        float diag = a[col][col];
        for (int c = 0; c < 4; ++c)
        {
            a[col][c] /= diag;
            inv[col][c] /= diag;
        }

        // 消去其他行当前列
        for (int r = 0; r < 4; ++r)
        {
            if (r == col)
                continue;

            float factor = a[r][col];
            if (factor == 0.0f)
                continue;

            for (int c = 0; c < 4; ++c)
            {
                a[r][c] -= factor * a[col][c];
                inv[r][c] -= factor * inv[col][c];
            }
        }
    }

    // 最后再检查一次结果是否有限
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            if (!std::isfinite(inv[i][j]))
                return false;
        }
    }

    // 成功后才写入 out，失败时 out 保持不变
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            out.m[i][j] = inv[i][j];
        }
    }

    return true;
}
