/*
【HW49：Mipmap 生成，已实现】
选择：Config.h 的 kActiveHomework=49，或 --hw 49。
调用：InitializeApplication -> TestHW49 的真实数值检查；通过后 Runner -> RenderHW49。
具体绘图流程：RenderHW49 -> CheckerMipmaps -> BuildMipmaps（缓存）-> Downsample 直到 1×1 -> Publish 全部级别。
结果由 Publish 组织成独立 600×600 中文对照画布。
数学与计算过程：
从基础图像逐级用 2×2 线性颜色均值缩小，直到 1×1，保存真正的多级图像。
奇数边长使用向上取整，边缘块只平均存在的像素，避免越界。
规则尺寸链 256→128→64→...→1；画布显示全部九级的实际数据而非仅缩放原图。
计算示例：256 图像连续减半八次，加上基础级共 9 级；3×5 按向上取整得到 2×3、1×2、1×1。
断点：本文件 Test/Render 和公共 Renderer/ImageProcessing、LearningPipeline、Geometry/Surface。
与原作业共用向量、矩阵、齐次裁剪、深度比较和 Top-left 规则；不更改 HW01～HW31 的输入。
*/
#include "Homework/LearningDemos.h"
#include "Renderer/Interpolation.h"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>
#include <cstdio>
using namespace HomeworkLearning;

// 数学入口：计算实际结果，与明确的预期比较，返回真实 PASS/FAIL 数量。
TestResult TestHW49()
{
    MathTestContext checks(49);
    Image image(4, 4, {0.2f, 0.4f, 0.6f});
    const auto levels = BuildMipmaps(image);
    checks.check(levels.size() == 3 && levels[1].width == 2 && levels[2].width == 1, "4→2→1 的真实级别链");
    checks.check(checks.near3(levels.back().At(0, 0), {0.2f, 0.4f, 0.6f}), "常量图在所有级别保留线性颜色");
    const auto odd = BuildMipmaps(Image(3, 5, {1, 1, 1}));
    checks.check(odd.size() == 4 && odd[1].width == 2 && odd[1].height == 3 && odd.back().height == 1,
                 "奇数尺寸完整下采样至 1×1");
    checks.check(CheckerMipmaps().size() == 9, "实际 256 棋盘图具有九级 Mipmap");
    std::printf("[HW49] 结果：%d PASS，%d FAIL\n", checks.result.passed, checks.result.failed);
    return checks.result;
}

// 图形入口：按注释标明的算法生成实际对照图，不使用静态图片冒充算法结果。
void RenderHW49()
{
    std::vector<Panel> panels;
    const auto &levels = CheckerMipmaps();
    for (std::size_t i = 0; i < levels.size(); ++i)
        panels.push_back({levels[i], L"L" + std::to_wstring(i) + L" " + std::to_wstring(levels[i].width) +
                                         L"×" + std::to_wstring(levels[i].height)});
    Publish(49, L"Mipmap 金字塔", L"逐级低通平均；各格显示该级真实纹理", std::move(panels));
}
