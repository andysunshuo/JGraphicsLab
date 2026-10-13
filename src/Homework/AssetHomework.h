#pragma once
#include "Homework/AssetHomeworkIds.h"
#include "Renderer/AssetPreview.h"
#include "Application/ModelDemos.h"
#include <array>

struct AssetHomework
{
    int id;
    const char *key;
    const wchar_t *title;
    const wchar_t *description;
    const wchar_t *path;
    const wchar_t *footer;
    const AssetPreviewFrame &(*render)(float angle);
};
const std::array<AssetHomework, 10> &AssetHomeworkRegistry();
const AssetHomework *FindAssetHomework(int id);
int ParseAssetHomeworkId(const std::string &name);
void ListAssetHomework();
bool IsAnimatedAssetHomework(int id);
std::size_t AssetHomeworkLoadCount(int id);
const AssetPreviewFrame &ShowAssetModel(int id, ModelDemo model, float angle);
const AssetPreviewFrame &ShowAssetImage(int id);
const AssetPreviewFrame &ShowAssetEnvironment(int id);

const AssetPreviewFrame &RenderHW50_1(float angle);
const AssetPreviewFrame &RenderHW50_2(float angle);
const AssetPreviewFrame &RenderHW50_3(float angle);
const AssetPreviewFrame &RenderHW50_4(float angle);
const AssetPreviewFrame &RenderHW50_5(float angle);
const AssetPreviewFrame &RenderHW50_6(float angle);
const AssetPreviewFrame &RenderHW50_7(float angle);
const AssetPreviewFrame &RenderHW50_8(float angle);
const AssetPreviewFrame &RenderHW50_9(float angle);
const AssetPreviewFrame &RenderHW50_10(float angle);
