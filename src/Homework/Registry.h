#pragma once
#include "Homework/Entries.h"
#include <array>
enum class HomeworkType
{
    Math,
    Render,
    Both
};
enum class HomeworkStatus
{
    Implemented,
    DemoOnly,
    NotImplemented
};
struct Homework
{
    int number;
    const char *name;
    HomeworkType type;
    TestResult (*test)();
    void (*render)();
    HomeworkStatus status;
};
const std::array<Homework, 50> &HomeworkRegistry();
const Homework *FindHomework(int number);
const char *HomeworkStatusName(HomeworkStatus status);
void PrintNotImplemented(int number);
