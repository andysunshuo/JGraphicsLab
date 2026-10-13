#include "Core/Color.h"
namespace Raster
{
    int GetR(Raster::Color color)
    {
        return (color >> 16) & 0xff;
    }
    int GetG(Raster::Color color)
    {
        return (color >> 8) & 0xff;
    }
    int GetB(Raster::Color color)
    {
        return color & 0xff;
    }

    Color MakeColor(int r, int g, int b)
    {
        return
            (static_cast<Color>(r & 255) << 16) |
            (static_cast<Color>(g & 255) << 8) |
            static_cast<Color>(b & 255);
    }

}
