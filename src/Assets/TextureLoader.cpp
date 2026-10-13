#include "Assets/TextureLoader.h"
#include "Assets/ResourcePaths.h"
#include <windows.h>
#include <wincodec.h>
#include <wrl/client.h>
#include <stdexcept>
namespace Assets
{
bool LoadTexture(const std::filesystem::path &path, Texture &texture, std::string &error)
{
    texture = {};
    error.clear();
    const HRESULT com = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    const bool releaseCom = SUCCEEDED(com);
    try
    {
        if (FAILED(com) && com != RPC_E_CHANGED_MODE)
            throw std::runtime_error("无法初始化图像解码器");
        if (std::filesystem::file_size(path) > 64 * 1024 * 1024)
            throw std::runtime_error("纹理文件超过 64 MiB 上限");
        using Microsoft::WRL::ComPtr;
        ComPtr<IWICImagingFactory> factory;
        ComPtr<IWICBitmapDecoder> decoder;
        ComPtr<IWICBitmapFrameDecode> frame;
        ComPtr<IWICFormatConverter> converter;
        const auto check = [](HRESULT hr) {
            if (FAILED(hr))
                throw std::runtime_error("Windows 图像解码失败");
        };
        check(
            CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory)));
        check(factory->CreateDecoderFromFilename(path.c_str(), nullptr, GENERIC_READ,
                                                 WICDecodeMetadataCacheOnDemand, &decoder));
        check(decoder->GetFrame(0, &frame));
        UINT width = 0, height = 0;
        check(frame->GetSize(&width, &height));
        if (!width || !height || width > 8192 || height > 8192 ||
            static_cast<std::uint64_t>(width) * height > 16 * 1024 * 1024)
            throw std::runtime_error("纹理尺寸超过 8192 或 1600 万像素上限");
        check(factory->CreateFormatConverter(&converter));
        check(converter->Initialize(frame.Get(), GUID_WICPixelFormat32bppBGR, WICBitmapDitherTypeNone,
                                    nullptr, 0, WICBitmapPaletteTypeCustom));
        texture.pixels.resize(static_cast<std::size_t>(width) * height);
        check(converter->CopyPixels(nullptr, width * 4, static_cast<UINT>(texture.pixels.size() * 4),
                                    reinterpret_cast<BYTE *>(texture.pixels.data())));
        texture.width = width;
        texture.height = height;
    }
    catch (const std::exception &e)
    {
        texture = {};
        error = "加载纹理失败：" + PathUtf8(path) + "；" + e.what();
    }
    if (releaseCom)
        CoUninitialize();
    return error.empty();
}
} // namespace Assets
