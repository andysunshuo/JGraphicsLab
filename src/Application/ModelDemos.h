#pragma once
#include "Assets/ObjLoader.h"
#include "Renderer/MeshRenderer.h"
#include <array>
enum class ModelDemo
{
    None,
    Cube,
    Teapot,
    Bunny,
    Dragon
};
const char *ModelDemoName(ModelDemo demo);
class ModelDemoCache
{
  public:
    const Assets::MeshLoadResult &Get(ModelDemo demo);
    std::size_t LoadAttempts(ModelDemo demo) const;
    const Mat4 &FittedModel(ModelDemo demo);

  private:
    std::array<Assets::MeshLoadResult, 4> models_;
    std::array<std::size_t, 4> attempts_{};
    std::array<Mat4, 4> fitted_{};
};
RenderStats RenderModelDemo(ModelDemoCache &cache, ModelDemo demo, float angleRadians = 0.4f);
