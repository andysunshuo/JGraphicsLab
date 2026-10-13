#pragma once
#include "Homework/Entries.h"
#include "Renderer/LearningPipeline.h"
#include "Renderer/AssetPreview.h"
#include "Geometry/Surface.h"

namespace HomeworkLearning
{
using namespace RenderLearning;
struct Panel
{
    Image image;
    std::wstring label;
};
void Publish(int number, const std::wstring &name, const std::wstring &principle, std::vector<Panel> panels);
const AssetPreviewFrame *DisplayFor(int number);
const Mesh &Sphere();
const Mesh &CoarseSphere();
const Mesh &Cube();
const Mesh &Ground();
const Image &ColorTexture();
// 纹理、Mipmap 和场景几何使用函数内静态缓存，切换章节时复用。
const Image &CheckerTexture();
const std::vector<Image> &CheckerMipmaps();
Image Render(const Mesh &mesh, PipelineOptions options = {}, Mat4 model = IdentityMat4(),
             int resolution = 256, int samples = 1);
Image RenderGround(PipelineOptions options);
Image Magnify(const Image &source, bool bilinear, int size = 256);
Image Decimate(const Image &source, int factor);
Image Occlusion(int samples, int size = 96);
} // namespace HomeworkLearning
