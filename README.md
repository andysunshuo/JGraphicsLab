# JGraphicsLab

经典光照与贴图已接通：默认木材 Cube，可在窗口菜单选择模型、材质和 Lambert／布林冯，按 1／2 切换光照，也支持 PNG 导出。运行、参数、资源对应关系及已知限制见 [OBJ 材质贴图与光照使用](docs/OBJ材质贴图与光照使用.md)。

此前的接口练习与待填框架保留在 [经典着色学习与 CPP 框架](docs/经典着色学习与CPP框架.md)，供对照学习；当前实现已经完成，不要再用空框架覆盖源码。

2026-10-15：基础 OBJ → MVP → 裁剪 → 三角形填充/深度管线已修通，实际帧缓冲改为 1000×1000，默认显示 Cube。运行和参数位置见 [基础 OBJ 渲染](docs/基础OBJ渲染_1000x1000.md)。

新目录的自有头文件接口与中文实现注释已补齐，建议从数学基础逐步实现 .cpp；步骤见 [头文件接口与实现顺序](docs/头文件接口与实现顺序.md)。资产目录已从 .gitignore 排除规则中豁免，提交时请勾选 assets 内模型。

此前的三顶点演示修复记录见 [当前分支编译修复](docs/当前分支编译修复.md)。当前分支已推进到基础 OBJ 实心渲染，以上面的最新文档为准；完整 HW 工程在 `main` 分支。
Computer Graphics from Scratch in C++ — 100 Hands-on Exercises

> 

