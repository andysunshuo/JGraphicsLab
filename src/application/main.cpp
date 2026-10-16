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
    SceneSettings scene;
    std::string output;
    bool headless = false;
};
std::string Utf8(const wchar_t *value)
{
    const auto bytes = std::filesystem::path(value).u8string();
    return {bytes.begin(), bytes.end()};
}
void ParseArguments(StartupOptions &result)
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
    // 先记录无窗口模式，参数解析失败时也不能弹出阻塞自动运行的消息框。
    for (int i = 1; i < count; ++i)
        if (std::wstring(args.data[i]) == L"--headless") result.headless = true;
    for (int i = 1; i < count; ++i)
    {
        const std::wstring key = args.data[i];
        if (key == L"--headless")
            result.headless = true;
        else if (key == L"--basic")
            result.scene.basic = true;
        else if (key == L"--model" || key == L"--output" || key == L"--material" ||
                 key == L"--texture" || key == L"--lighting" || key == L"--shading")
        {
            if (++i >= count)
                throw std::runtime_error("启动参数缺少值");
            if (key == L"--output")
                result.output = Utf8(args.data[i]);
            else if (key == L"--model") result.scene.modelPath = Utf8(args.data[i]);
            else if (key == L"--material") result.scene.material = Utf8(args.data[i]);
            else if (key == L"--texture") result.scene.texture = Utf8(args.data[i]);
            else if (key == L"--lighting")
            {
                const auto value = Utf8(args.data[i]);
                if (value == "lambert") result.scene.lighting = LightingModel::Lambert;
                else if (value == "blinn-phong" || value == "blinn") result.scene.lighting = LightingModel::BlinnPhong;
                else if (value == "unlit") result.scene.lighting = LightingModel::Unlit;
                else if (value == "normal") result.scene.lighting = LightingModel::NormalVisualization;
                else throw std::invalid_argument("光照模型支持 lambert、blinn-phong、unlit、normal");
            }
            else
            {
                const auto value = Utf8(args.data[i]);
                if (value == "flat") result.scene.shading = ShadingMode::Flat;
                else if (value == "gouraud") result.scene.shading = ShadingMode::Gouraud;
                else if (value == "phong") result.scene.shading = ShadingMode::Phong;
                else throw std::invalid_argument("着色位置支持 flat、gouraud、phong");
            }
        }
        else
            throw std::runtime_error("未知参数：支持 --model、--material、--texture、--lighting、--shading、--output、--headless、--basic");
    }
    // 带原配贴图的模型默认使用自身 MTL；未指定材质时才自动选择。
    bool materialSpecified = false;
    for (int i = 1; i < count; ++i)
        materialSpecified |= std::wstring(args.data[i]) == L"--material";
    if (!materialSpecified && result.scene.modelPath != kDefaultModelPath &&
        result.scene.modelPath != "cube" && result.scene.modelPath != "triangle" &&
        result.scene.modelPath != "bunny" && result.scene.modelPath != "icosahedron")
        result.scene.material = "auto";
}

