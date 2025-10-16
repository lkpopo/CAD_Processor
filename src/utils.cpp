#include "utils.h"

    // ------------------ 创建圆形近似 ------------------
std::vector<Vertex> makeCircle(double cx, double cy, double r, int segments = 64)
{
    std::vector<Vertex> pts;
    for (int i = 0; i < segments; ++i) {
        double theta = 2.0 * M_PI * i / segments;
        pts.push_back({ static_cast<float>(cx + r * cos(theta)), static_cast<float>(cy + r * sin(theta)), 0.0 });
    }
    return pts;
}

// 生成柱体顶点，用于 GL_TRIANGLES 绘制
std::vector<Vertex> generateExtrudedMesh(const std::vector<Vertex>& bottomVerts, float height)
{
    std::vector<Vertex> vertices;
    size_t n = bottomVerts.size();
    if (n < 3) return vertices;

    // 1️⃣ 顶面顶点
    std::vector<Vertex> topVerts;
    for (auto& b : bottomVerts)
        topVerts.push_back({b.x, b.y, b.z + height});

    // 2️⃣ 使用 earcut 三角化底面
    using Point = std::pair<float, float>;
    std::vector<std::vector<Point>> polygon;
    std::vector<Point> outer;
    for (auto& v : bottomVerts) outer.push_back({v.x, v.y});
    polygon.push_back(outer); // 单外环

    std::vector<uint32_t> indices = mapbox::earcut<uint32_t>(polygon);

    // 3️⃣ 底面三角形
    for (size_t i = 0; i < indices.size(); i += 3) {
        vertices.push_back(bottomVerts[indices[i]]);
        vertices.push_back(bottomVerts[indices[i+1]]);
        vertices.push_back(bottomVerts[indices[i+2]]);
    }

    // 4️⃣ 顶面三角形（法线朝外，顶面需要逆序）
    for (size_t i = 0; i < indices.size(); i += 3) {
        vertices.push_back(topVerts[indices[i]]);
        vertices.push_back(topVerts[indices[i+2]]);
        vertices.push_back(topVerts[indices[i+1]]);
    }

    // 5️⃣ 侧面三角形
    for (size_t i = 0; i < n; ++i) {
        size_t next = (i + 1) % n;
        Vertex b0 = bottomVerts[i];
        Vertex b1 = bottomVerts[next];
        Vertex t0 = topVerts[i];
        Vertex t1 = topVerts[next];

        // 三角形 1
        vertices.push_back(b0);
        vertices.push_back(t0);
        vertices.push_back(t1);

        // 三角形 2
        vertices.push_back(b0);
        vertices.push_back(t1);
        vertices.push_back(b1);
    }

    return vertices;
}

bool exportToOBJ(const std::string& filename, const std::vector<Vertex>& vertices)
{
    std::ofstream out(filename);
    if (!out.is_open()) {
        std::cerr << "Failed to open " << filename << " for writing.\n";
        return false;
    }

    out << "# Exported from DXF Extrude Viewer\n";

    // 写顶点
    for (auto& v : vertices)
        out << "v " << v.x << " " << v.y << " " << v.z << "\n";

    // 写面（三个顶点一个面）
    for (size_t i = 0; i < vertices.size(); i += 3)
        out << "f " << i + 1 << " " << i + 2 << " " << i + 3 << "\n";

    out.close();
    std::cout << "OBJ exported: " << filename << std::endl;
    return true;
}
