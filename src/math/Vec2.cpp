#include "Vec2.h"

#include <cmath>
#include <stdexcept>



Vec2 Vec2::operator+(const Vec2& other) const
{
    return Vec2{ this->x + other.x, this->y + other.y };
}
Vec2 Vec2::operator-(const Vec2& other) const
{
	return Vec2{this->x - other.x, this->y - other.y };
}
Vec2 Vec2::operator-() const
{
	return Vec2{ -x , -y };
}
Vec2 Vec2::operator*(float scalar) const
{
	return Vec2{ x * scalar, y * scalar };
}
// 除数为零或非有限时抛 std::invalid_argument。
Vec2  Vec2::operator/(float scalar) const 
{
     // isfinite 判断 是不是一个有效数字
    if (!std::isfinite(scalar) || scalar == 0)
    {
        throw std::invalid_argument(
            "Vec2 division: invalid scalar"
        );
    }

	return Vec2{ x / scalar, y / scalar };

}
Vec2& Vec2::operator+=(const Vec2& other)
{
    x = x + other.x;
    y =y+ other.y ;
    return *this;
}
Vec2& Vec2::operator-=(const Vec2& other)
{
    x = x - other.x;
    y = y - other.y;
    return *this;
}

// 点积 x*other.x+y*other.y；二维叉积返回有向面积的标量。
float  Vec2::Dot(const Vec2& other) const 
{
    return x * other.x + y * other.y;
}

//叉乘只存在于：三维 . 二维没有真正叉乘。
//严格数学：二维没有。叉乘需要 z
// 返回标量，表示两个二维向量的有向面积
float  Vec2::Cross(const Vec2& other) const 
{
    // 图形学定义叉乘
    return x * other.y - y * other.x;
}

float  Vec2::LengthSquared() const 
{
    return x * x + y * y;
}

float  Vec2::Length() const 
{
    return std::sqrt( x*x + y*y);
}

// 在类内部： Length()  等价于 this->Length()

// 先算长度；长度不超过 epsilon 返回零向量，避免除零。
Vec2 Vec2::Normalized(float epsilon) const
{
    // 因为乘法比除法快一点。 所以 不用除法。
    float lengthSquared = LengthSquared();

    if (lengthSquared <= epsilon * epsilon)
    {
        return Vec2{ 0,0 };
    }

    float invLength = 1.0f / sqrt(lengthSquared);

    return Vec2{
        x * invLength,
        y * invLength
    };
    // return Vec2{x / length, y / length};
}

// 判断 xy是否有效，即向量是否有效
bool Vec2::IsFinite() const
{
    return std::isfinite(x) &&
        std::isfinite(y);
}

// 插值 a+(b-a)*t；不强制把 t 钳制到 [0,1]，允许外插。
// 在两个点之间按照比例 t 找一个位置。
Vec2 Vec2::Lerp(const Vec2& a, const Vec2& b, float t)
{
    return a + (b - a) * t;
}


// 实现时直接转调 value*scalar，不再复制数乘公式。
Vec2 operator*(float scalar, const Vec2& value)
{
    return Vec2{ value.x * scalar, value.y * scalar };
}