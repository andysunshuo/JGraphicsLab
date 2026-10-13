#include "Renderer/Framebuffer.h"
#include <fstream>
namespace { Raster::Color framebuffer[Raster::WIDTH * Raster::HEIGHT] = {}; }
namespace Raster
{
    void Clear(Color color)
    {
        for (int i = 0; i < WIDTH * HEIGHT; i++)
        {
            framebuffer[i] = color;
        }
    }

    void PutPixel(int x, int y, Color color)
    {
        if (x < 0 || x >= WIDTH ||
            y < 0 || y >= HEIGHT)
        {
            return;
        }

        // Y 向上，原点在左下角
        int index = (HEIGHT - 1 - y) * WIDTH + x;

        framebuffer[index] = color;
    }
}


namespace Raster {
const Color* Pixels() { return framebuffer; }
Color ReadPixel(int x, int y) {
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return 0;
    return framebuffer[(HEIGHT - 1 - y) * WIDTH + x];
}
bool SavePPM(const std::filesystem::path& path) {
    std::ofstream file(path, std::ios::binary);
    if (!file) return false;
    file << "P6\n" << WIDTH << ' ' << HEIGHT << "\n255\n";
    for (Color c : framebuffer) {
        const char rgb[3] = {static_cast<char>(GetR(c)), static_cast<char>(GetG(c)), static_cast<char>(GetB(c))};
        file.write(rgb, 3);
    }
    return static_cast<bool>(file);
}
}
