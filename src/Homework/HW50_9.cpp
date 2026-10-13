#include "Homework/AssetHomework.h"

/*
HW50_9：砖墙高度图
配置写法：inline constexpr int kActiveHomework = 50_9;
等价写法：inline constexpr int kActiveHomework = HW50_9;
运行参数：--hw 50_9；本入口不占用 HW51 等原作业编号。
资源：Assets/HeightMaps/brick_height.jpg
用途：灰度表示相对高度，较亮通常较高；可用于凹凸/位移，不是颜色输入。
调用过程：Application -> 独立 AssetHomeworkRegistry -> RenderHW50_9 -> 缓存 WIC 解码 -> 保持原颜色并按比例放进资源画布 -> 文件路径和中文用途说明。
使用固定缓存，模型/图片首次加载后复用；失败输出中文路径与原因。
原 HW01～HW50 的实现、测试回调与状态不在此处修改。
*/
const AssetPreviewFrame &RenderHW50_9(float /* angle */)
{
    return ShowAssetImage(HW50_9);
}
