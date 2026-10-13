#pragma once
#include "Application/ModelDemos.h"
#include "Homework/AssetHomeworkIds.h"
// 默认作业选择入口：修改下面的编号，保存并重新编译后生效。
// 例如 17=三维向量数学测试，29=线框 Cube，30=裁剪演示，31=实心 Cube。
// 0=执行全部已有数学测试后绘制 HW31；命令参数 --hw N 会覆盖这个默认值。
// 原作业的使用方法和单步流程见 docs/快速上手与作业调试.md。
// 32～50 已完成，每章有数值检查与中文对照图；窗口中按左右键切换这 19 章。
// 例如 43=Gouraud/Phong 对照、47=双线性滤波、50=Mipmap 三线性滤波。
// 资源展示选择：50_1～50_4=四个模型，50_5～50_9=贴图，50_10=HDR 环境。
// 50_1 是本工程定义的 C++ 字面量，也可以写 HW50_1；详见 docs/HW50_资源展示.md。
inline constexpr int kActiveHomework = 46;
// Homework 模式保持 None；改成 Teapot 等枚举值则默认进入独立 OBJ 演示。
// 模型演示不占用 HW01～HW50 编号，--demo 名称也能在运行时选择。
inline constexpr ModelDemo kActiveModelDemo = ModelDemo::None;
