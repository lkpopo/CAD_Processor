#pragma once
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <vector>
#include <fstream>

struct Vertex {
    float x, y, z;
    Vertex(float _x, float _y, float _z)
        : x(_x)
        , y(_y)
        , z(_z)
    {
    }
};
enum class ShapeType {
    Circle,
    Polyline
};

struct Shape {
    ShapeType type;
    std::vector<Vertex> vertices; 
    Shape(ShapeType t, const std::vector<Vertex>& verts)
        : type(t)
        , vertices(verts)
    {
    }
};

struct Face {
    int a, b, c;
};

std::vector<Vertex> makeCircle(double cx, double cy, double r, int segments);
std::vector<Vertex> generateExtrudedMesh(const std::vector<Vertex>& bottomVerts, float height);
bool exportToOBJ(const std::string& filename, const std::vector<Vertex>& vertices);