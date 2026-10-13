#include "Homework/Registry.h"
#include <cstdio>

const std::array<Homework, 50> &HomeworkRegistry()
{
    static const std::array<Homework, 50> registry{{
        {1, "像素绘制", HomeworkType::Render, nullptr, RenderHW01, HomeworkStatus::NotImplemented},
        {2, "水平线与垂直线", HomeworkType::Render, nullptr, RenderHW02, HomeworkStatus::Implemented},
        {3, "Bresenham 直线", HomeworkType::Render, nullptr, RenderHW03, HomeworkStatus::Implemented},
        {4, "矩形绘制", HomeworkType::Render, nullptr, RenderHW04, HomeworkStatus::Implemented},
        {5, "圆形绘制", HomeworkType::Render, nullptr, RenderHW05, HomeworkStatus::Implemented},
        {6, "三角形线框", HomeworkType::Render, nullptr, RenderHW06, HomeworkStatus::Implemented},
        {7, "边函数", HomeworkType::Render, nullptr, RenderHW07, HomeworkStatus::Implemented},
        {8, "三角形内部判断", HomeworkType::Render, nullptr, RenderHW08, HomeworkStatus::Implemented},
        {9, "三角形光栅化", HomeworkType::Render, nullptr, RenderHW09, HomeworkStatus::Implemented},
        {10, "重心坐标", HomeworkType::Both, TestHW10, RenderHW10, HomeworkStatus::Implemented},
        {11, "颜色插值", HomeworkType::Render, nullptr, RenderHW11, HomeworkStatus::Implemented},
        {12, "深度缓冲演示", HomeworkType::Render, nullptr, RenderHW12, HomeworkStatus::DemoOnly},
        {13, "背面剔除", HomeworkType::Render, nullptr, RenderHW13, HomeworkStatus::Implemented},
        {14, "二维 MSAA 演示", HomeworkType::Render, nullptr, RenderHW14, HomeworkStatus::DemoOnly},
        {15, "索引图元整合", HomeworkType::Both, TestHW15, RenderHW15, HomeworkStatus::Implemented},
        {16, "二维向量", HomeworkType::Math, TestHW16, nullptr, HomeworkStatus::Implemented},
        {17, "三维向量", HomeworkType::Math, TestHW17, nullptr, HomeworkStatus::Implemented},
        {18, "齐次向量", HomeworkType::Math, TestHW18, nullptr, HomeworkStatus::Implemented},
        {19, "三阶矩阵", HomeworkType::Math, TestHW19, nullptr, HomeworkStatus::Implemented},
        {20, "四阶矩阵", HomeworkType::Math, TestHW20, nullptr, HomeworkStatus::Implemented},
        {21, "二维变换", HomeworkType::Both, TestHW21, RenderHW21, HomeworkStatus::Implemented},
        {22, "三维变换", HomeworkType::Math, TestHW22, nullptr, HomeworkStatus::Implemented},
        {23, "模型矩阵", HomeworkType::Math, TestHW23, nullptr, HomeworkStatus::Implemented},
        {24, "观察矩阵", HomeworkType::Math, TestHW24, nullptr, HomeworkStatus::Implemented},
        {25, "正交投影", HomeworkType::Math, TestHW25, nullptr, HomeworkStatus::Implemented},
        {26, "透视投影", HomeworkType::Math, TestHW26, nullptr, HomeworkStatus::Implemented},
        {27, "透视除法", HomeworkType::Math, TestHW27, nullptr, HomeworkStatus::Implemented},
        {28, "视口变换", HomeworkType::Math, TestHW28, nullptr, HomeworkStatus::Implemented},
        {29, "线框立方体 MVP", HomeworkType::Both, TestHW29, RenderHW29, HomeworkStatus::Implemented},
        {30, "视锥裁剪", HomeworkType::Both, TestHW30, RenderHW30, HomeworkStatus::Implemented},
        {31, "三维光栅化与深度", HomeworkType::Both, TestHW31, RenderHW31, HomeworkStatus::Implemented},
        {32, "走样", HomeworkType::Both, TestHW32, RenderHW32, HomeworkStatus::Implemented},
        {33, "信号采样", HomeworkType::Both, TestHW33, RenderHW33, HomeworkStatus::Implemented},
        {34, "图像滤波", HomeworkType::Both, TestHW34, RenderHW34, HomeworkStatus::Implemented},
        {35, "超采样", HomeworkType::Both, TestHW35, RenderHW35, HomeworkStatus::Implemented},
        {36, "三维 MSAA", HomeworkType::Both, TestHW36, RenderHW36, HomeworkStatus::Implemented},
        {37, "光栅化规则", HomeworkType::Both, TestHW37, RenderHW37, HomeworkStatus::Implemented},
        {38, "表面法线", HomeworkType::Both, TestHW38, RenderHW38, HomeworkStatus::Implemented},
        {39, "Lambert 光照", HomeworkType::Both, TestHW39, RenderHW39, HomeworkStatus::Implemented},
        {40, "Blinn-Phong 光照", HomeworkType::Both, TestHW40, RenderHW40, HomeworkStatus::Implemented},
        {41, "平面着色", HomeworkType::Both, TestHW41, RenderHW41, HomeworkStatus::Implemented},
        {42, "Gouraud 着色", HomeworkType::Both, TestHW42, RenderHW42, HomeworkStatus::Implemented},
        {43, "Phong 着色", HomeworkType::Both, TestHW43, RenderHW43, HomeworkStatus::Implemented},
        {44, "法线变换", HomeworkType::Both, TestHW44, RenderHW44, HomeworkStatus::Implemented},
        {45, "UV 映射", HomeworkType::Both, TestHW45, RenderHW45, HomeworkStatus::Implemented},
        {46, "纹理采样", HomeworkType::Both, TestHW46, RenderHW46, HomeworkStatus::Implemented},
        {47, "双线性滤波", HomeworkType::Both, TestHW47, RenderHW47, HomeworkStatus::Implemented},
        {48, "透视正确插值", HomeworkType::Both, TestHW48, RenderHW48, HomeworkStatus::Implemented},
        {49, "Mipmap 生成", HomeworkType::Both, TestHW49, RenderHW49, HomeworkStatus::Implemented},
        {50, "三线性滤波", HomeworkType::Both, TestHW50, RenderHW50, HomeworkStatus::Implemented},
    }};
    return registry;
}
const Homework *FindHomework(int number)
{
    const auto &registry = HomeworkRegistry();
    // 显式保留 50 个 switch case；编号只绑定注册表中的唯一条目。
    switch (number)
    {
    case 1:
        return &registry[0];
    case 2:
        return &registry[1];
    case 3:
        return &registry[2];
    case 4:
        return &registry[3];
    case 5:
        return &registry[4];
    case 6:
        return &registry[5];
    case 7:
        return &registry[6];
    case 8:
        return &registry[7];
    case 9:
        return &registry[8];
    case 10:
        return &registry[9];
    case 11:
        return &registry[10];
    case 12:
        return &registry[11];
    case 13:
        return &registry[12];
    case 14:
        return &registry[13];
    case 15:
        return &registry[14];
    case 16:
        return &registry[15];
    case 17:
        return &registry[16];
    case 18:
        return &registry[17];
    case 19:
        return &registry[18];
    case 20:
        return &registry[19];
    case 21:
        return &registry[20];
    case 22:
        return &registry[21];
    case 23:
        return &registry[22];
    case 24:
        return &registry[23];
    case 25:
        return &registry[24];
    case 26:
        return &registry[25];
    case 27:
        return &registry[26];
    case 28:
        return &registry[27];
    case 29:
        return &registry[28];
    case 30:
        return &registry[29];
    case 31:
        return &registry[30];
    case 32:
        return &registry[31];
    case 33:
        return &registry[32];
    case 34:
        return &registry[33];
    case 35:
        return &registry[34];
    case 36:
        return &registry[35];
    case 37:
        return &registry[36];
    case 38:
        return &registry[37];
    case 39:
        return &registry[38];
    case 40:
        return &registry[39];
    case 41:
        return &registry[40];
    case 42:
        return &registry[41];
    case 43:
        return &registry[42];
    case 44:
        return &registry[43];
    case 45:
        return &registry[44];
    case 46:
        return &registry[45];
    case 47:
        return &registry[46];
    case 48:
        return &registry[47];
    case 49:
        return &registry[48];
    case 50:
        return &registry[49];
    default:
        return nullptr;
    }
}
const char *HomeworkStatusName(HomeworkStatus status)
{
    switch (status)
    {
    case HomeworkStatus::Implemented:
        return "已有实现";
    case HomeworkStatus::DemoOnly:
        return "仅演示或部分实现";
    default:
        return "未实现（Not Implemented）";
    }
}
void PrintNotImplemented(int number)
{
    std::printf("HW%02d：未实现（Not Implemented），未执行通过测试\n", number);
}
