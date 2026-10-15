#pragma once

// 在 Vec2.cpp 实现下面的方法；除已有构造函数外，头文件只保存数据和声明。
// 你已加入默认/值构造函数：Vec2{1,2} 仍可用，但它现在是构造调用，不再是聚合初始化。
// 下方构造函数已有头文件内定义，不要再在 Vec2.cpp 定义同一签名。
struct Vec2
{
    float x = 0, y = 0;

    // 默认
    Vec2() : x(0), y(0)
    {
    }
    Vec2(float x, float y) : x(x), y(y)
    {
    }
    // 分量相加/相减；负号同时改变两个分量，不能只改变长度。
    Vec2 operator+(const Vec2 &other) const;
    Vec2 operator-(const Vec2 &other) const;
    Vec2 operator-() const;
    Vec2 operator*(float scalar) const;
    // 除数为零或非有限时抛 std::invalid_argument。
    Vec2 operator/(float scalar) const;
    Vec2 &operator+=(const Vec2 &other);
    Vec2 &operator-=(const Vec2 &other);

    // 点积 x*other.x+y*other.y；二维叉积返回有向面积的标量。
    float Dot(const Vec2 &other) const;
    float Cross(const Vec2 &other) const;
    float LengthSquared() const;
    float Length() const;
    // 实现待补安全检查：epsilon 必须有限且非负，向量必须有限；非法输入抛 invalid_argument。
    // 零向量或长度不超过 epsilon 返回零。大分量直接平方可能溢出：
    // 建议先除以 max(abs(x),abs(y))，在缩放后的范围计算长度，再归一化。
    // LengthSquared 的 Inf 不表示原输入一定非有限，不能据此直接把大有限向量变成零。
    Vec2 Normalized(float epsilon = 1e-8f) const;
    bool IsFinite() const;
    // 插值 a+(b-a)*t；不强制把 t 钳制到 [0,1]，允许外插。
    static Vec2 Lerp(const Vec2 &a, const Vec2 &b, float t);
};

// 实现时直接转调 value*scalar，不再复制数乘公式。
// 是一个 普通函数（全局函数），
// 非成员函数 实现的是： 5 * Vec2；当前 cpp 手写分量是正确的，可随后改为转调减少重复。
Vec2 operator*(float scalar, const Vec2 &value);
