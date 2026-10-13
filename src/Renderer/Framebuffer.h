
#pragma once

#include "Core/Color.h"
#include <filesystem>

namespace Raster
{
    constexpr int WIDTH = 100;
    constexpr int HEIGHT = 100;



    Color MakeColor(int r, int g, int b);

    void Clear(Color color);

    void PutPixel(int x, int y, Color color);


    int GetR(Raster::Color color);
    int GetG(Raster::Color color);
    int GetB(Raster::Color color);

}

namespace Raster {
const Color* Pixels();
Color ReadPixel(int x, int y);
bool SavePPM(const std::filesystem::path& path);
}
