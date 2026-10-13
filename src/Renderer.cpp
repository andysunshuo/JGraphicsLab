
#include "RasterWindow.h"
#include <stdexcept>

#include "geometry/OBJLoader.h"
#include "rasterizer/Transform.h"
#include "rasterizer/TriangleRasterizer.h"
// ============================================================
// JGraphicsLab
//
// HW01 - Pixel Rendering
//
// Framebuffer: 100 × 100
//
// X: 0 ~ 99
// Y: 0 ~ 99
//
// Origin: Bottom-Left
//
// ============================================================

void RenderScene()
{
    Raster::Clear(Raster::MakeColor(0, 0, 0));

    Mesh mesh = OBJLoader::Load("assets/models/triangle.obj");

    if (mesh.indices.empty())
        throw std::runtime_error("模型加载失败");

    for (std::size_t i = 0; i + 2 < mesh.indices.size(); i += 3)
    {

        Vertex v0 = mesh.vertices[mesh.indices[i]];

        Vertex v1 = mesh.vertices[mesh.indices[i + 1]];

        Vertex v2 = mesh.vertices[mesh.indices[i + 2]];

        ScreenVertex s0 = ToScreen(v0.position, Raster::WIDTH, Raster::HEIGHT);

        ScreenVertex s1 = ToScreen(v1.position, Raster::WIDTH, Raster::HEIGHT);

        ScreenVertex s2 = ToScreen(v2.position, Raster::WIDTH, Raster::HEIGHT);

        RasterizeTriangle(s0, s1, s2);
    }
}