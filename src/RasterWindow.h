
#pragma once

#include <cstdint>

namespace Raster
{
    constexpr int WIDTH = 100;
    constexpr int HEIGHT = 100;

    using Color = std::uint32_t;

    Color MakeColor(int r, int g, int b);

    void Clear(Color color);

    void PutPixel(int x, int y, Color color);


    int GetR(Raster::Color color);
    int GetG(Raster::Color color);
    int GetB(Raster::Color color);

}
