#include "MyDxf_reader.hpp"
#include "utils.h"

// ------------------ 键盘交互 ------------------
float rotY = 0.0f;

int main(int argc, char **argv)
{
    std::string filename = "../data/sample.dxf";
    MyDXFReader reader(100.0, "../obj_res");
    dxfRW dxf(filename.c_str()); // 创建 DXF 读取对象

    std::cout << "Reading file: " << filename << std::endl;

    if (!dxf.read(&reader, false))
    { // false 表示不保留块引用
        std::cerr << "Failed to read file.\n";
        return 1;
    }
    std::cout << "Parsed polygons: " << reader.polys.size() << "\n";

    auto groups = reader.groupOuterWithHoles();
    std::cout << "Groups (outer with holes): " << groups.size() << "\n";

    // For each group, build polygonRings (outer then holes), extrude and triangulate (earcut)
    const float height = reader.defaultHeight;
    std::vector<Vertex> allTris; // all triangles for export / rendering
    size_t groupIdx = 0;
    for (auto &g : groups)
    {
        std::vector<std::vector<Vertex>> rings;
        
        // 先添加外环，如果有对应的内环再添加内环
        rings.push_back(g.first.pts);
        for (auto &hole : g.second)
            rings.push_back(hole.pts);

        // 利用earcut生成上下面的三角网格
        auto tris = triangulateRingsToTris(rings, height, 0.0f);
        // appendVerts(allTris, tris);
        // 生成侧面三角形
        for (auto &ring : rings)
        {
            auto side = generateSideTriangles(ring, height);
            // appendVerts(allTris, side);
            appendVerts(tris, side);
        }

        // 导出OBJ
        exportGroupToOBJ(tris, groupIdx++);
    }

    std::cout << "DXF parsing finished.\n";

    return 0;
}
