#include "utils.h"

// 利用二维 Green 定理的离散化计算多边形有向面积
double polygonSignedArea(const std::vector<Vertex> &pts)
{
    double a = 0;
    size_t n = pts.size();
    for (size_t i = 0; i < n; i++)
    {
        size_t j = (i + 1) % n;
        a += (double)pts[i].x * (double)pts[j].y - (double)pts[j].x * (double)pts[i].y;
    }
    return 0.5 * a;
}

// 射线法判断点是否在多边形内
bool pointInPoly(const std::vector<Vertex> &poly, double x, double y)
{
    bool inside = false;
    size_t n = poly.size();
    for (size_t i = 0, j = n - 1; i < n; j = i++)
    {
        double xi = poly[i].x, yi = poly[i].y;
        double xj = poly[j].x, yj = poly[j].y;
        // &&前面的表达式判断点知否在这条边的上下之间
        // 后面的表达式表示多边形边与水平线Y的交点在X轴上的位置x'，因为向右射线，所以是 x < 交点的 x' 坐标
        // 其中1e-18避免浮点数除以0，(边平行于射线的时候)
        bool intersect = ((yi > y) != (yj > y)) &&
                         (x < (xj - xi) * (y - yi) / (yj - yi + 1e-18) + xi);
        if (intersect)
            inside = !inside;
    }
    return inside;
}

// 把 2D 多边形“拉升成立体柱体”，并生成底面和顶面的三角形
std::vector<Vertex> triangulateRingsToTris(const std::vector<std::vector<Vertex>> &polygonRings, float zTop, float zBottom)
{
    // prepare earcut input
    using Point = std::pair<float, float>;
    std::vector<std::vector<Point>> data;
    for (auto &ring : polygonRings)
    {
        std::vector<Point> r;
        for (auto &p : ring)
            r.push_back({p.x, p.y});
        data.push_back(std::move(r));
    }

    std::vector<uint32_t> idx = mapbox::earcut<uint32_t>(data);

    // Flatten vertex list: earcut indices reference flattened list of rings concatenated in order
    std::vector<Vertex> flat;
    for (auto &ring : polygonRings)
        for (auto &v : ring)
            flat.push_back({v.x, v.y, (float)zBottom});

    // bottom triangles from earcut (assume earcut gives CCW for outer)
    std::vector<Vertex> tris;
    for (size_t i = 0; i < idx.size(); i += 3)
    {
        Vertex a = flat[idx[i]];
        Vertex b = flat[idx[i + 1]];
        Vertex c = flat[idx[i + 2]];
        tris.push_back(a);
        tris.push_back(b);
        tris.push_back(c);
    }
    // top triangles: same indices but with z=zTop and reversed to flip normal outward
    // we need a mapping from flat index to top coordinate
    std::vector<Vertex> flatTop = flat;
    for (auto &v : flatTop)
        v.z = (float)zTop;
    for (size_t i = 0; i < idx.size(); i += 3)
    {
        // reverse order for top
        Vertex a = flatTop[idx[i]];
        Vertex b = flatTop[idx[i + 2]];
        Vertex c = flatTop[idx[i + 1]];
        tris.push_back(a);
        tris.push_back(b);
        tris.push_back(c);
    }
    return tris;
}

// 生成侧面三角形
std::vector<Vertex> generateSideTriangles(const std::vector<Vertex> &ring, float height)
{
    std::vector<Vertex> side;
    size_t n = ring.size();
    if (n < 2)
        return side;
    for (size_t i = 0; i < n; i++)
    {
        size_t j = (i + 1) % n;
        Vertex b0 = ring[i];
        Vertex b1 = ring[j];
        Vertex t0 = {b0.x, b0.y, b0.z + height};
        Vertex t1 = {b1.x, b1.y, b1.z + height};
        // tri 1: b0, t0, t1
        side.push_back(b0);
        side.push_back(t0);
        side.push_back(t1);
        // tri 2: b0, t1, b1
        side.push_back(b0);
        side.push_back(t1);
        side.push_back(b1);
    }
    return side;
}

void appendVerts(std::vector<Vertex> &dst, const std::vector<Vertex> &src)
{
    dst.insert(dst.end(), src.begin(), src.end());
}

void exportGroupToOBJ(const std::vector<Vertex> &verts, size_t index)
{
    std::ostringstream fname;
    fname << "shape_" << std::setw(3) << std::setfill('0') << index << ".obj";
    std::ofstream out(fname.str());
    if (!out.is_open())
    {
        std::cerr << "Failed to open " << fname.str() << " for writing.\n";
        return;
    }

    // 写入顶点
    for (const auto &v : verts)
    {
        out << "v " << v.x << " " << v.y << " " << v.z << "\n";
    }

    // 写入面（每3个顶点为一个三角形）
    for (size_t i = 0; i + 2 < verts.size(); i += 3)
    {
        out << "f " << i + 1 << " " << i + 2 << " " << i + 3 << "\n";
    }

    out.close();
    std::cout << "Exported " << fname.str() << " (" << verts.size() / 3 << " triangles)\n";
}