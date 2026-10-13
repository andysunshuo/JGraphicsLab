#include "Homework/Runner.h"
#include "Renderer/Framebuffer.h"
#include "Renderer/DepthBuffer.h"
#include <cstdio>
TestResult RunMathHomeworkTest(int number)
{
    if (number == 0)
        return RunAllMathTests();
    const Homework *hw = FindHomework(number);
    if (!hw)
    {
        std::fprintf(stderr, "无效作业编号：%d（应为 1～50）\n", number);
        return {0, 1};
    }
    if (!hw->test)
    {
        std::printf("HW%02d：没有已实现的数学测试；%s\n", number, HomeworkStatusName(hw->status));
        return {};
    }
    return hw->test();
}
TestResult RunAllMathTests()
{
    TestResult total;
    for (const auto &hw : HomeworkRegistry())
        if (hw.test)
        {
            const TestResult result = hw.test();
            total.passed += result.passed;
            total.failed += result.failed;
        }
    std::printf("全部已有数学测试：%d PASS，%d FAIL\n", total.passed, total.failed);
    return total;
}
bool RunHomework(int number, bool runTests)
{
    const Homework *hw = FindHomework(number == 0 ? 31 : number);
    if (!hw)
    {
        std::fprintf(stderr, "无效作业编号：%d\n", number);
        return false;
    }
    if (runTests && (number == 0 || hw->test) && RunMathHomeworkTest(number).failed)
        return false;
    Raster::Clear(Raster::MakeColor(0, 0, 0));
    ClearDepthBuffer();
    if (hw->render)
        hw->render();
    if (hw->status == HomeworkStatus::NotImplemented)
        return false;
    if (runTests && hw->status == HomeworkStatus::DemoOnly)
        std::printf("HW%02d：%s，见验收文档中的限制\n", number, HomeworkStatusName(hw->status));
    return true;
}
void ListHomework()
{
    for (const auto &hw : HomeworkRegistry())
        std::printf("HW%02d %-30s %s\n", hw.number, hw.name, HomeworkStatusName(hw.status));
}
