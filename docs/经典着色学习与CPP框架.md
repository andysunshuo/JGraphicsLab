# 经典着色学习与 .cpp 实现框架

后续状态：你完成 Light/Material 后，剩余管线已继续接通。当前运行与材质贴图用法见 [OBJ 材质贴图与光照使用](OBJ材质贴图与光照使用.md)。下面保留当时的待填框架用于学习，不应再复制它覆盖已经完成的实现。

本次只修改项目自有 `.h`，并增加本文档；没有修改 `.cpp`、CMake、模型或光追模块。函数体由你实现。文中的代码块是供你以后填写的框架，没有自动写入源文件。

当前实际运行仍是 `DrawMeshBasic` 的固定面色。新增声明可以通过编译，不等于已经实现光照；在你完成定义并接线前，不要把窗口入口切到新函数。本文是本轮着色练习的说明，旧文档中的纹理、动态帧缓冲和光追属于其他阶段。

## 1. 本轮做到什么程度

目标是使用已有 OBJ、MVP、齐次裁剪、透视正确插值和深度测试，在现有 1000×1000 窗口显示经典 Blinn-Phong 着色。

完成顺序：

1. Unlit：统一材质颜色，去掉调试用的六色 palette。
2. NormalVisualization：将世界空间法线映射为颜色，检查法线变换。
3. Lambert：环境光与漫反射。
4. BlinnPhong：加入半程向量高光。
5. Flat、Gouraud、Phong：使用相同光照公式，比较不同计算位置。

本轮不需要纹理加载、法线贴图、环境贴图、阴影、PBR、反射、折射或光追。沿用单样本固定缓冲，不需要先实现动态 `FrameBuffer` 或 MSAA。

| 文件 | 你要实现或理解的内容 |
|---|---|
| `src/scene/Light.h` → `Light.cpp` | 表面指向光源的方向、光源颜色、强度、可选距离衰减 |
| `src/material/Material.h` → `Material.cpp` | `LightingTerms` 三项结果、纯色材质、Lambert、Blinn-Phong、模式分派 |
| `src/renderer/Rasterizer.h` → `Rasterizer.cpp` | 新增 `DrawMeshShaded`；变换法线，在裁剪前准备 Flat/Gouraud 颜色 |
| `src/renderer/TriangleRasterizer.h` → `TriangleRasterizer.cpp` | 新增 `RasterizeTriangleShaded`；复用覆盖和深度算法，逐片元求色 |
| `src/geometry/Vertex.h` | 区分模型/世界法线，区分基础面色与已经照明的线性颜色 |
| `src/application/Renderer.h` | 接入顺序说明；现有 `RenderScene` 签名和默认模型保持原样 |

头文件已经写明公式、参数空间、函数调用关系和错误处理。不要再新建第二套 `Vec3`、矩阵、光照函数或 OBJ 解析器。

## 2. 先理解公式和颜色

现有面色来自 `Rasterizer.cpp` 的 `palette[(face / 2) % 6]`，按三角形编号选择颜色，没有材质或光照含义。着色入口不使用 palette，也不额外乘 `Vertex::color`，从 `Material::baseColor` 开始计算。

全部光照在线性 RGB 中计算。例：`baseColor={0.8f,0.4f,0.2f}` 表示线性反射颜色；不能把 200、100、50 直接填进去。若想使用显示 RGB，先用现有 `Raster::DecodeSrgbColor(Raster::MakeColor(...))` 转换。最后写像素时只调用一次 `Raster::EncodeLinearColor`。

记 `⊙` 为逐通道相乘，对应 `Vec3::Hadamard`，不是点积。对于世界空间表面位置 P：

