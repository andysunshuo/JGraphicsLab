#include <windows.h>
#include <shellapi.h>
#include <cstdio>
#include <stdexcept>
#include "Application/Application.h"
#include "Homework/AssetHomework.h"
#include "Renderer/Framebuffer.h"
namespace
{
ApplicationOptions windowOptions;
ULONGLONG startTicks = 0;
} // namespace
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
        const auto surface = GetApplicationDisplaySurface();

        bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

        bitmapInfo.bmiHeader.biWidth = surface.width;

        bitmapInfo.bmiHeader.biHeight = -surface.height;

        bitmapInfo.bmiHeader.biPlanes = 1;
        bitmapInfo.bmiHeader.biBitCount = 32;
        bitmapInfo.bmiHeader.biCompression = BI_RGB;

        SetStretchBltMode(hdc, COLORONCOLOR);

        StretchDIBits(hdc, 0, 0, rect.right - rect.left, rect.bottom - rect.top, 0, 0, surface.width,
                      surface.height, surface.pixels, &bitmapInfo, DIB_RGB_COLORS, SRCCOPY);

        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_KEYDOWN:
        // 仅为新完成的学习章节增加左右切换，原作业与资源展示按键行为不变。
        if (windowOptions.demo == ModelDemo::None && windowOptions.homework >= 32 &&
            windowOptions.homework <= 50 && (wParam == VK_LEFT || wParam == VK_RIGHT))
        {
            const int step = wParam == VK_RIGHT ? 1 : -1;
            windowOptions.homework = 32 + (windowOptions.homework - 32 + step + 19) % 19;
            if (InitializeApplication(windowOptions))
            {
                SetWindowTextW(hwnd, ApplicationWindowTitle(windowOptions).c_str());
                InvalidateRect(hwnd, nullptr, FALSE);
            }
            return 0;
        }
        break;
    case WM_TIMER:
        if (!RenderApplicationFrame(windowOptions,
                                    0.4f + static_cast<float>(GetTickCount64() - startTicks) * 0.0005f))
        {
            KillTimer(hwnd, 1);
        }
        InvalidateRect(hwnd, nullptr, FALSE);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hwnd, message, wParam, lParam);
}

// 图形程序 JGraphicsLab.exe 的真正入口；默认编号读取 Config.h，启动参数可覆盖。
// 顺序：解析参数 -> InitializeApplication（检查/绘制到帧缓冲）-> 建窗口 -> WM_PAINT 显示。
// 进入各 HW 的位置在 Homework/Runner.cpp；本入口不直接重写任何作业算法。
int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCommand)
{
    if (!AttachConsole(ATTACH_PARENT_PROCESS))
        AllocConsole();
    SetConsoleOutputCP(CP_UTF8);
    FILE *stream = nullptr;
    freopen_s(&stream, "CONOUT$", "w", stdout);
    freopen_s(&stream, "CONOUT$", "w", stderr);
    try
    {
        int argc = 0;
        wchar_t **argv = CommandLineToArgvW(GetCommandLineW(), &argc);
        if (!argv)
            throw std::runtime_error("无法解析启动参数");
        std::vector<std::string> arguments;
        for (int i = 1; i < argc; ++i)
        {
            const int size = WideCharToMultiByte(CP_UTF8, 0, argv[i], -1, nullptr, 0, nullptr, nullptr);
            std::string value(size, '\0');
            WideCharToMultiByte(CP_UTF8, 0, argv[i], -1, value.data(), size, nullptr, nullptr);
            value.pop_back();
            arguments.push_back(value);
        }
        LocalFree(argv);
        windowOptions = ParseArguments(arguments);
        if (!InitializeApplication(windowOptions))
            return 1;
        if (windowOptions.headless || windowOptions.allTests || windowOptions.list)
            return 0;
    }
    catch (const std::exception &e)
    {
        std::fprintf(stderr, "启动失败：%s\n", e.what());
        return 1;
    }
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

    HWND hwnd =
        CreateWindowExW(0, className, L"JGraphicsLab", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                        rect.right - rect.left, rect.bottom - rect.top, nullptr, nullptr, instance, nullptr);

    if (!hwnd)
    {
        return 1;
    }
    SetWindowTextW(hwnd, ApplicationWindowTitle(windowOptions).c_str());

    // ========================================================
    // 执行你的图形学作业
    // ========================================================

    startTicks = GetTickCount64();
    if (windowOptions.animate &&
        (windowOptions.demo != ModelDemo::None || IsAnimatedAssetHomework(windowOptions.homework)))
        SetTimer(hwnd, 1, 16, nullptr);

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
