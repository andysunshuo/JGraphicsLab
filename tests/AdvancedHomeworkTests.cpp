#include "Application/Application.h"
#include "Homework/LearningDemos.h"
#include "Homework/Registry.h"
#include "Homework/Runner.h"
#include "Assets/ResourcePaths.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <limits>
#include <stdexcept>

namespace
{
int failures = 0;
void Check(bool ok, const char *text)
{
    std::printf("%s：%s\n", ok ? "PASS" : "FAIL", text);
    if (!ok)
        ++failures;
}
} // namespace
int main()
{
    try
    {
        const auto output =
            std::filesystem::current_path() / "validation" / JRENDERLAB_TEST_CONFIG / "homework32_50";
        std::filesystem::create_directories(output);
        int passed = 0, failed = 0;
        for (int number = 32; number <= 50; ++number)
        {
            const auto *hw = FindHomework(number);
            Check(hw && hw->test && hw->render && hw->status == HomeworkStatus::Implemented,
                  "新作业同时注册独立测试和绘制入口");
            const auto result = RunMathHomeworkTest(number);
            passed += result.passed;
            failed += result.failed;
            Check(result.passed > 0 && result.failed == 0, "该章真实数学检查通过");
            auto options = ParseArguments({"--hw", std::to_string(number)});
            options.output = output / ("HW" + std::to_string(number) + ".ppm");
            Check(InitializeApplication(options), "该章经过实际应用初始化并成功绘制");
            const auto frame = GetApplicationDisplaySurface();
            Check(frame.width == 600 && frame.height == 600, "该章提供中文学习对照画布");
            std::size_t content = 0;
            for (int y = 125; y < 535; ++y)
                for (int x = 15; x < 585; ++x)
                    content +=
                        ((frame.pixels[y * frame.width + x] & 0xffffffu) != Raster::MakeColor(18, 22, 30));
            Check(content > 4000, "图像主体实际有计算结果而不只是文字");
            Check(std::filesystem::file_size(options.output) > 600 * 600 * 3, "完整画布导出成功");
        }
        using namespace RenderLearning;
        // 同一张真实地面，三线性滤波应改变基础级混叠图，而不是仅改变标题。
        PipelineOptions base, filtered;
        base.shading = filtered.shading = Shading::Texture;
        base.texture = filtered.texture = &HomeworkLearning::CheckerTexture();
        base.filter = TextureFilter::Bilinear;
        filtered.filter = TextureFilter::Trilinear;
        filtered.mipmaps = &HomeworkLearning::CheckerMipmaps();
        const auto a = HomeworkLearning::RenderGround(base), b = HomeworkLearning::RenderGround(filtered);
        double difference = 0;
        for (std::size_t i = 0; i < a.pixels.size(); ++i)
            difference += Length(Subtract(a.pixels[i], b.pixels[i]));
        Check(difference > 100, "三线性采样确实使用 Mipmap 改变远处纹理结果");
        // 越界、CW、非有限、属性裁剪的集成安全检查。
        RenderTarget target(8, 8, 4);
        PipelineOptions flat;
        flat.shading = Shading::Flat;
        PipelineStats stats;
        PipelineVertex v{{1e30f, 1e30f}, 0, 1, {{}, {}, {1, 0, 0}, {}}};
        RasterizeAttributedTriangle(v, v, v, target, flat, stats);
        Check(stats.depthWrites == 0, "巨大或退化屏幕三角形不产生越界写入");
        PipelineVertex invalidA{{0, 0}, 0, 1, {{}, {0, 0, 1}, {1, 0, 0}, {}}}, invalidB = invalidA,
            invalidC = invalidA;
        invalidB.screen = {8, 0};
        invalidC.screen = {0, 8};
        invalidA.attributes.uv.x = std::numeric_limits<float>::quiet_NaN();
        RasterizeAttributedTriangle(invalidA, invalidB, invalidC, target, flat, stats);
        Check(stats.depthWrites == 0, "非有限顶点属性在光栅化前被拒绝");
        Mesh broken = MakeUVTriangle();
        broken.indices[0] = 999;
        bool invalidIndex = false;
        try
        {
            DrawMesh(broken, IdentityMat4(), Camera{}, flat, target);
        }
        catch (const std::invalid_argument &)
        {
            invalidIndex = true;
        }
        Check(invalidIndex, "非法 Mesh 索引给出明确错误");
        const ClipVertex nearA{{-0.5f, -0.5f, -2, 1}, {}, {}, {0, 0, 1}, {0, 0}},
            nearB{{0.5f, -0.5f, 0, 1}, {}, {}, {0, 0, 1}, {1, 0}},
            nearC{{0, 0.5f, 0, 1}, {}, {}, {0, 0, 1}, {0.5f, 1}};
        const auto polygon = ClipTriangleToFrustum(nearA, nearB, nearC);
        bool allInside = polygon.size() == 4, intersection = false;
        for (const auto &p : polygon)
        {
            allInside &= p.position.z >= -p.position.w;
            intersection |=
                std::abs(p.position.z + 1) < 1e-6f && std::abs(p.uv.x - 0.5f) < 1e-6f && p.uv.y == 0;
        }
        Check(allInside && intersection, "原六面裁剪正确保留新增 UV 属性和近面交点");
        bool rejected = false;
        try
        {
            RenderTarget invalid(2048, 2048, 4);
        }
        catch (const std::invalid_argument &)
        {
            rejected = true;
        }
        Check(rejected, "多采样内存预算生效");
        ApplicationOptions legacy;
        legacy.homework = 31;
        legacy.demo = ModelDemo::None;
        Check(InitializeApplication(legacy) && GetApplicationDisplaySurface().width == 100,
              "可从新画布切回原 HW31");
        auto asset = ParseArguments({"--hw", "50_8"});
        Check(InitializeApplication(asset) && GetApplicationDisplaySurface().width == 600,
              "原下划线资源入口仍能运行");
        std::printf("HW32～HW50 数学检查：%d PASS，%d FAIL；集成失败：%d\n", passed, failed, failures);
        return failures || failed ? 1 : 0;
    }
    catch (const std::exception &e)
    {
        std::fprintf(stderr, "新作业验收异常：%s\n", e.what());
        return 1;
    }
}
