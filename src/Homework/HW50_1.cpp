#include "Homework/AssetHomework.h"

/*
HW50_1：立方体 OBJ
配置写法：inline constexpr int kActiveHomework = 50_1;
等价写法：inline constexpr int kActiveHomework = HW50_1;
运行参数：--hw 50_1；本入口不占用 HW51 等原作业编号。
资源：Assets/Models/cube.obj
用途：显示模型的真实三角形几何；面法线方向映射为颜色。
调用过程：Application -> 独立 AssetHomeworkRegistry -> RenderHW50_1 -> 缓存 OBJ -> 自动居中缩放 -> DrawMesh 的 MVP/裁剪/光栅化/深度 -> 放大到说明画布。
使用固定缓存，模型/图片首次加载后复用；失败输出中文路径与原因。
原 HW01～HW50 的实现、测试回调与状态不在此处修改。
*/
const AssetPreviewFrame &RenderHW50_1(float angle)
{
    return ShowAssetModel(HW50_1, ModelDemo::Cube, angle);
}
