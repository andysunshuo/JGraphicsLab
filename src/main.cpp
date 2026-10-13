
#include <cstdint>
#include <exception>
#include <windows.h>

#include "RasterWindow.h"

// 你的作业函数
void RenderScene();

// ============================================================
// Framebuffer
// ============================================================

namespace
{
Raster::Color framebuffer[Raster::WIDTH * Raster::HEIGHT] = {};
}

// ============================================================
// Raster API
// ============================================================

namespace Raster
{
Color MakeColor(int r, int g, int b)
{
    return (static_cast<Color>(r & 255) << 16) | (static_cast<Color>(g & 255) << 8) | static_cast<Color>(b & 255);
}

void Clear(Color color)
{
    for (int i = 0; i < WIDTH * HEIGHT; i++)
    {
        framebuffer[i] = color;
    }
}

void PutPixel(int x, int y, Color color)
{
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
    {
        return;
    }

    // Y 向上，原点在左下角
    int index = (HEIGHT - 1 - y) * WIDTH + x;

    framebuffer[index] = color;
}
} // namespace Raster

// ============================================================
// Windows Message Handler
// ============================================================

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_ERASEBKGND:
        return 1;

    case WM_PAINT: {
        PAINTSTRUCT ps = {};
        HDC hdc = BeginPaint(hwnd, &ps);

        RECT rect = {};
        GetClientRect(hwnd, &rect);

        BITMAPINFO bitmapInfo = {};

        bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

        bitmapInfo.bmiHeader.biWidth = Raster::WIDTH;

        bitmapInfo.bmiHeader.biHeight = -Raster::HEIGHT;

        bitmapInfo.bmiHeader.biPlanes = 1;
        bitmapInfo.bmiHeader.biBitCount = 32;
        bitmapInfo.bmiHeader.biCompression = BI_RGB;

        SetStretchBltMode(hdc, COLORONCOLOR);

        StretchDIBits(hdc, 0, 0, rect.right - rect.left, rect.bottom - rect.top, 0, 0, Raster::WIDTH, Raster::HEIGHT,
                      framebuffer, &bitmapInfo, DIB_RGB_COLORS, SRCCOPY);

        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hwnd, message, wParam, lParam);
}

// ============================================================
// Application Entry
// ============================================================

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCommand)
{
    const wchar_t *className = L"JGraphicsLabWindow";

    WNDCLASSW wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = instance;
    wc.lpszClassName = className;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);

    if (!RegisterClassW(&wc))
    {
        return 1;
    }

    RECT rect = {0, 0, 600, 600};

    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    HWND hwnd = CreateWindowExW(0, className, L"JGraphicsLab", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                                rect.right - rect.left, rect.bottom - rect.top, nullptr, nullptr, instance, nullptr);

    if (!hwnd)
    {
        return 1;
    }

    // ========================================================
    // 执行你的图形学作业
    // ========================================================

    try
    {
        RenderScene();
    }
    catch (const std::exception &)
    {
        MessageBoxW(hwnd, L"模型加载或绘制失败，请检查 assets/models/triangle.obj 和资源目录。",
                    L"JGraphicsLab 启动失败", MB_OK | MB_ICONERROR);
        DestroyWindow(hwnd);
        return 1;
    }

    // ========================================================

    ShowWindow(hwnd, showCommand);
    UpdateWindow(hwnd);

    MSG msg = {};

    while (GetMessageW(&msg, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return static_cast<int>(msg.wParam);
}
