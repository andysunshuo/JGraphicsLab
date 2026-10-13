#include "Homework/AssetHomework.h"

/*
HW50_2：茶壶 OBJ
配置写法：inline constexpr int kActiveHomework = 50_2;
等价写法：inline constexpr int kActiveHomework = HW50_2;
运行参数：--hw 50_2；本入口不占用 HW51 等原作业编号。
资源：Assets/Models/teapot.obj
用途：显示曲面测试模型，保留全部 6320 个三角形。
调用过程：Application -> 独立 AssetHomeworkRegistry -> RenderHW50_2 -> 缓存 OBJ -> 自动居中缩放 -> DrawMesh 的 MVP/裁剪/光栅化/深度 -> 放大到说明画布。
使用固定缓存，模型/图片首次加载后复用；失败输出中文路径与原因。
原 HW01～HW50 的实现、测试回调与状态不在此处修改。
*/
const AssetPreviewFrame &RenderHW50_2(float angle)
{
    return ShowAssetModel(HW50_2, ModelDemo::Teapot, angle);
}