```text
N = normalize(世界空间法线)
V = normalize(eye - P)                  // 本轮使用透视相机
L = 光源.DirectionFrom(P)               // 从表面指向光源
I = 光源.RadianceAt(P)                  // 已含颜色、强度和距离衰减
B = BaseColorAt(uv)

A = ambient ⊙ B                         // 只加一次
D = Σ(diffuse ⊙ B ⊙ I) × max(0, N·L)
H = normalize(L + V)                    // 每个光源有各自的 H
S = Σ(specular ⊙ I) × pow(max(0, N·H), shininess)

Lambert     = A + D
Blinn-Phong = A + D + S
```

高光有前提：`N·L>0`，且 N、L、V、H 有效。背光不产生高光。L 为零时跳过该光；V 或 H 为零时只跳过高光，漫反射仍可计算。N 为零时只留下环境项，不产生 NaN。

这里 `ambient` 已合并环境光强与环境反射系数，环境项不依赖光源列表长度；不是从 env 文件加载的环境光。高光不乘基色，橙色物体在白光下可以有白色高光。本轮是经典经验模型，不添加能量归一化、Fresnel 或物理材质参数。

`shininess` 越大，高光通常越小、边缘越集中；亮度由 `specular` 与光强共同控制。先分别观察 A、D、S，再把它们相加。合成后允许超过 1，由显示编码统一钳制；不要分别钳制中间项。

## 3. 第一步：填写 Light.cpp

先写两个函数，验证通过后再写材质。下面是整个空文件的框架；函数体中抛错是“尚未实现”的标记，不是最终算法。完成时替换函数体，不要仅在抛错之前加一些代码后保留无条件抛错。

```cpp
#include "scene/Light.h"
#include <cmath>
#include <stdexcept>

Vec3 Light::DirectionFrom(const Vec3 &surfacePosition) const
{
    // 待实现 1：检查参与运算的坐标有限，以及 type 是有效枚举。
    // 待实现 2：Point 使用 position-surfacePosition，Directional 使用 -direction。
    // 待实现 3：调用 Vec3::Normalized；零长度返回零。
    // 观察 delta 和返回的 L：L 应当从表面指向光源，而不是光线传播方向。
    throw std::logic_error("待实现：光源方向");
}

Vec3 Light::RadianceAt(const Vec3 &surfacePosition) const
{
    // 待实现 1：检查颜色/强度/衰减参数及必要坐标，非法参数明确报错。
    // 待实现 2：方向光不衰减；点光计算距离 d 和 kc+kl*d+kq*d*d。
    // 待实现 3：检查分母为有限正数，再计算 color*intensity/denominator。
    // 观察 d、attenuation、结果 I；这里不计算材质颜色，也不计算 N.Dot(L)。
    throw std::logic_error("待实现：光源强度与衰减");
}
```

手算检查：

| 设置 | 预期 |
|---|---|
| 点光位置 `(0,0,2)`，表面原点 | L 为 `(0,0,1)` |
| 方向光 `direction=(0,0,-1)` | L 为 `(0,0,1)`，与表面位置无关 |
| 白光、强度 1、默认衰减 `(1,0,0)` | I 为 `(1,1,1)` |
| 上述点光，改成 `(kc,kl,kq)=(0,0,1)` | 距离 2 时 I 为 `(0.25,0.25,0.25)` |
| 点光位置等于表面位置 | L 为零；若分母也为零，衰减设置必须明确报错 |
| 负强度、NaN、点光全零衰减系数 | 输出中文错误，不能继续生成非有限颜色 |

默认常量衰减最适合先检查方向。等明暗正确后，再实验距离平方反比。

## 4. 第二步：填写 Material.cpp

`EvaluateTerms` 是光照公式的唯一实现；`Evaluate` 只是模式分派和结果合成。

