#pragma once
#include <stdexcept>
#include <windows.h>
#include <wincodec.h>
#include <wrl/client.h>

namespace ImageIO
{
// 只平衡本次成功的 COM 初始化；已在 STA 中的窗口线程也允许使用 WIC。
class Apartment
{
  public:
    Apartment() : result_(CoInitializeEx(nullptr, COINIT_MULTITHREADED))
    {
        if (FAILED(result_) && result_ != RPC_E_CHANGED_MODE)
            throw std::runtime_error("图像编解码：COM 初始化失败");
    }
    ~Apartment()
    {
        if (SUCCEEDED(result_))
            CoUninitialize();
    }
    Apartment(const Apartment &) = delete;
    Apartment &operator=(const Apartment &) = delete;
  private:
    HRESULT result_;
};
inline void Check(HRESULT result, const char *message)
{
    if (FAILED(result))
        throw std::runtime_error(message);
}
} // namespace ImageIO
