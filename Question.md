# JGraphicsLab
Computer Graphics from Scratch in C++ — 100 Hands-on Exercises

> 

    - [x] **Q01 — Rasterization02**：滤波,人和滤波-- 左边的图像通过傅里叶变换从图像空间变到频域空间。
- [ ] **Q02 — Horizontal & Vertical Lines**：使用循环绘制水平线、垂直线和十字坐标轴。


为什么不 为什么不直接靠 Depth Buffer ，而是要 背面剔除呢？  HW013
    // 4. 将浮点坐标转换为整数像素范围
    int xStart = static_cast<int>(std::floor(minX));
    int xEnd = static_cast<int>(std::ceil(maxX));

    int yStart = static_cast<int>(std::floor(minY));
    int yEnd = static_cast<int>(std::ceil(maxY));