#include "Homework/AssetHomework.h"

/*
HW50_8：砖墙法线贴图
配置写法：inline constexpr int kActiveHomework = 50_8;
等价写法：inline constexpr int kActiveHomework = HW50_8;
运行参数：--hw 50_8；本入口不占用 HW51 等原作业编号。
资源：Assets/NormalMaps/brick_normal.jpg
用途：RGB 编码切线空间方向；解码 n=2*RGB/255-1，平坦区域通常偏蓝紫色。
调用过程：Application -> 独立 AssetHomeworkRegistry -> RenderHW50_8 -> 缓存 WIC 解码 -> 保持原颜色并按比例放进资源画布 -> 文件路径和中文用途说明。
使用固定缓存，模型/图片首次加载后复用；失败输出中文路径与原因。
原 HW01～HW50 的实现、测试回调与状态不在此处修改。
*/
const AssetPreviewFrame &RenderHW50_8(float /* angle */)
{
    return ShowAssetImage(HW50_8);
}
