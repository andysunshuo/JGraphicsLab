#include "Homework/AssetHomework.h"

/*
HW50_6：木材颜色贴图
配置写法：inline constexpr int kActiveHomework = 50_6;
等价写法：inline constexpr int kActiveHomework = HW50_6;
运行参数：--hw 50_6；本入口不占用 HW51 等原作业编号。
资源：Assets/Textures/wood.jpg
用途：木纹基础颜色图；记录表面颜色，不记录表面高度或法线。
调用过程：Application -> 独立 AssetHomeworkRegistry -> RenderHW50_6 -> 缓存 WIC 解码 -> 保持原颜色并按比例放进资源画布 -> 文件路径和中文用途说明。
使用固定缓存，模型/图片首次加载后复用；失败输出中文路径与原因。
原 HW01～HW50 的实现、测试回调与状态不在此处修改。
*/
const AssetPreviewFrame &RenderHW50_6(float /* angle */)
{
    return ShowAssetImage(HW50_6);
}
