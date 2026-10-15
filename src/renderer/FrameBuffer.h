#pragma once
#include "RasterWindow.h"
#include <cstddef>
#include <vector>

// 此动态类保留为后续练习，尚未实现；当前基础管线使用 FrameBuffer.cpp 的 Raster 固定缓冲。
// 深度与样本采用左下原点；显示颜色数组从上到下，Data() 可交给窗口绘制。
class FrameBuffer
{
  public:
    // 先校验再分配：单边 1～2048、samples 只能 1/4、总样本最多 4*1024*1024。
    // 分配 sampleColors/sampleDepths 与 width*height 的显示颜色，然后清空。
    explicit FrameBuffer(int width = Raster::WIDTH, int height = Raster::HEIGHT, int samples = 1);
    void Resize(int width, int height, int samples = 1);
    void Clear(const Vec3 &linearColor = {0, 0, 0}); // 只清颜色；同步更新显示数组。
    void ClearDepth(float depth = 1.0f);

    int Width() const;
    int Height() const;
    int Samples() const;
    bool Contains(int x, int y) const;
    // 检查坐标/样本/深度有限和 [0,1]；严格 depth<saved 时更新并返回 true。
    // 相等深度不覆盖，失败不改原值；禁止把四样本共用一份深度。
    bool DepthTest(int x, int y, float depth, int sample = 0);
    float GetDepth(int x, int y, int sample = 0) const;
    void WriteSample(int x, int y, int sample, const Vec3 &linearColor);
    // 将一个像素的全部样本设为同一颜色，方便二维实验；不改变深度。
    void PutPixel(int x, int y, Raster::Color encodedColor);
    Raster::Color GetPixel(int x, int y) const;
    // 逐像素在线性空间平均样本，调用 EncodeLinearColor，写入倒置 Y 的显示数组。
    // DrawMesh 结束调用一次，不要每写一个样本就重新处理整幅图。
    void Resolve();
    const Raster::Color *Data() const;
    // 保存 Data() 的 P6 RGB 文件；失败返回 false，不能默默认为保存成功。
    bool SavePPM(const char *utf8Path) const;

  private:
    // sampleIndex=(y*width+x)*samples+sample；显示 index=(height-1-y)*width+x。
    // 先检查边界，防止负整数转 size_t 后变成巨大索引。
    std::size_t SampleIndex(int x, int y, int sample) const;
    int width_ = 0, height_ = 0, samples_ = 1;
    std::vector<Vec3> sampleColors_;
    std::vector<float> sampleDepths_;
    std::vector<Raster::Color> displayColors_;
};
