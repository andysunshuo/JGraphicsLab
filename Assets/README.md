# JRenderLab 测试资源（HW32–HW55）

整理日期：2026-10-13。实际资源根目录：`D:\github\JGraphicsLab\JRenderLab\Assets`。
当前仓库名是 JGraphicsLab；这里按请求保留 `JRenderLab/Assets` 相对路径。
资源已经下载、转换并验证，运行时无需下载脚本、Python、Unity、Unreal 或 FBX。

## 文件清单

下表大小为最终落盘文件的精确字节数。`asset_manifest.json` 记录 SHA-256、源文件链接、模型统计和 C++ 验证结果。

| 文件（相对 Assets） | 大小 / bytes | 数据 |
| --- | ---: | --- |
| [Environment/studio_small_09.hdr](Environment/studio_small_09.hdr) | 1,615,248 | 1024 × 512 / Radiance RGBE HDR |
| [HeightMaps/brick_height.jpg](HeightMaps/brick_height.jpg) | 269,978 | 1024 × 1024 / L |
| [Models/bunny.obj](Models/bunny.obj) | 7,460,681 | 35,132 vertices / 69,451 triangles |
| [Models/cube.obj](Models/cube.obj) | 1,144 | 24 vertices / 12 triangles |
| [Models/dragon.obj](Models/dragon.obj) | 27,721,782 | 125,307 vertices / 249,882 triangles |
| [Models/teapot.obj](Models/teapot.obj) | 682,613 | 3,808 vertices / 6,320 triangles |
| [NormalMaps/brick_normal.jpg](NormalMaps/brick_normal.jpg) | 1,935,906 | 1024 × 1024 / RGB |
| [Textures/brick_color.jpg](Textures/brick_color.jpg) | 1,456,114 | 1024 × 1024 / RGB |
| [Textures/checker.png](Textures/checker.png) | 1,855 | 512 × 512 / RGB |
| [Textures/wood.jpg](Textures/wood.jpg) | 871,537 | 1024 × 1024 / RGB |

## 模型统计与转换

Vertex 数量指最终 OBJ 中的 `v` 记录数；triangle 数量指 `f` 记录数。UV 和 normal 均覆盖全部三角形的三个角。

| 模型 | 原始 vertex | 导出 vertex | triangle | UV | normal |
| --- | ---: | ---: | ---: | --- | --- |
| bunny.obj | 35,947 | 35,132 | 69,451 | 是（每个面角） | 是（每个面角） |
| dragon.obj | 125,066 | 125,307 | 249,882 | 是（每个面角） | 是（每个面角） |
| teapot.obj | 3,644 | 3,808 | 6,320 | 是（每个面角） | 是（每个面角） |
| cube.obj | 8 | 24 | 12 | 是（每个面角） | 是（每个面角） |

