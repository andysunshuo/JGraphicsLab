#include "Application/ModelDemos.h"
#include "Assets/ResourcePaths.h"
#include <cstdio>
#include <stdexcept>
const char *ModelDemoName(ModelDemo demo)
{
    switch (demo)
    {
    case ModelDemo::Cube:
        return "cube";
    case ModelDemo::Teapot:
        return "teapot";
    case ModelDemo::Bunny:
        return "bunny";
    case ModelDemo::Dragon:
        return "dragon";
    default:
        return "无";
    }
}
static std::size_t DemoIndex(ModelDemo demo)
{
    const int i = static_cast<int>(demo) - 1;
    if (i < 0 || i >= 4)
        throw std::invalid_argument("无效模型演示编号");
    return static_cast<std::size_t>(i);
}
const Assets::MeshLoadResult &ModelDemoCache::Get(ModelDemo demo)
{
    const auto i = DemoIndex(demo);
    if (!attempts_[i])
    {
        ++attempts_[i];
        try
        {
            models_[i] =
                Assets::LoadObj(Assets::Resolve(std::string("Models/") + ModelDemoName(demo) + ".obj"));
            if (models_[i])
                fitted_[i] = MakeAutoFitModel(models_[i].mesh);
        }
        catch (const std::exception &e)
        {
            models_[i].error = e.what();
        }
        const auto &model = models_[i];
        if (model)
            std::printf("模型 %s：%zu 顶点，%zu 三角形，加载 %.3f 毫秒\n", ModelDemoName(demo),
                        model.mesh.vertices.size(), model.mesh.indices.size() / 3, model.milliseconds);
        else
            std::fprintf(stderr, "%s\n", model.error.c_str());
        if (!model.warning.empty())
            std::fprintf(stderr, "%s\n", model.warning.c_str());
    }
    return models_[i];
}
std::size_t ModelDemoCache::LoadAttempts(ModelDemo demo) const
{
    return attempts_[DemoIndex(demo)];
}
const Mat4 &ModelDemoCache::FittedModel(ModelDemo demo)
{
    if (!Get(demo))
        throw std::runtime_error(models_[DemoIndex(demo)].error);
    return fitted_[DemoIndex(demo)];
}
RenderStats RenderModelDemo(ModelDemoCache &cache, ModelDemo demo, float angleRadians)
{
    const auto &loaded = cache.Get(demo);
    if (!loaded)
        throw std::runtime_error(loaded.error);
    const Mat4 model = Multiply(MakeRotationY3D(angleRadians), cache.FittedModel(demo));
    return DrawMesh(loaded.mesh, model, Camera{});
}
