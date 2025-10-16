#pragma once
#include "libdxfrw.h"
#include "utils.h"
// 继承 DRW_Interface，用于接收解析到的图元



class MyDXFReader : public DRW_Interface {
public:
    float _height; // 拉伸高度
    int poly_count,circle_count;
    std::string _obj_save_path;
    std::vector<Shape> shapes; // 存储所有拉伸后的形状

    void addCircle(const DRW_Circle& data) override
    {
        std::cout << "Circle: center("
                  << data.basePoint.x << ", " << data.basePoint.y
                  << "), radius=" << data.radious << "\n";

        auto pts = makeCircle(data.basePoint.x, data.basePoint.y, data.radious, 64);
        Shape s(ShapeType::Circle, generateExtrudedMesh(pts, _height));
        exportToOBJ(_obj_save_path + "/circle_" + std::to_string(circle_count++) + ".obj", s.vertices);
        shapes.push_back(std::move(s));
    }

    void addLWPolyline(const DRW_LWPolyline& data) override
    {
        std::cout << "LWPolyline: " << data.vertlist.size() << " vertices\n";
        std::vector<Vertex> points;
        for (const auto& v : data.vertlist) {
            std::cout << "   (" << v->x << ", " << v->y << ")\n";
            
            points.push_back({ static_cast<float>(v->x), static_cast<float>(v->y), 0.0 });
        }
        Shape s(ShapeType::Polyline, generateExtrudedMesh(points, _height));
        exportToOBJ(_obj_save_path + "/polyline_" + std::to_string(poly_count++) + ".obj", s.vertices);
        shapes.push_back(std::move(s));
    }

    MyDXFReader(float height,std::string path)
        : _height(height)
        , _obj_save_path(path)
        , poly_count(0)
        , circle_count(0)
    {
    }












    // 其他接口：空实现即可（不关心）-------------------------------------------------------------
    void addLine(const DRW_Line& data) override { }
    void addArc(const DRW_Arc& data) override { }
    void addPoint(const DRW_Point& data) override { }
    void addHeader(const DRW_Header*) override { }
    void addRay(const DRW_Ray&) override { }
    void addXline(const DRW_Xline&) override { }
    void addSolid(const DRW_Solid&) override { }
    void addMText(const DRW_MText&) override { }
    void addText(const DRW_Text&) override { }
    void addDimAlign(const DRW_DimAligned*) override { }
    void addDimLinear(const DRW_DimLinear*) override { }
    void addDimRadial(const DRW_DimRadial*) override { }
    void addDimDiametric(const DRW_DimDiametric*) override { }
    void addDimAngular(const DRW_DimAngular*) override { }
    void addDimAngular3P(const DRW_DimAngular3p*) override { }
    void addDimOrdinate(const DRW_DimOrdinate*) override { }
    void addLeader(const DRW_Leader*) override { }
    void addHatch(const DRW_Hatch*) override { }
    void addViewport(const DRW_Viewport&) override { }
    void addImage(const DRW_Image*) override { }
    void addInsert(const DRW_Insert&) override { }
    void add3dFace(const DRW_3Dface&) override { }
    void addPolyline(const DRW_Polyline&) override { }
    void addBlock(const DRW_Block&) override { }
    void endBlock() override { }
    void setBlock(const int) override { }
    void addDimStyle(const DRW_Dimstyle&) override { }
    void addAppId(const DRW_AppId&) override { }
    void addVport(const DRW_Vport&) override { }
    void addLayer(const DRW_Layer&) override { }
    void addLType(const DRW_LType&) override { }
    void addTextStyle(const DRW_Textstyle&) override { }
    void addEllipse(const DRW_Ellipse& data) override { }
    void addSpline(const DRW_Spline*) override { }
    void addKnot(const DRW_Entity& data) override { }
    void addTrace(const DRW_Trace&) override { }
    void linkImage(const DRW_ImageDef* data) override { }
    void addPlotSettings(const DRW_PlotSettings*) override { }
    void writeHeader(DRW_Header&) override { }
    void writeBlocks() override { }
    void writeBlockRecords() override { }
    void writeEntities() override { }
    void writeLTypes() override { }
    void writeLayers() override { }
    void writeTextstyles() override { }
    void writeVports() override { }
    void writeDimstyles() override { }
    void writeObjects() override { }
    void writeAppId() override { }

    void addComment(const char*) override { }
};