```cpp
#include "material/Material.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

Vec3 Material::BaseColorAt(const Vec2 &uv, float lod) const
{
    // 待实现 1：检查 baseColor 为有限 [0,1] 分量。
    // 待实现 2：本轮 albedoTexture 必须为空；非空明确报告“纹理采样尚未实现”。
    // 待实现 3：直接返回 baseColor；uv/lod 暂不使用，不调用 Texture 的空声明。
    throw std::logic_error("待实现：纯色材质");
}

LightingTerms Material::EvaluateTerms(const Vec3 &worldPosition, const Vec3 &worldNormal,
                                      const Vec3 &eye, const Vec2 &uv,
                                      const std::vector<Light> &lights, float lod) const
{
    // 待实现 1：只接受 Lambert/BlinnPhong；检查位置/法线/eye/UV 和材质参数。
    // 待实现 2：base=BaseColorAt，构造局部 LightingTerms，计算一次 ambient。
    // 待实现 3：归一化 N、V；零 N 保留环境项后返回，不进入直接光循环。
    // 待实现 4：对 lights 使用 const 引用循环，不复制 vector、不逐片元分配。
    //          依次获取 L、I、ndotl，累加 diffuse；背光/零 L 跳过。
    // 待实现 5：BlinnPhong 再处理 V、L+V、H、N.Dot(H)、pow，累加 specular。
    // 待实现 6：检查三个结果有限，返回，不编码颜色、不写帧缓冲。
    // 推荐断点：base / N / V / L / I / ndotl / H / 高光幂值 / 三个结果。
    throw std::logic_error("待实现：环境光、漫反射和布林冯高光");
}

Vec3 Material::Evaluate(const Vec3 &worldPosition, const Vec3 &worldNormal,
                        const Vec3 &eye, const Vec2 &uv,
                        const std::vector<Light> &lights, float lod) const
{
    // 待实现：按 model 分派，不复制 EvaluateTerms 的数学公式。
    // Unlit：返回 BaseColorAt。
    // NormalVisualization：法线归一化后映射到 [0,1]；零法线显示中灰。
    // Lambert/BlinnPhong：调用 EvaluateTerms，返回 ambient+diffuse+specular。
    // 未知枚举抛 invalid_argument，不能默认返回黑色掩盖错误。
    throw std::logic_error("待实现：材质着色模式分派");
}
```

定义中的 `lod` 不再写默认值，默认参数只出现在 `.h`。同样不要在定义中再写结构体字段默认值。

建议先只实现 `BaseColorAt` 与 `Evaluate` 的 Unlit、法线分支，再逐步加入 Lambert 和高光。未完成的分支保留明确的“待实现”错误，不伪造成功。

### 可以精确手算的一组材质

```text
P=(0,0,0)，N=(0,0,1)，eye=(0,0,2)
白色方向光 direction=(0,0,-1)，intensity=1
baseColor=(0.8,0.4,0.2)
ambient=(0.1,0.1,0.1)
diffuse=(0.5,0.5,0.5)
specular=(0.25,0.25,0.25)
shininess=32
```

此时 N、L、V、H 都是 `(0,0,1)`：

| 分量/模式 | 预期线性 RGB |
|---|---|
| A | `(0.08,0.04,0.02)` |
| D | `(0.40,0.20,0.10)` |
| S | `(0.25,0.25,0.25)` |
| Lambert | `(0.48,0.24,0.12)` |
| BlinnPhong | `(0.73,0.49,0.37)` |
| 无光源 | `(0.08,0.04,0.02)` |
| 两个相同方向光 | `(1.38,0.94,0.72)`，环境项仍然只加一次 |

这是写入显示缓冲之前的数值，用约 `1e-5` 容差检查，不与 8 位 sRGB 字节直接比较。多光源例的红色超过 1 是允许的。

还要检查：背光只有 A；`L+V=0` 时只有 A+D；`eye=P` 时跳过高光但保留漫反射；零法线时只有 A；法线可视化的 +Z 为 `(0.5,0.5,1)`。强度设为零应只剩环境项。

## 5. 第三步：填写 DrawMeshShaded

将下面的新定义追加到 `renderer/Rasterizer.cpp`，不要覆盖原有 `DrawMeshBasic`。补上需要的标准头文件。不要从文档复制整份文件替换现有文件。

