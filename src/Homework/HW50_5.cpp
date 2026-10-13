#include "Homework/AssetHomework.h"

/*
HW50_5：棋盘格颜色贴图
配置写法：inline constexpr int kActiveHomework = 50_5;
等价写法：inline constexpr int kActiveHomework = HW50_5;
运行参数：--hw 50_5；本入口不占用 HW51 等原作业编号。
资源：Assets/Textures/checker.png
用途：规则颜色图案适合检查 UV、重复和采样；这里展示原始贴图。
调用过程：Application -> 独立 AssetHomeworkRegistry -> RenderHW50_5 -> 缓存 WIC 解码 -> 保持原颜色并按比例放进资源画布 -> 文件路径和中文用途说明。
使用固定缓存，模型/图片首次加载后复用；失败输出中文路径与原因。
原 HW01～HW50 的实现、测试回调与状态不在此处修改。
*/
const AssetPreviewFrame &RenderHW50_5(float /* angle */)
{
    return ShowAssetImage(HW50_5);
}
