#include "Application/Application.h"
#include "Homework/Runner.h"
#include "Assets/ResourcePaths.h"
#include "Homework/AssetHomework.h"
#include "Homework/LearningDemos.h"
#include "Renderer/Framebuffer.h"
#include <cstdio>
#include <stdexcept>
namespace
{
ModelDemoCache cache;
const AssetPreviewFrame *assetFrame = nullptr;
} // namespace
ApplicationOptions ParseArguments(const std::vector<std::string> &arguments)
{
    ApplicationOptions options;
    for (std::size_t i = 0; i < arguments.size(); ++i)
    {
        const auto &arg = arguments[i];
        const auto value = [&]() -> std::string {
            if (++i >= arguments.size())
                throw std::invalid_argument("命令参数缺少值：" + arg);
            return arguments[i];
        };
        if (arg == "--hw")
        {
            const std::string v = value();
            std::size_t used = 0;
            const int assetId = ParseAssetHomeworkId(v);
            if (assetId)
                options.homework = assetId;
            else
            {
                options.homework = std::stoi(v, &used);
                if (used != v.size() || options.homework < 0 || options.homework > 50)
                    throw std::invalid_argument("作业编号应为 0～50，或 50_1～50_10");
            }
            options.demo = ModelDemo::None;
        }
        else if (arg == "--demo")
        {
            const std::string name = value();
            bool found = false;
            for (ModelDemo demo : {ModelDemo::Cube, ModelDemo::Teapot, ModelDemo::Bunny, ModelDemo::Dragon})
                if (name == ModelDemoName(demo))
                {
                    options.demo = demo;
                    found = true;
                }
            if (!found)
                throw std::invalid_argument("模型名称应为 cube、teapot、bunny 或 dragon");
        }
        else if (arg == "--all-tests")
            options.allTests = true;
        else if (arg == "--list")
            options.list = true;
        else if (arg == "--headless")
            options.headless = true;
        else if (arg == "--animate")
            options.animate = true;
        else if (arg == "--output")
            options.output = std::filesystem::u8path(value());
        else
            throw std::invalid_argument("未知命令参数：" + arg);
    }
    return options;
}
bool RenderApplicationFrame(const ApplicationOptions &options, float angle)
{
    try
    {
        assetFrame = nullptr;
        if (options.demo == ModelDemo::None)
            if (const auto *asset = FindAssetHomework(options.homework))
            {
                assetFrame = &asset->render(angle);
                return true;
            }
        if (options.demo == ModelDemo::None)
        {
            const bool result = RunHomework(options.homework, false);
            if (result && options.homework >= 32 && options.homework <= 50)
                assetFrame = HomeworkLearning::DisplayFor(options.homework);
            return result;
        }
        Raster::Clear(Raster::MakeColor(0, 0, 0));
        const auto stats = RenderModelDemo(cache, options.demo, angle);
        static ModelDemo last = ModelDemo::None;
        if (last != options.demo)
        {
            std::printf("模型 %s：渲染 %.3f 毫秒，提交 %zu 三角形，光栅化 %zu，拒绝 %zu\n",
                        ModelDemoName(options.demo), stats.milliseconds, stats.inputTriangles,
                        stats.rasterizedTriangles, stats.rejectedTriangles);
            last = options.demo;
        }
        return true;
    }
    catch (const std::exception &e)
    {
        std::fprintf(stderr, "渲染失败：%s\n", e.what());
        return false;
    }
}
bool InitializeApplication(const ApplicationOptions &options)
{
    if (options.list)
    {
        ListHomework();
        ListAssetHomework();
    }
    if (options.allTests)
        return RunAllMathTests().failed == 0;
    if (options.list)
        return true;
    const Homework *hw = FindHomework(options.homework == 0 ? 31 : options.homework);
    const auto *asset = FindAssetHomework(options.homework);
    if (!hw && !asset)
    {
        std::fprintf(stderr, "无效作业编号：%d\n", options.homework);
        return false;
    }
    if (hw && options.demo == ModelDemo::None && (options.homework == 0 || hw->test))
        if (RunMathHomeworkTest(options.homework).failed)
            return false;
    if (hw && options.demo == ModelDemo::None && hw->status == HomeworkStatus::DemoOnly)
        std::printf("HW%02d：%s\n", hw->number, HomeworkStatusName(hw->status));
    if (!RenderApplicationFrame(options))
        return false;
    if (!options.output.empty() &&
        !(assetFrame ? AssetPreview::SavePPM(*assetFrame, options.output) : Raster::SavePPM(options.output)))
    {
        std::fprintf(stderr, "无法保存图像：%s\n", Assets::PathUtf8(options.output).c_str());
        return false;
    }
    return true;
}
void RenderScene()
{
    InitializeApplication(ApplicationOptions{});
}
ApplicationDisplaySurface GetApplicationDisplaySurface()
{
    if (assetFrame)
        return {assetFrame->pixels.data(), assetFrame->width, assetFrame->height};
    return {Raster::Pixels(), Raster::WIDTH, Raster::HEIGHT};
}
std::wstring ApplicationWindowTitle(const ApplicationOptions &options)
{
    if (options.demo == ModelDemo::None && options.homework >= 32 && options.homework <= 50)
        if (const auto *hw = FindHomework(options.homework))
            return L"JRenderLab — HW" + std::to_wstring(options.homework) + L" " +
                   std::filesystem::u8path(hw->name).wstring();
    if (options.demo == ModelDemo::None)
        if (const auto *asset = FindAssetHomework(options.homework))
            return L"JRenderLab — " +
                   std::wstring(asset->key, asset->key + std::char_traits<char>::length(asset->key)) + L" " +
                   asset->title;
    return L"JGraphicsLab";
}