```cpp
RenderStats DrawMeshShaded(const Mesh &mesh, const Mat4 &model, const Camera &camera,
                           const Material &material, const std::vector<Light> &lights,
                           const TriangleRenderOptions &options)
{
    // 待实现 1：开始计时；检查索引数、有限属性、仿射 model、透视相机和配置。
    // 不先调用 Mesh::IsValid、ComputeBounds 等尚无定义的函数。
    // 复用基础入口检查；model 最后一行应为 (0,0,0,1)。

    // 待实现 2：每次 Draw 只计算一次 mvp 和 normalMatrix。
    // mvp=camera.ViewProjectionMatrix()*model。
    // normalMatrix=Transform::NormalMatrix(model)，不能重复实现求逆。

    // 待实现 3：按三角形索引读取原始顶点，检查索引，计算世界位置和几何面法线。
    // 退化面跳过；已有顶点法线用 normalMatrix 变换并归一化。
    // 任一顶点法线缺失/为零，则本面三个顶点统一使用几何面法线。
    // 生成 ClipVertex：position=mvp*位置；worldPosition、normal 保持世界空间。

    // 待实现 4：Flat 在世界面心调用一次 Evaluate，三个顶点 color 写相同结果。
    // Gouraud 在三个原始顶点各调用一次 Evaluate。
    // Phong 暂不计算顶点颜色，保留世界位置/法线/UV，交给片元阶段。

    // 待实现 5：调用现有 ClipTriangleToFrustum，不在裁剪前除 w。
    // 用已有 InterpolateClipVertex 同步插值属性和预计算颜色。
    // 对裁剪多边形作三角扇，调用 Transform::ToScreen，再提交 RasterizeTriangleShaded。
    // Flat 的面心/面法线取原始三角形，不对裁剪得到的每个小三角形重新计算光照。

    // 待实现 6：填写真实 input/rasterized/rejected 和耗时，返回 RenderStats。
    // 这里不清屏、不加载 OBJ、不保存文件、不调用动态 FrameBuffer::Resolve。
    throw std::logic_error("待实现：带经典着色的模型管线");
}
```

世界空间属性准备顺序：

```text
Pworld = (M * Vec4::Position(vertex.position)).xyz
Nworld = normalize(Transform::NormalMatrix(M) * vertex.normal)
Pclip  = P * V * M * Vec4::Position(vertex.position)
```

为什么法线不直接乘 M？非均匀缩放后，普通方向变换不能保证法线继续垂直表面。法线矩阵取模型矩阵左上角 3×3 的逆转置；平移、View 和 Projection 都不参与世界法线变换。该算法已经有公共实现，不要重写。

手算例：原切线 `(1,-1,0)`，原法线为 `normalize(1,1,0)`。模型缩放 `(2,1,1)` 后，切线变成 `(2,-1,0)`；正确法线方向为 `(0.5,1,0)`。两者点积为零，而直接乘缩放矩阵得到的 `(2,1,0)` 不再垂直。

本轮先用透视相机，沿用 `V=normalize(eye-P)`。如果传入正交相机，应明确报告本阶段正交着色尚未支持，不能悄悄用错误的观察方向；基础 `DrawMeshBasic` 不受影响。将来可给材质统一提供观察方向，以支持正交相机的平行视线。

### 三种着色不要混淆

| 模式 | 使用的法线 | 何时调用 Material::Evaluate | 像素阶段 |
|---|---|---|---|
| Flat | 原始世界空间几何面法线 | 每原始三角形一次 | 显示已算好的同一颜色 |
| Gouraud | 变换后的顶点法线 | 每原始三角形的三个顶点各一次 | 透视正确插值光照颜色 |
| Phong | 插值后的顶点法线，再归一化 | 每个通过深度测试的采样点 | 计算光照颜色 |

