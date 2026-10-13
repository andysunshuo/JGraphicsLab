#include "Application/Application.h"
#include <windows.h>
#include <cstdio>
#include <exception>
// 控制台程序 JRenderLabCLI.exe 的真正入口，适合 --hw 17 等数学检查。
// 解析参数后执行与窗口版相同的初始化/作业调用；不创建图形窗口，结束时返回退出码。
int wmain(int argc, wchar_t **argv)
{
    SetConsoleOutputCP(CP_UTF8);
    try
    {
        std::vector<std::string> arguments;
        for (int i = 1; i < argc; ++i)
        {
            const int size = WideCharToMultiByte(CP_UTF8, 0, argv[i], -1, nullptr, 0, nullptr, nullptr);
            std::string value(size, '\0');
            WideCharToMultiByte(CP_UTF8, 0, argv[i], -1, value.data(), size, nullptr, nullptr);
            value.pop_back();
            arguments.push_back(value);
        }
        return InitializeApplication(ParseArguments(arguments)) ? 0 : 1;
    }
    catch (const std::exception &e)
    {
        std::fprintf(stderr, "启动失败：%s\n", e.what());
        return 1;
    }
}
