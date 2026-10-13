#include "Geometry/Surface.h"
#include <cmath>
#include <algorithm>
#include <stdexcept>

bool IsTopLeftEdge(const Point2 &start, const Point2 &end)
{
    const float dx = end.x - start.x, dy = end.y - start.y;
    return dy < 0 || (dy == 0 && dx < 0);
}
Vec3 FaceNormal(Vec3 a, Vec3 b, Vec3 c)
{
    return Normalize(Cross(Subtract(b, a), Subtract(c, a)));
}
std::vector<Vec3> ComputeVertexNormals(Mesh &mesh)
{
    if (mesh.indices.size() % 3 != 0 || mesh.indices.size() > 1500000 || mesh.vertices.size() > 1500000)
        throw std::invalid_argument("法线计算的三角索引或顶点数量无效/超出预算");
    std::vector<Vec3> faces(mesh.indices.size() / 3), sums(mesh.vertices.size());
    for (std::size_t i = 0; i < faces.size(); ++i)
    {
        const auto a = mesh.indices[i * 3], b = mesh.indices[i * 3 + 1], c = mesh.indices[i * 3 + 2];
        if (a >= mesh.vertices.size() || b >= mesh.vertices.size() || c >= mesh.vertices.size())
            throw std::invalid_argument("计算法线时发现非法面索引");
        const Vec3 cross = Cross(Subtract(mesh.vertices[b].position, mesh.vertices[a].position),
                                 Subtract(mesh.vertices[c].position, mesh.vertices[a].position));
        if (!IsFinite(cross))
            throw std::invalid_argument("计算法线时坐标无效");
        faces[i] = Normalize(cross);
        // 叉积长度正比于面积，累加未归一化叉积即为面积加权邻接面法线。
        sums[a] = Add(sums[a], cross);
        sums[b] = Add(sums[b], cross);
        sums[c] = Add(sums[c], cross);
    }
    for (std::size_t i = 0; i < sums.size(); ++i)
        mesh.vertices[i].normal = Normalize(sums[i]);
    return faces;
}
Mat3 MakeNormalMatrix(const Mat4 &m)
{
    const float a = m.m[0][0], b = m.m[0][1], c = m.m[0][2], d = m.m[1][0], e = m.m[1][1], f = m.m[1][2],
                g = m.m[2][0], h = m.m[2][1], i = m.m[2][2];
    const float det = a * (e * i - f * h) - b * (d * i - f * g) + c * (d * h - e * g);
    if (!std::isfinite(det) || std::abs(det) <= 1e-8f)
        throw std::invalid_argument("法线变换矩阵奇异或非有限");
    // 余子式矩阵除以行列式就是 inverse(A) 的转置。
    Mat3 r{};
    r.m[0][0] = (e * i - f * h) / det;
    r.m[0][1] = (f * g - d * i) / det;
    r.m[0][2] = (d * h - e * g) / det;
    r.m[1][0] = (c * h - b * i) / det;
    r.m[1][1] = (a * i - c * g) / det;
    r.m[1][2] = (b * g - a * h) / det;
    r.m[2][0] = (b * f - c * e) / det;
    r.m[2][1] = (c * d - a * f) / det;
    r.m[2][2] = (a * e - b * d) / det;
    return r;
}
Mesh MakeUVSphere(int slices, int stacks)
{
    if (slices < 3 || stacks < 2 || slices > 128 || stacks > 64)
        throw std::invalid_argument("学习球体细分数量超出范围");
    constexpr float pi = 3.14159265358979323846f;
    Mesh mesh;
    for (int j = 0; j <= stacks; ++j)
        for (int i = 0; i <= slices; ++i)
        {
            const float phi = pi * j / stacks, theta = 2 * pi * i / slices;
            const Vec3 n{std::sin(phi) * std::cos(theta), std::cos(phi), std::sin(phi) * std::sin(theta)};
            mesh.vertices.push_back(
                {Multiply(n, 0.9f), n, {static_cast<float>(i) / slices, 1 - static_cast<float>(j) / stacks}});
        }
    const auto face = [&](Index a, Index b, Index c) {
        const auto pa = mesh.vertices[a].position, pb = mesh.vertices[b].position,
                   pc = mesh.vertices[c].position;
        if (Dot(Cross(Subtract(pb, pa), Subtract(pc, pa)), Add(Add(pa, pb), pc)) < 0)
            std::swap(b, c);
        mesh.indices.insert(mesh.indices.end(), {a, b, c});
    };
    for (int j = 0; j < stacks; ++j)
        for (int i = 0; i < slices; ++i)
        {
            const Index a = j * (slices + 1) + i, b = a + 1, c = a + slices + 1, d = c + 1;
            if (j != 0)
                face(a, c, b);
            if (j != stacks - 1)
                face(b, c, d);
        }
    return mesh;
}
Mesh MakeUVTriangle()
{
    Mesh mesh;
    mesh.vertices = {{{-1, -0.8f, 0}, {0, 0, 1}, {0, 0}},
                     {{1, -0.8f, 0}, {0, 0, 1}, {1, 0}},
                     {{-0.8f, 0.9f, 0}, {0, 0, 1}, {0, 1}}};
    mesh.indices = {0, 1, 2};
    return mesh;
}
Mesh MakeUVCube()
{
    // 每面四个独立顶点：同一几何角在不同面有不同法线/UV，保留硬边与接缝。
    const Vec3 faces[6][4] = {{{-1, -1, 1}, {1, -1, 1}, {1, 1, 1}, {-1, 1, 1}},
                              {{1, -1, -1}, {-1, -1, -1}, {-1, 1, -1}, {1, 1, -1}},
                              {{-1, -1, -1}, {-1, -1, 1}, {-1, 1, 1}, {-1, 1, -1}},
                              {{1, -1, 1}, {1, -1, -1}, {1, 1, -1}, {1, 1, 1}},
                              {{-1, 1, 1}, {1, 1, 1}, {1, 1, -1}, {-1, 1, -1}},
                              {{-1, -1, -1}, {1, -1, -1}, {1, -1, 1}, {-1, -1, 1}}};
    const Vec2 uv[4] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
    Mesh mesh;
    for (const auto &face : faces)
    {
        const auto base = static_cast<Index>(mesh.vertices.size());
        const auto n = FaceNormal(face[0], face[1], face[2]);
        for (int i = 0; i < 4; ++i)
            mesh.vertices.push_back({face[i], n, uv[i]});
        mesh.indices.insert(mesh.indices.end(), {base, base + 1, base + 2, base, base + 2, base + 3});
    }
    return mesh;
}
