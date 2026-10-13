# tinyobjloader

使用工程已准备的本地 `out/assets_setup/sources/tiny_obj_loader.h` 与许可证，复制到源码目录后不依赖构建缓存或联网下载。

实现宏只在 `src/geometry/OBJLoader.cpp` 定义一次。库负责 OBJ 格式、缺失属性、负索引和三角化；应用层将独立位置/UV/法线索引转换为统一 Mesh，并验证索引与资源预算。

本分支只显示三角形的彩色顶点；本次修复保留此行为，不增加光照、纹理或三维投影。
