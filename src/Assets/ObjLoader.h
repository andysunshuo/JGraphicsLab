#pragma once
#include "Geometry/Mesh.h"
#include <filesystem>
#include <string>

namespace Assets
{
struct LoadLimits
{
    std::uintmax_t maxFileBytes = 64 * 1024 * 1024;
    std::size_t maxLineBytes = 64 * 1024;
    std::size_t maxPositions = 500000;
    std::size_t maxAttributeRecords = 1000000;
    std::size_t maxVertices = 1500000;
    std::size_t maxTriangles = 500000;
};
struct MeshLoadResult
{
    Mesh mesh;
    std::string error;
    std::string warning;
    double milliseconds = 0;
    explicit operator bool() const
    {
        return error.empty() && !mesh.indices.empty();
    }
};
MeshLoadResult LoadObj(const std::filesystem::path &path, const LoadLimits &limits = {});
} // namespace Assets