Gouraud 此处按三角形计算，重复顶点可以先不做结果缓存。Phong 是计算位置，BlinnPhong 是光照公式，两个枚举各管一件事。先实现 Phong，再接 Flat/Gouraud 做对照也可以。

## 6. 第四步：填写 RasterizeTriangleShaded

下面的新定义追加到 `renderer/TriangleRasterizer.cpp`。该文件目前只前置声明了材质/相机；实现里调用成员函数，需要包含完整头文件：

```cpp
#include "material/Material.h"
#include "scene/Camera.h"
```

随后填写：

```cpp
void RasterizeTriangleShaded(const ScreenVertex &a, const ScreenVertex &b,
                             const ScreenVertex &c, const Material &material,
                             const Camera &camera, const std::vector<Light> &lights,
                             const TriangleRenderOptions &options)
{
    // 待实现 1：复用已有有效性、有向面积、屏幕包围盒与 Top-left 覆盖逻辑。
    // 参数为 const 引用；若要交换绕序，使用局部顶点副本，不能只交换位置而漏掉属性。

    // 待实现 2：像素中心 (x+0.5,y+0.5) 求覆盖和重心坐标。
    // ndcZ 用屏幕重心线性插值，depth=clamp(ndcZ*0.5+0.5,0,1)。
    // 沿用原有有限/范围检查后再 clamp，不能把任意越界几何靠 clamp 拉回屏幕。

    // 待实现 3：options.depthTest 为 true 时调用现有 Raster::DepthTest。
    // 不通过立即继续；false 时既不比较也不更新深度。

    // 待实现 4：调用现有 InterpolateAttributes，传入 perspectiveCorrect。
    // Flat/Gouraud 取插值后的 color；Phong 调用 material.Evaluate，传 camera.eye。
    // 不要把插值法线的长度直接用于点积，归一化由材质入口保证。

    // 待实现 5：EncodeLinearColor 后 PutPixel，不再翻转 Y、不提前编码顶点颜色。
    // 没有纹理读取、没有每像素容器分配、没有射线求交。
    throw std::logic_error("待实现：逐片元经典着色");
}
```

### 如何复用覆盖循环

不要长期复制两份 `RasterizeTriangle` 的双重循环。你可以在同一 `.cpp` 的匿名命名空间抽出一个内部模板函数，负责覆盖、深度和属性插值，用可调用对象负责“属性变颜色”：

```cpp
namespace
{
// 这只是内部函数签名草图；实现时在同一 cpp 中写出模板定义。
// shade(interpolated) 返回线性 Vec3；不要使用每像素可能分配内存的动态封装。
template <class Shade>
void RasterizeCoveredTriangle(ScreenVertex a, ScreenVertex b, ScreenVertex c,
                               bool depthTest, bool perspectiveCorrect,
                               Shade &&shade);
}
```

由你将现有循环移入这个内部函数，三个入口的关系应当是：

```text
已有 RasterizeTriangle -> 内部覆盖循环 -> 返回插值 color
新 RasterizeTriangleShaded（Flat/Gouraud）-> 同一覆盖循环 -> 返回插值 color
新 RasterizeTriangleShaded（Phong）-> 同一覆盖循环 -> Material::Evaluate
```

基础入口保持默认深度测试和透视正确颜色插值。共享边规则、Y 方向、深度写入和已有数学函数不改变。提取后先跑现有测试，再接光照，方便定位是覆盖算法还是着色出了问题。

透视属性公式仍是 `Σ(λ*q/w)/Σ(λ/w)`；世界位置、法线、UV 和 Gouraud 颜色使用它。屏幕 x/y 与 NDC z 不再额外除一次 w。

## 7. 最后接到窗口：你以后在 Renderer.cpp 修改的位置

完成前面的定义后，保留 `RenderScene(modelPath)` 中已有的清屏、深度清理、模型缓存、相机、AutoFit 和模型矩阵。只在原来调用 `DrawMeshBasic` 的地方切换调用。以下代码是该位置的示例，不是新增一个 RenderScene 定义：

