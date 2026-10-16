#include "application/Renderer.h"
#include "assets/AssetPath.h"
#include "geometry/OBJLoader.h"
#include "renderer/Rasterizer.h"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <windows.h>

namespace
{
void Check(bool value, const char *message)
{
    if (!value) throw std::runtime_error(message);
    std::cout << "通过：" << message << '\n';
}
bool Near(Vec3 a, Vec3 b)
{
    return a.IsFinite() && std::abs(a.x - b.x) < 1e-4f && std::abs(a.y - b.y) < 1e-4f && std::abs(a.z - b.z) < 1e-4f;
}
template <class F> bool Throws(F f)
{
    try { f(); } catch (const std::exception &) { return true; }
    return false;
}
std::uint64_t Hash()
{
    std::uint64_t result = 14695981039346656037ull;
    for (int i = 0; i < Raster::WIDTH * Raster::HEIGHT; ++i)
        result = (result ^ Raster::Pixels()[i]) * 1099511628211ull;
    return result;
}
void Write(const std::filesystem::path &path, const std::string &text)
{
    std::ofstream file(path, std::ios::binary);
    if (!(file << text)) throw std::runtime_error("无法写入测试资产");
}
} // namespace
int main()
{
    SetConsoleOutputCP(CP_UTF8);
    try
    {
        Light light;
        light.type = LightType::Directional;
        light.direction = {0, 0, -1};
        const std::vector<Light> lights{light};
        Material material;
        material.baseColor = {.8f, .4f, .2f};
        material.ambient = {.1f, .1f, .1f};
        material.diffuse = {.5f, .5f, .5f};
        material.specular = {.25f, .25f, .25f};
        const auto terms = material.EvaluateTerms({}, {0, 0, 1}, {0, 0, 2}, {}, lights);
        Check(Near(terms.ambient, {.08f, .04f, .02f}) && Near(terms.diffuse, {.4f, .2f, .1f}) &&
              Near(terms.specular, {.25f, .25f, .25f}), "用户实现的环境、漫反射和半程向量高光符合手算");
        Check(Near(material.Evaluate({}, {0, 0, 1}, {0, 0, 2}, {}, lights), {.73f, .49f, .37f}), "布林冯三项合成");
        material.model = LightingModel::Lambert;
        Check(Near(material.Evaluate({}, {0, 0, 1}, {0, 0, 2}, {}, lights), {.48f, .24f, .12f}), "Lambert 不包含高光");
        material.model = LightingModel::BlinnPhong;
        Check(Near(material.Evaluate({}, {0, 0, -1}, {0, 0, 2}, {}, lights), terms.ambient), "背光不产生漫反射或高光");
        Check(Near(material.Evaluate({}, {0, 0, 1}, {0, 0, 2}, {}, {}), terms.ambient), "无光源仅保留一次环境项");
        Check(Near(material.Evaluate({}, {0, 0, 1}, {0, 0, -2}, {}, lights), {.48f, .24f, .12f}), "零半程向量不会生成 NaN");
        Light point;
        point.position = {0, 0, 2};
        point.constantAttenuation = 0;
        point.quadraticAttenuation = 1;
        Check(Near(point.DirectionFrom({}), {0, 0, 1}) && Near(point.RadianceAt({}), {.25f, .25f, .25f}), "点光方向与距离平方反比");
        material.shininess = -1;
        Check(Throws([&] { material.Evaluate({}, {0, 0, 1}, {0, 0, 2}, {}, lights); }), "非法高光指数明确报错");
        material.shininess = 32;

        auto texture = std::make_shared<Texture>();
        texture->Create(2, 2, {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {1, 1, 1}});
        Check(Near(texture->SampleNearest({.25f, .75f}, 0, TextureWrap::Clamp), {1, 0, 0}) &&
              Near(texture->SampleNearest({.25f, .25f}, 0, TextureWrap::Clamp), {0, 0, 1}), "UV 的 V 向上，贴图行向下");
        Check(Near(texture->SampleBilinear({.5f, .5f}), {.5f, .5f, .5f}), "双线性在线性 RGB 中平均");
        Check(Near(texture->SampleNearest({-.75f, .75f}), {1, 0, 0}), "负 UV 重复地址正确");
        texture->GenerateMipmaps();
        Check(texture->LevelCount() == 2 && Near(texture->SampleTrilinear({.5f, .5f}, 1), {.5f, .5f, .5f}), "Mipmap 与三线性可复用");
        material.albedoTexture = texture;
        Check(Near(material.BaseColorAt({.25f, .75f}), {.8f, 0, 0}), "贴图和基色逐通道相乘");
        material.albedoTexture.reset();

        Mesh largeUnitTriangle;
        largeUnitTriangle.vertices = {{{-10000, -10000, 0}, {}, {0, 0, 1}},
                                      {{10000, -10000, 0}, {}, {0, 0, 1}},
                                      {{0, 10000, 0}, {}, {0, 0, 1}}};
        largeUnitTriangle.indices = {0, 1, 2};
        Raster::Clear(0);
        Raster::ClearDepth();
        Check(DrawMeshShaded(largeUnitTriangle, Transform::Scale({.0001f, .0001f, .0001f}), Camera{}, material, lights)
                  .rasterizedTriangles == 1,
              "大单位模型自动小比例缩放后仍能正确求法线并绘制");

        const auto folder = std::filesystem::path(TEST_OUTPUT_DIR) / L"材质与贴图";
        std::filesystem::create_directories(folder / "materials");
        std::filesystem::create_directories(folder / "textures");
        Raster::Clear(Raster::MakeColor(128, 128, 128));
        Raster::PutPixel(100, 900, Raster::MakeColor(255, 0, 0));
        Raster::PutPixel(100, 100, Raster::MakeColor(0, 0, 255));
        const auto image = folder / "textures/color.png";
        Check(Raster::SavePNG(AssetPathUtf8(image).c_str()), "导出真实 PNG 文件");
        Texture colorImage, dataImage;
        std::string error;
        Check(colorImage.Load(AssetPathUtf8(image), TextureEncoding::SrgbColor, &error) &&
              dataImage.Load(AssetPathUtf8(image), TextureEncoding::LinearData, &error), "中文目录 WIC 读图");
        Check(Near(colorImage.SampleNearest({.1005f, .9005f}), {1, 0, 0}) &&
              Near(colorImage.SampleNearest({.1005f, .1005f}), {0, 0, 1}), "PNG 保存/读取没有重复翻转");
        Check(colorImage.SampleNearest({.5f, .5f}).x < .22f && dataImage.SampleNearest({.5f, .5f}).x > .5f,
              "颜色图解码 sRGB，数据图保持线性通道");
        Check(!colorImage.Load(AssetPathUtf8(folder / "missing.png"), TextureEncoding::SrgbColor, &error) &&
              colorImage.Empty() && !error.empty(), "缺失贴图清空结果并返回明确错误");

        const auto model = folder / "two.obj";
        const auto mtl = folder / "materials/two.mtl";
        Write(mtl, "newmtl red\nKa .1 .1 .1\nKd 1 0 0\nKs .2 .2 .2\nNs 32\nmap_Kd ../textures/color.png\n"
                   "newmtl green\nKa .1 .1 .1\nKd 0 1 0\nKs 0 0 0\nNs 1\nmap_Kd ../textures/color.png\n");
        Write(model, "mtllib materials/two.mtl\nv -.8 -.7 0\nv .8 -.7 0\nv 0 .8 0\nv 0 0 -.1\n"
                     "vt 0 0\nvt 1 0\nvt .5 1\nvn 0 0 1\nusemtl red\nf 1/1/1 2/2/1 3/3/1\n"
                     "usemtl green\nf -4/2/1 -2/3/1 -1/1/1\n");
        const auto asset = OBJLoader::LoadAsset(AssetPathUtf8(model));
        Check(asset.triangleMaterials.size() == 2 && asset.triangleMaterials[0] != asset.triangleMaterials[1],
              "usemtl 保留逐三角形材质，负索引仍可解析");
        Check(asset.materials[0].albedoTexture == asset.materials[1].albedoTexture && asset.textureCount == 1,
              "贴图相对 MTL 目录查找，重复引用共享同一纹理");
        Check(!asset.generatedUV && asset.mesh.vertices.size() > 4, "保留独立 UV 索引和原配 UV");
        Camera camera;
        Raster::Clear(0);
        Raster::ClearDepth();
        const auto stats = DrawModelShaded(asset, Mat4::Identity(), camera, lights);
        Check(stats.inputTriangles == 2 && stats.rasterizedTriangles == 2 &&
              std::count_if(Raster::Pixels(), Raster::Pixels() + 1000000, [](auto c) { return c != 0; }) > 10000,
              "OBJ/MTL/PNG 经真实裁剪、光栅化和深度测试输出像素");
        auto invalid = asset;
        invalid.triangleMaterials[0] = 10000;
        Check(Throws([&] { DrawModelShaded(invalid, Mat4::Identity(), camera, lights); }), "非法材质编号不会越界");

        // 只改变光照模型，磁盘副本被清空后仍应从模型/材质缓存渲染。
        Write(model, "v -.5 -.5 0\nv .5 -.5 0\nv 0 .5 0\nf 1 2 3\n");
        Write(mtl, "newmtl test\nKa .1 .1 .1\nKd .5 .5 .5\nKs .5 .5 .5\nNs 2\n");
        SceneSettings scene;
        scene.modelPath = AssetPathUtf8(model);
        scene.material = AssetPathUtf8(mtl);
        RenderScene(scene);
        const auto blinn = Hash();
        Write(model, "");
        Write(mtl, "");
        scene.lighting = LightingModel::Lambert;
        RenderScene(scene);
        Check(blinn != Hash(), "切换 Lambert/布林冯改变画面，同时不重新读取 OBJ/MTL");
        Write(model, "mtllib missing.mtl\nv 0 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 3\n");
        Check(Throws([&] { OBJLoader::LoadAsset(AssetPathUtf8(model)); }), "缺失 MTL 输出路径错误而不悄悄忽略");
        Check(std::abs((Transform::NormalMatrix(Transform::Scale({2, 1, 1})) * Vec3{1, 1, 0}).Dot({2, -1, 0})) < 1e-5f,
              "非均匀缩放后的法线仍垂直变换后的切线");
        std::cout << "经典着色与资源链路验收通过\n";
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "验收失败：" << e.what() << '\n';
        return 1;
    }
}