- [x] **HW01 — Pixel Rendering**：使用 PutPixel() 绘制不同颜色的像素，理解 RGB、Framebuffer 和像素坐标。
- [ ] **HW02 — Horizontal & Vertical Lines**：使用循环绘制水平线、垂直线和十字坐标轴。
- [ ] **HW03 — Bresenham Line**：实现支持全部八个八分区的整数直线光栅化算法。
- [ ] **HW04 — Rectangle Rendering**：根据两个对角顶点绘制空心矩形和实心矩形。
- [ ] **HW05 — Circle Rendering**：使用圆方程实现空心圆和实心圆。
- [ ] **HW06 — Triangle Wireframe**：使用三个顶点和已有直线算法绘制三角形线框。
- [ ] **HW07 — Edge Function**：实现二维边函数，判断点位于有向边的哪一侧。
- [ ] **HW08 — Inside Triangle**：利用三个边函数判断采样点是否位于三角形内部。
- [ ] **HW09 — Triangle Rasterization**：计算三角形 Bounding Box，遍历并填充内部像素。
- [ ] **HW10 — Barycentric Coordinates**：计算重心坐标 α、β、γ，验证三个权重之和为 1。
- [ ] **HW11 — Color Interpolation**：给三个顶点设置 RGB 颜色，利用重心坐标实现平滑插值。
- [ ] **HW12 — Depth Buffer**：为每个像素存储深度，实现重叠三角形的遮挡关系。
- [ ] **HW13 — Back-Face Culling**：通过三角形有向面积和顶点绕序实现背面剔除。
- [ ] **HW14 — MSAA 2×2**：为每个像素设置四个采样点，独立处理覆盖、颜色、深度并进行 Resolve。
- [ ] **HW15 — Rasterizer Integration**：使用顶点数组和三角形索引绘制多个图元，检查共享边与绘制顺序。
- [ ] **HW16 — Vec2**：定义二维向量，实现加法、减法、数乘、长度和归一化。
- [ ] **HW17 — Vec3**：实现三维向量的 Dot Product、Cross Product、Length 和 Normalize。
- [ ] **HW18 — Vec4**：实现四维向量，理解齐次坐标中 Position 与 Direction 的区别。
- [ ] **HW19 — Mat3**：实现 3×3 矩阵、单位矩阵、矩阵乘法和矩阵乘向量。
- [ ] **HW20 — Mat4**：实现 4×4 矩阵、单位矩阵、矩阵乘法和矩阵乘向量。
- [ ] **HW21 — 2D Transformation**：通过齐次变换实现二维平移、旋转、缩放、反射和错切。
- [ ] **HW22 — 3D Transformation**：实现三维平移、缩放和绕 X/Y/Z 轴的旋转。
- [ ] **HW23 — Model Matrix**：构造模型矩阵，控制三维物体的位置、方向和大小。
- [ ] **HW24 — View Matrix**：使用 LookAt 构造观察矩阵，完成世界空间到相机空间的转换。
- [ ] **HW25 — Orthographic Projection**：实现正交投影矩阵，将三维坐标转换到裁剪空间。
- [ ] **HW26 — Perspective Projection**：根据 FOV、Aspect、Near、Far 构造透视投影矩阵。
- [ ] **HW27 — Perspective Divide**：实现齐次坐标透视除法，将 Clip Space 转换到 NDC。
- [ ] **HW28 — Viewport Transformation**：将 NDC 坐标映射到屏幕坐标，并验证坐标轴方向和边界。
- [ ] **HW29 — MVP Pipeline**：组合 Model、View、Projection，绘制可以旋转的三维线框立方体。
- [ ] **HW30 — Frustum Clipping**：在齐次裁剪空间中裁剪三角形，正确处理近裁剪面和透视除法。
- [ ] **HW31 — 3D Triangle Rasterization**：将三维立方体的三角面投影到屏幕，填充颜色并实现深度遮挡。
- [ ] **HW32 — Aliasing**：绘制高频图案、斜线和重复纹理，观察锯齿与摩尔纹。
- [ ] **HW33 — Signal Sampling**：生成周期信号，比较不同采样频率的结果，理解 Nyquist Sampling。
- [ ] **HW34 — Image Filtering**：实现二维卷积和低通滤波，对比直接降采样与滤波后降采样。
- [ ] **HW35 — SSAA**：以更高分辨率渲染图像，再进行过滤和降采样。
- [ ] **HW36 — MSAA Improvement**：将多重采样应用于三维光栅化场景，处理深度、覆盖与颜色。
- [ ] **HW37 — Rasterization Rules**：实现 Top-Left Rule，处理退化三角形、共享边和像素中心采样规则。
- [ ] **HW38 — Surface Normal**：根据三角形顶点计算面法线，并根据邻接面计算顶点法线。
- [ ] **HW39 — Lambert Lighting**：实现 Ambient 与 Diffuse，利用法线和光照方向计算亮度。
- [ ] **HW40 — Blinn-Phong Lighting**：实现 Ambient、Diffuse、Specular，支持控制材质参数和光源位置。
- [ ] **HW41 — Flat Shading**：每个三角形只计算一次光照，观察模型的平面着色效果。
- [ ] **HW42 — Gouraud Shading**：在顶点计算光照颜色，再插值到三角形内部。
- [ ] **HW43 — Phong Shading**：对顶点法线插值，在每个像素计算光照，并与 Gouraud 对比。
- [ ] **HW44 — Normal Transformation**：实现法线矩阵，正确处理模型的非均匀缩放。
- [ ] **HW45 — UV Mapping**：为三角形和立方体添加 UV 坐标，理解纹理坐标与顶点属性。
- [ ] **HW46 — Texture Sampling**：读取纹理图像，实现 UV 到纹理像素的映射及最近邻采样。
- [ ] **HW47 — Bilinear Filtering**：根据周围四个 Texel 进行双线性插值。
- [ ] **HW48 — Perspective-Correct Interpolation**：利用 1/w 修正纹理坐标及其他属性的透视插值。
- [ ] **HW49 — Mipmap Generation**：逐级降采样生成 Mipmap，建立纹理金字塔。
- [ ] **HW50 — Trilinear Filtering**：估算纹理 LOD，在相邻 Mipmap 层级之间进行插值。
- [ ] **HW51 — Anisotropic Filtering**：实现基础各向异性纹理过滤实验，对比斜视表面的采样质量。
- [ ] **HW52 — Bump Mapping**：根据高度图梯度修改表面法线，产生凹凸视觉效果。
- [ ] **HW53 — Displacement Mapping**：根据高度图移动几何顶点，对比真实位移与 Bump Mapping。
- [ ] **HW54 — Environment Mapping**：根据视线和表面反射方向采样环境纹理，实现基础环境反射。
- [ ] **HW55 — Final Software Rasterizer**：整合 MVP、Clipping、Z-Buffer、MSAA、纹理采样与 Blinn-Phong，渲染完整的可旋转三维模型。
- [ ] **HW56 — Implicit Geometry**：使用隐式方程表示圆、球体及其他基础几何体，判断点与表面的空间关系。
- [ ] **HW57 — Signed Distance Function**：实现球体和盒子的 SDF，计算空间点到表面的有符号距离。
- [ ] **HW58 — Constructive Solid Geometry**：使用 Union、Intersection、Difference 组合隐式几何体。
- [ ] **HW59 — Explicit Geometry**：使用顶点与三角形索引定义几何网格，理解显式几何表示。
- [ ] **HW60 — Mesh Data Structure**：构建 Vertex、Edge、Face 和邻接关系，支持查询网格拓扑。
- [ ] **HW61 — Point Cloud**：生成、变换并显示三维点云，比较点云和三角网格的区别。
- [ ] **HW62 — Linear Interpolation**：实现控制点之间的线性插值，观察插值参数对位置的影响。
- [ ] **HW63 — Quadratic Bézier Curve**：使用三个控制点实现二次贝塞尔曲线。
- [ ] **HW64 — Cubic Bézier Curve**：使用四个控制点实现三次贝塞尔曲线。
- [ ] **HW65 — de Casteljau Algorithm**：实现递归贝塞尔求值，对比直接多项式公式的结果。
- [ ] **HW66 — Bézier Properties**：验证曲线的端点、凸包、仿射不变性和控制点影响。
- [ ] **HW67 — Piecewise Bézier Curves**：连接多段贝塞尔曲线，实现 C0 与 C1 连续。
- [ ] **HW68 — Bézier Surface**：通过二维参数生成贝塞尔曲面，并转换为三角网格。
- [ ] **HW69 — Mesh Topology**：识别三角网格的边界边、内部边和相邻三角形。
- [ ] **HW70 — Mesh Normals**：计算几何网格的面法线和顶点法线，对比硬边与平滑表面。
- [ ] **HW71 — Loop Subdivision**：实现三角网格 Loop 细分，观察模型曲面逐渐平滑。
- [ ] **HW72 — Catmull-Clark Subdivision**：实现四边形网格细分，将低多边形模型转为平滑曲面。
- [ ] **HW73 — Mesh Simplification**：通过边折叠减少三角形数量，观察模型细节损失。
- [ ] **HW74 — Quadric Error Metrics**：使用 QEM 评价边折叠误差，实现基于误差的网格简化。
- [ ] **HW75 — Geometry Integration**：整合曲线、曲面、网格细分及简化，制作 Geometry Processor。
- [ ] **HW76 — Ray Representation**：定义 Ray 的 Origin、Direction 和参数方程，计算射线上任意点。
- [ ] **HW77 — Primary Ray Generation**：根据相机位置、FOV 和图像尺寸，为每个像素生成 Primary Ray。
- [ ] **HW78 — Ray-Sphere Intersection**：求解射线与球体的交点，返回最近的正参数交点。
- [ ] **HW79 — Ray-Plane Intersection**：实现射线与无限平面的求交，处理平行及退化情况。
- [ ] **HW80 — Ray-Triangle Intersection**：实现 Möller-Trumbore 算法，计算交点与重心坐标。
- [ ] **HW81 — Basic Ray Caster**：逐像素发射射线，遍历几何体并输出最近交点颜色。
- [ ] **HW82 — Surface Normal Visualization**：将交点表面法线映射到 RGB，检查求交与法线计算。
- [ ] **HW83 — Shadow Ray**：从交点向光源发射阴影射线，实现硬阴影。
- [ ] **HW84 — Reflection Ray**：根据入射方向和法线计算反射射线，实现递归镜面反射。
- [ ] **HW85 — Refraction Ray**：根据 Snell's Law 计算折射方向，处理全反射。
- [ ] **HW86 — Fresnel Reflection**：实现 Fresnel-Schlick 近似，控制反射与折射比例。
- [ ] **HW87 — Whitted Ray Tracer**：整合直接光照、阴影、反射、折射，渲染镜面球及玻璃球。
- [ ] **HW88 — Axis-Aligned Bounding Box**：计算几何体 AABB，理解包围盒结构。
- [ ] **HW89 — Ray-AABB Intersection**：使用 Slab Method 实现射线与 AABB 求交。
- [ ] **HW90 — Uniform Spatial Grid**：实现基础空间网格加速，并比较暴力求交和网格查询。
- [ ] **HW91 — BVH Construction**：构建二叉包围盒层次结构，组织大量三角形。
- [ ] **HW92 — BVH Traversal**：实现 BVH 遍历与最近交点查询。
- [ ] **HW93 — BVH Splitting**：实现最长轴划分和中位数划分，比较节点分布。
- [ ] **HW94 — Surface Area Heuristic**：使用 SAH 优化 BVH 划分策略，记录性能数据。
- [ ] **HW95 — Acceleration Benchmark**：比较 Brute Force、Uniform Grid、BVH 的求交次数与运行耗时。
- [ ] **HW96 — Radiometry & BRDF**：理解 Radiance、Irradiance 等物理量，实现 Lambertian BRDF。
- [ ] **HW97 — Rendering Equation & Monte Carlo**：理解渲染方程，实现 Monte Carlo 积分与半球采样，验证 PDF 权重。
- [ ] **HW98 — Direct Lighting & Path Tracing**：实现面光源直接光照、软阴影及递归间接光照。
- [ ] **HW99 — Russian Roulette & Sampling**：实现路径随机终止和每像素多样本累积，对比不同 SPP 的噪声。
- [ ] **HW100 — Final Path Tracer**：整合 Camera、Ray Intersection、BVH、BRDF、Monte Carlo 与间接光照，渲染 Cornell Box。