```cpp
Material material;
material.model = LightingModel::BlinnPhong;
material.baseColor = {0.8f, 0.4f, 0.2f};
material.ambient = {0.1f, 0.1f, 0.1f};
material.diffuse = {0.5f, 0.5f, 0.5f};
material.specular = {0.25f, 0.25f, 0.25f};
material.shininess = 32;

Light light;
light.type = LightType::Directional;
light.direction = {-1, -1, -1}; // 光传播方向；表面指向光源的方向是它的相反方向。
const std::vector<Light> lights{light};

TriangleRenderOptions options;
options.shading = ShadingMode::Phong;
const auto stats = DrawMeshShaded(cachedMesh, model, camera, material, lights, options);
```

现有头文件已间接提供这些类型，仍建议实现文件按使用类型直接包含 `material/Material.h`、`scene/Light.h`、`<vector>`，减少对间接包含的依赖。

切换观察项目：

```cpp
material.model = LightingModel::Unlit;
material.model = LightingModel::NormalVisualization;
material.model = LightingModel::Lambert;
material.model = LightingModel::BlinnPhong;

options.shading = ShadingMode::Flat;
options.shading = ShadingMode::Gouraud;
options.shading = ShadingMode::Phong;
```

上面是不同取值示例，实际每次选择其中一个，不要全部粘贴后误以为同时显示。当前没有新增键盘、UI 或命令行着色选择器；仍通过代码设置并重新运行观察。

当前窗口只有调用 `RenderScene` 时才重新渲染，不是持续动画。你修改光源、相机或材质并重编译运行，才会看到更新；本轮无需新增动画循环。

## 8. 将来接入时的 CMake 注意事项

本次没有修改 CMake。现在 `Light.cpp`、`Material.cpp` 是空文件，列在可执行程序 `JGraphicsLab` 的源列表中，而 `Rasterizer.cpp` 属于共享静态库 `JRenderBasic`。当后者开始调用材质/光源后，测试也需要链接到这两个实现。

你实现并接入时：

1. 在 `add_library(JRenderBasic STATIC ...)` 的源码列表中增加 `src/scene/Light.cpp` 与 `src/material/Material.cpp`。
2. 从现有 `target_sources(JGraphicsLab PRIVATE ...)` 中移除这两个文件，避免把实现分散到两个目标。
3. 保留已有 `Texture.cpp` 等空模块的位置，不调用其未实现函数；不增加光追源码调用。

迁移后的可执行程序额外源列表应只保留原来的其余三项：

```cmake
target_sources(JGraphicsLab PRIVATE src/material/Texture.cpp
    src/raytracing/Ray.cpp src/raytracing/BVH.cpp)
```

Ray/BVH 只是本来就存在的空占位，本文不要求实现或调用它们。不要将同一个函数同时定义在文档新文件和原 `.cpp`，否则会出现重复符号。

如果出现 LNK2019：先看未解析的是自己没写的函数，还是源文件没加入正确目标。动态 `FrameBuffer`、`Texture::Sample`、`Mesh::IsValid` 等空声明目前都不是完成本轮的前提。

## 9. 资产法线的实际情况

本次只检查资产，不修改资产。

- `triangle.obj`：有 +Z 法线，绕序计算出的面法线也是 +Z，适合作为第一张验证图。
- `cube.obj`：文件中的 `vn` 指向外侧，但现有三角形绕序计算出的面法线朝内。例如第一面 A=`(-1,-1,-1)`、B=`(1,-1,-1)`、C=`(1,1,-1)`，`(B-A)×(C-A)` 是 +Z，文件法线却是 -Z。Flat 与使用 OBJ 顶点法线的 Gouraud/Phong 因此可能出现明暗方向不一致。
- `low_poly_icosahedron.obj`：只有 +Y/-Y 两条法线，不能代表球形模型的完整平滑法线；不能用它判断插值光照是否足够平滑。

