#pragma once
#include "libdxfrw.h"
#include "utils.h"
// 继承 DRW_Interface，用于接收解析到的图元

class MyDXFReader : public DRW_Interface
{
public:
    float defaultHeight = 10.0f; // 拉伸高度
    int poly_count, circle_count;
    std::string _obj_save_path;
    std::vector<RawPoly> polys;

    MyDXFReader(float height, std::string path)
        : defaultHeight(height), _obj_save_path(path), poly_count(0), circle_count(0)
    {
    }

    void addCircle(const DRW_Circle &data) override
    {
        std::cout << "Circle: center("
                  << data.basePoint.x << ", " << data.basePoint.y
                  << "), radius=" << data.radious << "\n";

        const int segments = 64;
        RawPoly p;
        p.area = 0;
        for (int i = 0; i < segments; i++)
        {
            double theta = 2.0 * M_PI * i / segments;
            Vertex V{(float)(data.basePoint.x + cos(theta) * data.radious),
                     (float)(data.basePoint.y + sin(theta) * data.radious),
                     0.0f};
            p.pts.push_back(V);
        }
        p.area = polygonSignedArea(p.pts);
        polys.push_back(std::move(p));
    }

    void addLWPolyline(const DRW_LWPolyline &data) override
    {
        if (data.vertlist.empty())
            return;
        std::cout << "LWPolyline: " << data.vertlist.size() << " vertices\n";

        RawPoly p;
        p.area = 0;
        for (const auto &v : data.vertlist)
        {
            std::cout << "   (" << v->x << ", " << v->y << ")\n";
            Vertex V({(float)v->x, (float)v->y, 0.0f});
            p.pts.push_back(V);
        }
        p.area = polygonSignedArea(p.pts);
        // if poly closed? sometimes last equals first, remove duplicate last if present
        if (p.pts.size() > 1)
        {
            if (std::fabs(p.pts.front().x - p.pts.back().x) < 1e-6f &&
                std::fabs(p.pts.front().y - p.pts.back().y) < 1e-6f)
            {
                p.pts.pop_back();
            }
        }
        polys.push_back(std::move(p));
    }

    std::vector<std::pair<RawPoly, std::vector<RawPoly>>> groupOuterWithHoles()
    {
        size_t m = polys.size();
        std::vector<int> parent(m, -1); // 父级多边形索引（即它所属的外环）
        // 计算多边形的面积，因为计算方法的限制，面积可能为负数（取决于顶点顺序），所以取绝对值
        std::vector<double> absArea(m);
        for (size_t i = 0; i < m; i++)
            absArea[i] = std::abs(polys[i].area);

        // 对每个多边形 j，取它的第一个顶点作为测试点
        // 寻找包含该点的面积最小多边形 i，作为它的父级
        for (size_t j = 0; j < m; j++)
        {
            // pick a test point from polys[j], e.g. first vertex
            if (polys[j].pts.empty())
                continue;
            double tx = polys[j].pts[0].x;
            double ty = polys[j].pts[0].y;
            int best = -1;
            double bestArea = 1e300;
            for (size_t i = 0; i < m; i++)
            {
                if (i == j)
                    continue; // 不跟自己比较
                if (absArea[i] <= absArea[j])
                    continue; // 外环面积必须更大
                if (pointInPoly(polys[i].pts, tx, ty))
                {
                    if (absArea[i] < bestArea)
                    {
                        bestArea = absArea[i];
                        best = (int)i;
                    }
                }
            }
            parent[j] = best; // -1 means no parent -> it's an outer candidate
        }

        // 创建一个新的 group（pair），外环放在 first，空洞初始化为空 second
        std::vector<std::pair<RawPoly, std::vector<RawPoly>>> groups;
        std::vector<int> outerIndexMap(m, -1);
        for (size_t i = 0; i < m; i++)
        {
            if (parent[i] == -1)
            { // outer
                outerIndexMap[i] = (int)groups.size();
                groups.push_back({polys[i], {}});
            }
        }
        // assign holes
        for (size_t j = 0; j < m; j++)
        {
            if (parent[j] != -1)
            {
                int p = parent[j];
                int idx = outerIndexMap[p];
                if (idx >= 0)
                    groups[idx].second.push_back(polys[j]);
            }
        }
        return groups;
    }

    // 其他接口：空实现即可（不关心）-------------------------------------------------------------
    void addLine(const DRW_Line &data) override {}
    void addArc(const DRW_Arc &data) override {}
    void addPoint(const DRW_Point &data) override {}
    void addHeader(const DRW_Header *) override {}
    void addRay(const DRW_Ray &) override {}
    void addXline(const DRW_Xline &) override {}
    void addSolid(const DRW_Solid &) override {}
    void addMText(const DRW_MText &) override {}
    void addText(const DRW_Text &) override {}
    void addDimAlign(const DRW_DimAligned *) override {}
    void addDimLinear(const DRW_DimLinear *) override {}
    void addDimRadial(const DRW_DimRadial *) override {}
    void addDimDiametric(const DRW_DimDiametric *) override {}
    void addDimAngular(const DRW_DimAngular *) override {}
    void addDimAngular3P(const DRW_DimAngular3p *) override {}
    void addDimOrdinate(const DRW_DimOrdinate *) override {}
    void addLeader(const DRW_Leader *) override {}
    void addHatch(const DRW_Hatch *) override {}
    void addViewport(const DRW_Viewport &) override {}
    void addImage(const DRW_Image *) override {}
    void addInsert(const DRW_Insert &) override {}
    void add3dFace(const DRW_3Dface &) override {}
    void addPolyline(const DRW_Polyline &) override {}
    void addBlock(const DRW_Block &) override {}
    void endBlock() override {}
    void setBlock(const int) override {}
    void addDimStyle(const DRW_Dimstyle &) override {}
    void addAppId(const DRW_AppId &) override {}
    void addVport(const DRW_Vport &) override {}
    void addLayer(const DRW_Layer &) override {}
    void addLType(const DRW_LType &) override {}
    void addTextStyle(const DRW_Textstyle &) override {}
    void addEllipse(const DRW_Ellipse &data) override {}
    void addSpline(const DRW_Spline *) override {}
    void addKnot(const DRW_Entity &data) override {}
    void addTrace(const DRW_Trace &) override {}
    void linkImage(const DRW_ImageDef *data) override {}
    void addPlotSettings(const DRW_PlotSettings *) override {}
    void writeHeader(DRW_Header &) override {}
    void writeBlocks() override {}
    void writeBlockRecords() override {}
    void writeEntities() override {}
    void writeLTypes() override {}
    void writeLayers() override {}
    void writeTextstyles() override {}
    void writeVports() override {}
    void writeDimstyles() override {}
    void writeObjects() override {}
    void writeAppId() override {}

    void addComment(const char *) override {}
};
