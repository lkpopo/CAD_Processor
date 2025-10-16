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
    if (n < 3)
        return vertices; // 至少 3 个顶点才能成面

    // 1️⃣ 顶面顶点
    std::vector<Vertex> topVerts;
    for (auto& b : bottomVerts)
        topVerts.push_back({ b.x, b.y, b.z + height });

    // 2️⃣ 生成侧面三角形
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

    // 3️⃣ 生成底面和顶面三角形（triangle fan）
    Vertex centerB { 0.0f, 0.0f, 0.0f }, centerT { 0.0f, 0.0f, height };
    for (auto& v : bottomVerts) {
        centerB.x += v.x;
        centerB.y += v.y;
        centerB.z += v.z;
    }
    centerB.x /= n;
    centerB.y /= n;
    centerB.z /= n;
    centerT.x = centerB.x;
    centerT.y = centerB.y;
    centerT.z = centerB.z + height;

    for (size_t i = 0; i < n; ++i) {
        size_t next = (i + 1) % n;
        // 底面
        vertices.push_back(bottomVerts[i]);
        vertices.push_back(bottomVerts[next]);
        vertices.push_back(centerB);

        // 顶面
        vertices.push_back(topVerts[i]);
        vertices.push_back(centerT);
        vertices.push_back(topVerts[next]);
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
