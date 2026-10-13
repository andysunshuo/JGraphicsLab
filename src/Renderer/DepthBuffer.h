#pragma once
#include "Renderer/Framebuffer.h"
#include <vector>
// 共用严格小于的深度更新规则；新多采样缓冲和原单采样缓冲使用同一实现。
bool DepthTestValue(float &savedDepth,float incomingDepth);

extern float depthBuffer[Raster::HEIGHT][Raster::WIDTH];

void ClearDepthBuffer();

bool DepthTest(
    int x,
    int y,
    float depth);