- bunny：下载 [Stanford Bunny 原始 PLY 包](https://graphics.stanford.edu/pub/3Dscanrep/bunny.tar.gz)，读取 `bunny/reconstruction/bun_zipper.ply`，转换为 OBJ。移除未被面引用的顶点，保留原始三角面；UV 接缝会复制部分顶点。
- dragon：下载 [common-3d-test-models 的 XYZ RGB Dragon OBJ](https://github.com/alecjacobson/common-3d-test-models/blob/master/data/xyzrgb_dragon.obj)，来源为 [Stanford XYZ RGB Asian Dragon 数据集](https://graphics.stanford.edu/data/3Dscanrep/#xyzrgb)。这是仓库提供的预处理版本，不是完整 7,218,906 三角形的原扫描，也不是 `dragon_vrip.ply`。适合较复杂模型和 CPU 性能测试。
- teapot：下载 [Utah Teapot OBJ](https://github.com/alecjacobson/common-3d-test-models/blob/master/data/teapot.obj)，由 Martin Newell 的经典模型衍生。
- cube：下载 [tinyobjloader cube OBJ](https://github.com/tinyobjloader/tinyobjloader/blob/release/models/cube.obj)。原始 6 个四边形转为 12 个三角形；按面拆分为 24 个顶点，使 6 个面分别有硬边法线和完整方形 UV。
- 曲面模型重新计算面积加权的平滑单位法线，并补充绕 Y 轴的球面测试 UV。UV 用于教学测试，复杂凹面上存在投影拉伸，不能等同于人工制作的材质展开。
- 所有模型以包围盒中心为原点，等比缩放至最长边为 2，保持原始轴向和三角形绕序。转换时会剔除零面积三角形；具体数量见 manifest。
- 只输出 ASCII `v`、`vt`、`vn`、`f` 和注释，不引用 MTL 或外部材质文件。每个面恰好三个角，采用正的 1-based 索引，形式为 `f i/i/i j/j/j k/k/k`。

## 贴图与 HDR 来源

- `checker.png`：来自 [three.js 官方棋盘格 PNG](https://threejs.org/manual/examples/resources/images/checker.png)。原始 2×2 灰色棋盘格（128/192）采用最近邻放大并重复，整理成 512×512 RGB、32×32 个格子，每格 16×16 像素。适用于重复采样、摩尔纹及完整 Mipmap 金字塔测试。
- `brick_color.jpg`、`brick_normal.jpg`、`brick_height.jpg`：来自同一个 [ambientCG Bricks052](https://ambientcg.com/view?id=Bricks052) 1K-JPG 包，分别为 `Color`、`NormalGL`、`Displacement`，直接解压重命名，未改动 JPEG 内容。法线为 OpenGL +Y 的 tangent-space normal；高度图为 8-bit 灰度，白色更高。
- `wood.jpg`：来自 [ambientCG Wood051](https://ambientcg.com/view?id=Wood051) 1K-JPG 的 `Color`，直接解压重命名。
- `studio_small_09.hdr`：来自 [Poly Haven Studio Small 09](https://polyhaven.com/a/studio_small_09)，作者 Sergej Majboroda。下载 1K Radiance `.hdr`，1024×512、2:1 经纬度环境图，保留大于 1 的线性辐射亮度。
- 材质压缩包中的引擎或 DCC 配套文件未放入 Assets；这里仅保留通用图像数据。

## Homework 使用对应关系

| Homework | 资源 | 测试方式 |
| --- | --- | --- |
| HW32–HW34 | checker、brick_color、wood | 高频重复棋盘格、周期信号/采样、二维卷积、低通滤波和降采样；周期信号仍由代码生成 |
| HW35–HW37 | cube、teapot、checker | SSAA/MSAA、共享边、斜面与 Top-Left Rule；退化三角形和精确边界样例由代码构造 |
| HW38–HW44 | bunny、dragon、teapot、cube | 面法线/顶点法线、Lambert、Blinn-Phong、Flat/Gouraud/Phong、法线矩阵；可忽略 vn 并重新计算比较 |
| HW45–HW51 | 所有 OBJ 的 UV、checker、brick_color、wood | UV 映射、最近邻/双线性、透视校正插值、Mipmap、三线性和各向异性过滤；cube 的独立方形 UV 最适合基础验证 |
| HW52–HW53 | brick_normal、brick_height、brick_color、cube/teapot | tangent-space Normal Map、基于高度梯度的 Bump Mapping、几何 Displacement；三张砖块图共同使用相同 UV |
| HW54 | studio_small_09.hdr、teapot/dragon | 按反射方向采样 HDR 经纬度图，验证环境映射、曝光和 tone mapping |
| HW55 | 全部资源 | 整合 MVP、Clipping、Z-Buffer、MSAA、Blinn-Phong、UV 纹理、凹凸和环境反射的完整场景 |

## 纯 C++ CPU Rasterizer 读取约定

1. OBJ Loader 读取 `v x y z`、`vt u v`、`vn x y z`、三角形 `f v/vt/vn`；读入时把索引减 1。各属性索引已对齐，简单 Vertex 数组也能处理。无需 MTL Loader。
2. UV 接缝的 U 可能大于 1，这是为了避免横跨接缝的三角形出现错误插值。插值与 TBN 计算先使用原始 UV，采样时执行 Repeat：`u -= floor(u)`、`v -= floor(v)`；不要提前 clamp。
3. OBJ V=0 表示图像底部。若图像解码返回顶部第一行，采样行坐标使用 `(1-v)*(height-1)`，或只在加载时翻转一次行序。
4. PNG/JPG 用普通 8-bit 图像解码器读取；高度图是单通道，读取时使用其通道数或要求解码为 1/3 通道。颜色纹理做 sRGB→linear；Normal/Height 按线性数据使用，不能做 sRGB 转换。
5. Normal Map 解码为 `normalize(2*RGB/255-1)`，使用由位置边和 UV 差构建的 TBN 变换。法线贴图是 +Y；UV 退化处使用稳定的备用切线。
6. OBJ 没有预存 tangent。需要从三角形位置和 UV 计算 tangent/bitangent；cube 的各面保留硬边。Displacement 前需对 cube/低细分模型细分，只有 12 个三角形的 cube 不足以表现砖块几何细节。
7. HDR 必须读取为 RGB float，例如 `stbi_loadf()`；不能读取为 8-bit JPG，也不能把线性 HDR 当成 sRGB。按经纬度进行方向采样，最后曝光/tone mapping 后显示。
8. 工作目录若为仓库根目录，用 `JRenderLab/Assets/...`；若为 JRenderLab 目录，用 `Assets/...`。Visual Studio 从构建目录启动时应设置资源根路径或工作目录。

资源的格式可直接用于纯 C++ 渲染器。当前仓库的 `src` 尚未实现 OBJ、PNG/JPG 或 HDR 加载入口，因此“格式可用”不表示现有程序会自动显示这些资源；实现以上加载接口并绑定材质后即可使用。

## HW55 场景装配示例

- 相机：eye=(5,3,7)，target=(0,0,0)，up=(0,1,0)，FOV=45°，near=0.1，far=100。
- 地面：cube，scale=(4,0.05,3)，translate=(0,-1.1,0)，checker，UV 重复 8 次。
- bunny：translate=(-2,0,0)，使用白色 Blinn-Phong，比较 Flat/Gouraud/Phong。
- teapot：translate=(0,0,0)，wood，比较最近邻/双线性/三线性，添加 HDR 环境反射。
- dragon：translate=(2,0,0)，brick_color + brick_normal，作为复杂几何和 TBN 测试。
- cube：单独放在前景，用匹配的 brick_color/brick_height 比较 Bump 与细分后的 Displacement。
- 方向光：normalize(-1,-1,-1)，ambient=0.1。旋转各模型、非均匀缩放 teapot，检查裁剪、背面剔除、深度和法线矩阵。

上述是供 C++ 场景代码使用的装配参数，无需引擎专用场景文件。

## 验证结果

实际在 MSVC x64/C++20 编译的独立验证程序中使用 tinyobjloader 与 stb_image 读取全部最终文件。
检查了每个模型的三角面、全部角点索引、完整 UV/normal、有限数值、单位法线和非零面积；PNG/JPG 全像素解码成功；HDR 浮点解码成功、比例为 2:1，存在亮度大于 1 的像素。

```text
bunny: vertices=35132 triangles=69451 UV=yes normals=yes (all corners, finite values, unit normals, nondegenerate triangles)
dragon: vertices=125307 triangles=249882 UV=yes normals=yes (all corners, finite values, unit normals, nondegenerate triangles)
teapot: vertices=3808 triangles=6320 UV=yes normals=yes (all corners, finite values, unit normals, nondegenerate triangles)
cube: vertices=24 triangles=12 UV=yes normals=yes (all corners, finite values, unit normals, nondegenerate triangles)
Textures/checker.png: 512x512 channels=3 decoded OK
Textures/brick_color.jpg: 1024x1024 channels=3 decoded OK
Textures/wood.jpg: 1024x1024 channels=3 decoded OK
NormalMaps/brick_normal.jpg: 1024x1024 channels=3 decoded OK
HeightMaps/brick_height.jpg: 1024x1024 channels=1 decoded OK
Environment/studio_small_09.hdr: 1024x512 RGB float max=524 (Radiance HDR, equirectangular 2:1, decoded OK)
```



### 

