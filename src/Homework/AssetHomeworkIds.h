#pragma once

// 内部值与原 HW01～HW50 分离；对外始终使用 HW50_1～HW50_10 命名。
inline constexpr int HW50_1 = 5001;
inline constexpr int HW50_2 = 5002;
inline constexpr int HW50_3 = 5003;
inline constexpr int HW50_4 = 5004;
inline constexpr int HW50_5 = 5005;
inline constexpr int HW50_6 = 5006;
inline constexpr int HW50_7 = 5007;
inline constexpr int HW50_8 = 5008;
inline constexpr int HW50_9 = 5009;
inline constexpr int HW50_10 = 5010;

// 自定义整数字面量：让配置中的 50_1、50_10 与命名常量完全等价。
// 前缀必须是 50；例如误写 31_1 时，常量求值会在编译期间报错。
#define JRENDERLAB_ASSET_LITERAL(suffix, id)                                                                 \
    constexpr int operator"" suffix(unsigned long long base)                                                 \
    {                                                                                                        \
        if (base != 50)                                                                                      \
            throw "资源展示字面量的前缀必须是 50";                                                           \
        return id;                                                                                           \
    }
JRENDERLAB_ASSET_LITERAL(_1, HW50_1)
JRENDERLAB_ASSET_LITERAL(_2, HW50_2)
JRENDERLAB_ASSET_LITERAL(_3, HW50_3)
JRENDERLAB_ASSET_LITERAL(_4, HW50_4)
JRENDERLAB_ASSET_LITERAL(_5, HW50_5)
JRENDERLAB_ASSET_LITERAL(_6, HW50_6)
JRENDERLAB_ASSET_LITERAL(_7, HW50_7)
JRENDERLAB_ASSET_LITERAL(_8, HW50_8)
JRENDERLAB_ASSET_LITERAL(_9, HW50_9)
JRENDERLAB_ASSET_LITERAL(_10, HW50_10)
#undef JRENDERLAB_ASSET_LITERAL
