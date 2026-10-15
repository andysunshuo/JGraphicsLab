#include "Vec3.h"

#include <cmath>
#include <stdexcept>

// ============================================================
// 基础运算
// ============================================================


Vec3 Vec3::operator+(const Vec3& other) const
{
    return Vec3{
        x + other.x,
        y + other.y,
        z + other.z
    };
}


Vec3 Vec3::operator-(const Vec3& other) const
{
    return Vec3{
        x - other.x,
        y - other.y,
        z - other.z
    };
}


// 向量取反
// (x,y,z) -> (-x,-y,-z)
Vec3 Vec3::operator-() const
{
    return Vec3{
        -x,
        -y,
        -z
    };
}


// 向量乘标量
// v * scalar
Vec3 Vec3::operator*(float scalar) const
{
    return Vec3{
        x * scalar,
        y * scalar,
        z * scalar
    };
}


// 向量除标量
Vec3 Vec3::operator/(float scalar) const
{
    if (!std::isfinite(scalar) || scalar == 0)
    {
        throw std::invalid_argument(
            "Vec3 division: invalid scalar"
        );
    }


    return Vec3{
        x / scalar,
        y / scalar,
        z / scalar
    };
}


// ============================================================
// 复合赋值
// ============================================================


Vec3& Vec3::operator+=(const Vec3& other)
{
    x += other.x;
    y += other.y;
    z += other.z;

    return *this;
}


Vec3& Vec3::operator-=(const Vec3& other)
{
    x -= other.x;
    y -= other.y;
    z -= other.z;

    return *this;
}



// ============================================================
// Vector Math
// ============================================================


// 点积
//
// a dot b = axbx + ayby + azbz
//
// 用途：
// 1. 判断角度
// 2. Lambert 光照
// 3. 投影

float Vec3::Dot(const Vec3& other) const
{
    return
        x * other.x +
        y * other.y +
        z * other.z;
}



// 叉积
//
// Cross:
//
// ( y1*z2-z1*y2,
//   z1*x2-x1*z2,
//   x1*y2-y1*x2 )
//
// 结果:
// 一个同时垂直于 a,b 的向量
//
Vec3 Vec3::Cross(const Vec3& other) const
{
    return Vec3{

        y * other.z - z * other.y,

        z * other.x - x * other.z,

        x * other.y - y * other.x

    };
}



// 长度平方
//
// 避免 sqrt
// 常用于比较距离

float Vec3::LengthSquared() const
{
    return
        x * x +
        y * y +
        z * z;
}



// 长度

float Vec3::Length() const
{
    return std::sqrt(
        LengthSquared()
    );
}



// ============================================================
// Normalize
// ============================================================


Vec3 Vec3::Normalized(float epsilon) const
{
    float lengthSquared = LengthSquared();


    if (lengthSquared <= epsilon * epsilon)
    {
        return Vec3{ 0,0,0 };
    }


    float invLength = 1.0f / std::sqrt(lengthSquared);


    return Vec3{
        x * invLength,
        y * invLength,
        z * invLength
    };
}



// ============================================================
// Utility
// ============================================================


// 判断是否包含 NaN / Inf

bool Vec3::IsFinite() const
{
    return
        std::isfinite(x) &&
        std::isfinite(y) &&
        std::isfinite(z);
}



// 线性插值
//
// a + (b-a)t
//
// t=0 -> a
// t=1 -> b

Vec3 Vec3::Lerp(
    const Vec3& a,
    const Vec3& b,
    float t)
{
    return a + (b - a) * t;
}



// ============================================================
// Color / Material
// ============================================================


// 分量乘法
//
// (r1,g1,b1)*(r2,g2,b2)
//
// =
//
// (r1*r2,
//  g1*g2,
//  b1*b2)
//
// 用于：
// 光照颜色
// 材质颜色
// Texture

Vec3 Vec3::Hadamard(const Vec3& other) const
{
    return Vec3{

        x * other.x,

        y * other.y,

        z * other.z

    };
}



// ============================================================
// scalar * Vec3
// ============================================================

Vec3 operator*(float scalar, const Vec3& value)
{
    return value * scalar;
}