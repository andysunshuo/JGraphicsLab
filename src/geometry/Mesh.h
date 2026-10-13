#pragma once

#include "../core/Vec3.h"
#include <vector>

struct Vec2
{
    float x = 0;
    float y = 0;
};

struct Vertex
{
    Vec3 position{};

    Vec2 uv{};

    Vec3 normal{};
};

struct Mesh
{
    // 顶点必须先定义；索引保存顶点编号，全工程只保留这一份 Mesh。
    std::vector<Vertex> vertices;

    std::vector<unsigned int> indices;
};