SceneSettings currentScene;
const char *modelNames[] = {"cube", "triangle", "icosahedron", "bunny", "room"};
const char *materialNames[] = {"auto", "wood", "brick", "stone", "gold", "plain"};
HMENU CreateSceneMenu()
{
    const auto menu = CreateMenu(), models = CreatePopupMenu(), materials = CreatePopupMenu(), lighting = CreatePopupMenu();
    const wchar_t *modelLabels[] = {L"立方体", L"三角形", L"低多边形模型", L"斯坦福兔子", L"维京房间（原配贴图）"};
    const wchar_t *materialLabels[] = {L"OBJ 原材质", L"木材", L"砖墙", L"岩石", L"金色涂层", L"纯灰色"};
    for (int i = 0; i < 5; ++i) AppendMenuW(models, MF_STRING, 101 + i, modelLabels[i]);
    for (int i = 0; i < 6; ++i) AppendMenuW(materials, MF_STRING, 201 + i, materialLabels[i]);
    AppendMenuW(lighting, MF_STRING, 301, L"Lambert 漫反射（按 1）");
    AppendMenuW(lighting, MF_STRING, 302, L"布林冯高光（按 2）");
    AppendMenuW(menu, MF_POPUP, reinterpret_cast<UINT_PTR>(models), L"模型");
    AppendMenuW(menu, MF_POPUP, reinterpret_cast<UINT_PTR>(materials), L"材质");
    AppendMenuW(menu, MF_POPUP, reinterpret_cast<UINT_PTR>(lighting), L"光照模型");
    return menu;
}
void UpdateSceneUI(HWND hwnd)
{
    const auto lighting = currentScene.lighting == LightingModel::Lambert ? L"Lambert" :
                          currentScene.lighting == LightingModel::BlinnPhong ? L"布林冯" : L"辅助观察";
    const auto title = std::wstring(L"JGraphicsLab — ") + lighting + L" — " +
        std::filesystem::u8path(currentScene.modelPath).filename().wstring() + L" — " +
        std::filesystem::u8path(currentScene.material).wstring() + L" — 1000×1000";
    SetWindowTextW(hwnd, title.c_str());
    const auto menu = GetMenu(hwnd);
    CheckMenuRadioItem(GetSubMenu(menu, 2), 301, 302, currentScene.lighting == LightingModel::Lambert ? 301 :
        currentScene.lighting == LightingModel::BlinnPhong ? 302 : 0, MF_BYCOMMAND);
    for (int i = 0; i < 5; ++i)
        CheckMenuItem(GetSubMenu(menu, 0), 101 + i, MF_BYCOMMAND | (ModelPathFromName(currentScene.modelPath) == ModelPathFromName(modelNames[i]) ? MF_CHECKED : MF_UNCHECKED));
    for (int i = 0; i < 6; ++i)
        CheckMenuItem(GetSubMenu(menu, 1), 201 + i, MF_BYCOMMAND | (currentScene.material == materialNames[i] ? MF_CHECKED : MF_UNCHECKED));
}
void SelectScene(HWND hwnd, int command)
{
    auto candidate = currentScene;
    candidate.basic = false;
    if (command >= 101 && command <= 105)
    {
        candidate.modelPath = modelNames[command - 101];
        if (command == 105) candidate.material = "auto";
        candidate.texture.clear();
    }
    else if (command >= 201 && command <= 206)
    {
        candidate.material = materialNames[command - 201];
        candidate.texture.clear();
    }
    else if (command == 301) candidate.lighting = LightingModel::Lambert;
    else if (command == 302) candidate.lighting = LightingModel::BlinnPhong;
    else return;
    try
    {
        RenderScene(candidate);
        currentScene = std::move(candidate);
        UpdateSceneUI(hwnd);
        InvalidateRect(hwnd, nullptr, FALSE);
    }
    catch (const std::exception &error)
    {
        MessageBoxW(hwnd, std::filesystem::u8path(error.what()).c_str(), L"资源或渲染失败", MB_OK | MB_ICONERROR);
    }
}
} // namespace
LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        SelectScene(hwnd, LOWORD(wParam));
        return 0;
    case WM_KEYDOWN:
        if (wParam == '1' || wParam == '2') SelectScene(hwnd, wParam == '1' ? 301 : 302);
        return 0;
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
        ParseArguments(options);
        currentScene = options.scene;
        RenderScene(currentScene);
        if (!options.output.empty() && !SaveRenderedImage(options.output))
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
        AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, TRUE);
        const std::wstring title = L"JGraphicsLab — 经典光照与贴图";
        const auto hwnd =
            CreateWindowExW(0, className, title.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                            rect.right - rect.left, rect.bottom - rect.top, nullptr, CreateSceneMenu(), instance, nullptr);
        if (!hwnd)
            throw std::runtime_error("窗口创建失败");
        UpdateSceneUI(hwnd);
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
