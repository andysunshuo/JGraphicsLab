#include "RasterWindow.h"
#include "application/Renderer.h"
#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <windows.h>

// Shell API 依赖 Windows 类型，必须在 windows.h 之后包含。
#include <shellapi.h>

namespace
{
struct StartupOptions
{
    std::string model = kDefaultModelPath, output;
    bool headless = false;
};
std::string Utf8(const wchar_t *value)
{
    const auto bytes = std::filesystem::path(value).u8string();
    return {bytes.begin(), bytes.end()};
}
StartupOptions ParseArguments()
{
    int count = 0;
    struct Arguments
    {
        wchar_t **data;
        ~Arguments()
        {
            LocalFree(data);
        }
    } args{CommandLineToArgvW(GetCommandLineW(), &count)};
    if (!args.data)
        throw std::runtime_error("无法读取启动参数");
    StartupOptions result;
    for (int i = 1; i < count; ++i)
    {
        const std::wstring key = args.data[i];
        if (key == L"--headless")
            result.headless = true;
        else if (key == L"--model" || key == L"--output")
        {
            if (++i >= count)
                throw std::runtime_error("启动参数缺少值");
            if (key == L"--output")
                result.output = Utf8(args.data[i]);
            else
                result.model = Utf8(args.data[i]);
        }
        else
            throw std::runtime_error("未知参数，只支持 --model、--output、--headless");
    }
    if (result.model == "cube")
        result.model = "assets/models/cube.obj";
    if (result.model == "triangle")
        result.model = "assets/models/triangle.obj";
    if (result.model == "icosahedron")
        result.model = "assets/models/low_poly_icosahedron.obj";
    return result;
}
} // namespace
LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_ERASEBKGND:
        return 1;
    case WM_PAINT: {
        PAINTSTRUCT paint{};
        const auto dc = BeginPaint(hwnd, &paint);
        RECT rect{};
        GetClientRect(hwnd, &rect);
        BITMAPINFO info{};
        info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        info.bmiHeader.biWidth = Raster::WIDTH;
        info.bmiHeader.biHeight = -Raster::HEIGHT;
        info.bmiHeader.biPlanes = 1;
        info.bmiHeader.biBitCount = 32;
        info.bmiHeader.biCompression = BI_RGB;
        SetStretchBltMode(dc, COLORONCOLOR);
        StretchDIBits(dc, 0, 0, rect.right, rect.bottom, 0, 0, Raster::WIDTH, Raster::HEIGHT, Raster::Pixels(), &info,
                      DIB_RGB_COLORS, SRCCOPY);
        EndPaint(hwnd, &paint);
        return 0;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, message, wParam, lParam);
}
int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCommand)
{
    StartupOptions options;
    try
    {
        options = ParseArguments();
        RenderScene(options.model);
        if (!options.output.empty() && !Raster::SavePPM(options.output.c_str()))
            throw std::runtime_error("图像保存失败，请检查输出目录");
        if (options.headless)
            return 0;
        const wchar_t *className = L"JGraphicsLabWindow";
        WNDCLASSW wc{};
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = instance;
        wc.lpszClassName = className;
        wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        if (!RegisterClassW(&wc))
            throw std::runtime_error("窗口类注册失败");
        // 缓冲始终为 1000×1000；初始显示尺寸适配桌面，缩放窗口不改变光栅化分辨率。
        const int side = std::max(
            200, std::min({Raster::WIDTH, GetSystemMetrics(SM_CXSCREEN) - 80, GetSystemMetrics(SM_CYSCREEN) - 100}));
        RECT rect{0, 0, side, side};
        AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
        const std::wstring title =
            L"JGraphicsLab — OBJ 基础光栅化 1000×1000 — " + std::filesystem::u8path(options.model).filename().wstring();
        const auto hwnd =
            CreateWindowExW(0, className, title.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                            rect.right - rect.left, rect.bottom - rect.top, nullptr, nullptr, instance, nullptr);
        if (!hwnd)
            throw std::runtime_error("窗口创建失败");
        ShowWindow(hwnd, showCommand);
        UpdateWindow(hwnd);
        MSG message{};
        int status;
        while ((status = GetMessageW(&message, nullptr, 0, 0)) > 0)
        {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
        return status < 0 ? 1 : static_cast<int>(message.wParam);
    }
    catch (const std::exception &error)
    {
        std::fprintf(stderr, "启动失败：%s\n", error.what());
        if (!options.headless)
        {
            const auto message = std::filesystem::u8path(error.what()).wstring();
            MessageBoxW(nullptr, message.c_str(), L"JGraphicsLab 启动失败", MB_OK | MB_ICONERROR);
        }
        return 1;
    }
}
