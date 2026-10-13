#include "Homework/AssetHomework.h"

/*
HW50_10：HDR 环境图
配置写法：inline constexpr int kActiveHomework = 50_10;
等价写法：inline constexpr int kActiveHomework = HW50_10;
运行参数：--hw 50_10；本入口不占用 HW51 等原作业编号。
资源：Assets/Environment/studio_small_09.hdr
用途：360 度全景用于环境反射/照明；HDR 浮点亮度可大于 1，显示前压缩亮度。
调用过程：Application -> 独立 AssetHomeworkRegistry -> RenderHW50_10 -> 缓存 HDR 解码 -> Reinhard 与 gamma 映射 -> 全景图和镜面球方向采样 -> 中文说明。
使用固定缓存，模型/图片首次加载后复用；失败输出中文路径与原因。
原 HW01～HW50 的实现、测试回调与状态不在此处修改。
*/
const AssetPreviewFrame &RenderHW50_10(float /* angle */)
{
    return ShowAssetEnvironment(HW50_10);
}