先用 triangle 和上述手算用例验证公式。做 Cube 严格对比前，由你后续统一绕序与法线的约定；不要通过朝向相机自动翻转法线掩盖问题，也不要在此次头文件改动中偷偷重写 OBJ。当前着色入口先关闭背面剔除，保持基础管线行为。

如果以后写 `Mesh::RecalculateNormals`，要区分硬边和光滑曲面：Cube 的每个面应保留硬边；平滑模型可以累加相邻面法线后归一化。不能只因为顶点位置相同就跨 UV 接缝或硬边焊接。

## 10. 建议断点、实验与完成标准

按下面顺序记录输入、预期值和实际值；这些是待你实现后的检查项目，本次没有宣称新光照测试已经通过。

- [ ] Light 两个函数的方向、恒定强度、距离衰减与非法值检查。
- [ ] Unlit 在整个模型使用统一颜色，不再按三角形编号换色。
- [ ] 法线可视化随模型旋转而变化，平移不改变法线。
- [ ] 非均匀缩放后，法线与变换后的切线点积仍接近零。
- [ ] 手算的 A/D/S、Lambert 与 Blinn-Phong 数值正确。
- [ ] 无光源、背光、零法线、零观察方向、零半程向量都有明确结果。
- [ ] 固定物体和光源，移动相机：D 不变，S 通常改变。
- [ ] 固定物体和相机，移动点光：D 与 S 改变；启用衰减后远处变暗。
- [ ] 设 `specular={0,0,0}` 后，Blinn-Phong 与 Lambert 一致。
- [ ] 分别观察 `shininess=8/32/128`；高光范围随指数增加而收缩。
- [ ] Flat/Gouraud/Phong 在一致的法线数据上对比；硬边 Cube 不一定表现平滑差异，最好再用合适的平滑曲面。
- [ ] 近面裁剪仍保留完整属性，Gouraud 不因裁剪而重新计算新顶点光照。
- [ ] 开启深度测试时，改变三角形提交顺序不改变遮挡结果；原有共享边测试通过。
- [ ] Debug/Release 构建及已有两项 CTest 仍通过；运行不会逐帧加载 OBJ。

适合逐步进入的调用链：

```text
RenderScene
  -> DrawMeshShaded
       -> Transform::NormalMatrix
       -> Material::Evaluate                    // Flat/Gouraud
            -> Material::EvaluateTerms
                 -> Light::DirectionFrom
                 -> Light::RadianceAt
       -> ClipTriangleToFrustum                // 沿用已有实现
       -> Transform::ToScreen                  // 沿用已有实现
       -> RasterizeTriangleShaded
            -> 已有覆盖/重心/深度/属性插值
            -> Material::Evaluate              // 仅 Phong
            -> Raster::EncodeLinearColor
            -> Raster::PutPixel
```

第一天可以只做到 Light 与 Material 的手算验证；第二步再接到一个三角形；最后接 OBJ 与三种着色对比。一次只改变一个因素，更容易区分空间、插值、公式和资产的问题。

## 11. 本次交付检查（2026-10-16）

- 6 个修改后的头文件分别独立、重复包含，MSVC C++20 严格语法检查通过。
- 本文 Light、Material、DrawMeshShaded、RasterizeTriangleShaded 的待填框架组合后通过语法检查；这是签名/包含关系检查，不是光照功能测试。
- `out/repair` 下 Debug、Release 构建成功；两种配置各自的两项现有 CTest 全部通过。
- SHA-256 核对：`src` 下 18 个已有 `.cpp` 与 `CMakeLists.txt` 修改前后一致。
- 改动范围为 6 个 `.h`、本文档和 README 入口；没有执行 Git 暂存、提交或推送。

新增着色函数仍是待实现声明，窗口仍走基础面色管线。完成各函数体和接线后，才验证第 10 节列出的新着色功能；现有测试通过不能替代这些检查。
