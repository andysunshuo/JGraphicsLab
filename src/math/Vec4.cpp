#include "Vec4.h"

#include <cmath>
#include <stdexcept>


// ============================================================
// Basic Operators
// ============================================================


Vec4 Vec4::operator+(const Vec4& other) const
{
    return Vec4{
        x + other.x,
        y + other.y,
        z + other.z,
        w + other.w
    };
}



Vec4 Vec4::operator-(const Vec4& other) const
{
    return Vec4{
        x - other.x,
        y - other.y,
        z - other.z,
        w - other.w
    };
}



// 取负

Vec4 Vec4::operator-() const
{
    return Vec4{
        -x,
        -y,
        -z,
        -w
    };
}



// 标量乘法

Vec4 Vec4::operator*(float scalar) const
{
    return Vec4{
        x * scalar,
        y * scalar,
        z * scalar,
        w * scalar
    };
}



// 标量除法

Vec4 Vec4::operator/(float scalar) const
{
    if (!std::isfinite(scalar) || scalar == 0)
    {
        throw std::invalid_argument(
            "Vec4 division: invalid scalar"
        );
    }


    return Vec4{
        x / scalar,
        y / scalar,
        z / scalar,
        w / scalar
    };
}



// ============================================================
// Vector Math
// ============================================================


// 点积

float Vec4::Dot(const Vec4& other) const
{
    return
        x * other.x +
        y * other.y +
        z * other.z +
        w * other.w;
}



// 判断有效

bool Vec4::IsFinite() const
{
    return
        std::isfinite(x) &&
        std::isfinite(y) &&
        std::isfinite(z) &&
        std::isfinite(w);
}



// ============================================================
// Homogeneous Coordinate
// ============================================================


// 创建位置
//
// xyz 表示空间位置
// w=1 表示 point

Vec4 Vec4::Position(const Vec3& position)
{
    return Vec4{
        position.x,
        position.y,
        position.z,
        1.0f
    };
}



// 创建方向
//
// xyz 表示方向
// w=0 表示 vector

Vec4 Vec4::Direction(const Vec3& direction)
{
    return Vec4{
        direction.x,
        direction.y,
        direction.z,
        0.0f
    };
}



// ============================================================
// Lerp
// ============================================================


Vec4 Vec4::Lerp(
    const Vec4& a,
    const Vec4& b,
    float t)
{
    return a + (b - a) * t;
}



// ============================================================
// Perspective Divide
// ============================================================


// x,y,z 除以 w
//
// Clip Space
//
//        |
//        v
//
// NDC Space
//
// x'=x/w
// y'=y/w
// z'=z/w


bool Vec4::TryPerspectiveDivide(
    Vec3& out,
    float epsilon) const
{

    if (!IsFinite())
    {
        return false;
    }


    if (std::abs(w) <= epsilon)
    {
        return false;
    }


    out = Vec3{
        x / w,
        y / w,
        z / w
    };


    return true;
}




Vec4 Vec4::PerspectiveDivide() const
{

    Vec3 result;


    if (!TryPerspectiveDivide(result))
    {
        throw std::invalid_argument(
            "Vec4 PerspectiveDivide failed"
        );
    }


    return Vec4{
        result.x,
        result.y,
        result.z,
        1.0f
    };
}



// ============================================================
// scalar * Vec4
// ============================================================


Vec4 operator*(float scalar, const Vec4& value)
{
    return value * scalar;
}