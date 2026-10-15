#pragma once

// 在 Vec3.cpp 实现；采用右手系，Cross 的参数顺序会决定方向。
struct Vec3
{
    float x = 0, y = 0, z = 0;
    // 这两个构造函数已定义；使用 {1,2,3} 会调用值构造，不需要另加 cpp 构造实现。
    Vec3() = default;

    Vec3(float x, float y, float z) : x(x), y(y), z(z)
    {
    }
    Vec3 operator+(const Vec3 &other) const;
    Vec3 operator-(const Vec3 &other) const;
    Vec3 operator-() const;
    Vec3 operator*(float scalar) const;
    // 除数为零或非有限时抛 std::invalid_argument。
    Vec3 operator/(float scalar) const;
    Vec3 &operator+=(const Vec3 &other);
    Vec3 &operator-=(const Vec3 &other);

    // 点积为 xx'+yy'+zz'；叉积为 (yz'-zy',zx'-xz',xy'-yx')。
    float Dot(const Vec3 &other) const;
    Vec3 Cross(const Vec3 &other) const;
    float LengthSquared() const;
    float Length() const;
    // 待补：epsilon 有限且非负；非有限向量/非法 epsilon 抛 invalid_argument。
    // 零向量返回零；用最大绝对分量先缩放再计算长度，避免 1e30 的平方溢出。
    // 不能对 Inf/NaN 继续平方与乘倒数，也不能将大有限向量错误归一化为零。
    Vec3 Normalized(float epsilon = 1e-8f) const;
    bool IsFinite() const;
    static Vec3 Lerp(const Vec3 &a, const Vec3 &b, float t);
    // 颜色相乘是逐通道相乘，与 Dot 和标量乘法分别处理。
    Vec3 Hadamard(const Vec3 &other) const;
};

Vec3 operator*(float scalar, const Vec3 &value);
