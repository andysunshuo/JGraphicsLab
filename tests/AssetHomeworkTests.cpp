#include "Homework/AssetHomework.h"
#include "Homework/Registry.h"
#include "Application/Application.h"
#include "Assets/ResourcePaths.h"
#include "Assets/HdrLoader.h"
#include "Renderer/Framebuffer.h"
#include <algorithm>
#include <cstdio>
#include <fstream>
#include <set>
#include <stdexcept>

// 直接验证用户要求的配置写法在 C++20/MSVC 中可编译并映射到正确入口。
static_assert(50_1 == HW50_1 && 50_2 == HW50_2 && 50_3 == HW50_3 && 50_4 == HW50_4);
static_assert(50_5 == HW50_5 && 50_6 == HW50_6 && 50_7 == HW50_7 && 50_8 == HW50_8);
static_assert(50_9 == HW50_9 && 50_10 == HW50_10);
namespace
{
int failures = 0;
void Check(bool ok, const char *description)
{
    std::printf("%s：%s\n", ok ? "PASS" : "FAIL", description);
    if (!ok)
        ++failures;
}
} // namespace
int main()
{
    try
    {
        const auto output =
            std::filesystem::current_path() / "validation" / JRENDERLAB_TEST_CONFIG / "assets";
        std::filesystem::create_directories(output);
        std::set<int> ids;
        for (const auto &entry : AssetHomeworkRegistry())
        {
            ids.insert(entry.id);
            Check(!FindHomework(entry.id), "扩展编号不占用原 HW01～HW50 注册表");
            const std::string shortName = std::string(entry.key).substr(2);
            Check(ParseArguments({"--hw", shortName}).homework == entry.id, "--hw 50_下划线编号解析正确");
            Check(ParseArguments({"--hw", entry.key}).homework == entry.id, "--hw HW50_名称解析正确");
            ApplicationOptions options;
            options.homework = entry.id;
            options.demo = ModelDemo::None;
            options.output = output / (std::string(entry.key) + ".ppm");
            Check(InitializeApplication(options), "资源展示经过实际 Application 初始化及显示选择");
            const auto frame = GetApplicationDisplaySurface();
            Check(frame.width == 600 && frame.height == 600 && frame.pixels,
                  "新资源使用独立 600×600 展示画布");
            std::size_t content = 0;
            for (int y = 102; y < 550; ++y)
                for (int x = 30; x < 570; ++x)
                    if ((frame.pixels[y * frame.width + x] & 0xffffffu) != Raster::MakeColor(18, 22, 30))
                        ++content;
            Check(content > 5000, "主体图像区域实际包含模型或图片，而不只是文字标题");
            Check(AssetHomeworkLoadCount(entry.id) == 1, "每个 OBJ、图像或 HDR 首次只加载一次");
            const auto firstPixels = frame.pixels;
            Check(RenderApplicationFrame(options), "重复运行资源展示入口成功");
            Check(GetApplicationDisplaySurface().pixels == firstPixels &&
                      AssetHomeworkLoadCount(entry.id) == 1,
                  "重复显示复用画布与加载缓存");
            Check(std::filesystem::file_size(options.output) > 600 * 600 * 3, "图像导出包含完整说明画布");
        }
        Check(ids.size() == 10, "十个下划线入口唯一且全部可运行");
        for (const std::string name : {"50_0", "50_11", "51_1", "5001"})
        {
            bool rejected = false;
            try
            {
                (void)ParseArguments({"--hw", name});
            }
            catch (const std::exception &)
            {
                rejected = true;
            }
            Check(rejected, "错误下划线编号或内部编号不被误认为原作业");
        }
        const auto &entry = AssetHomeworkRegistry()[0];
        (void)entry.render(0.8f);
        Check(AssetHomeworkLoadCount(entry.id) == 1, "模型旋转仍复用同一 OBJ");
        Assets::HdrImage image;
        std::string error;
        Check(Assets::LoadHdr(Assets::Resolve("Environment/studio_small_09.hdr"), image, error),
              "准备好的 HDR 可解码为浮点亮度");
        float peak = 0;
        for (const auto &rgb : image.pixels)
            peak = std::max({peak, rgb.x, rgb.y, rgb.z});
        Check(image.width == 1024 && image.height == 512 && peak > 1, "HDR 保留高于 1 的亮度与 2:1 全景尺寸");
        Check(!Assets::LoadHdr(output / L"不存在.hdr", image, error) && !error.empty(),
              "缺失 HDR 返回中文路径错误");
        Check(!Assets::LoadHdr(Assets::Resolve("Textures/checker.png"), image, error),
              "不会把普通 PNG 当成 HDR");
        const auto malformed = output / "malformed.hdr";
        std::ofstream(malformed, std::ios::binary)
            << "#?RADIANCE\nFORMAT=32-bit_rle_rgbe\n\n-Y 512 +X 1024\n";
        Check(!Assets::LoadHdr(malformed, image, error), "截断 HDR 解码安全失败");
        ApplicationOptions old;
        old.homework = 31;
        old.demo = ModelDemo::None;
        Check(InitializeApplication(old), "展示资源后仍可回到原 HW31");
        Check(GetApplicationDisplaySurface().width == 100 && GetApplicationDisplaySurface().height == 100,
              "原作业继续使用原 100×100 帧缓冲");
        std::printf("资源展示验收失败数：%d\n", failures);
        return failures ? 1 : 0;
    }
    catch (const std::exception &e)
    {
        std::fprintf(stderr, "资源展示验收异常：%s\n", e.what());
        return 1;
    }
}
