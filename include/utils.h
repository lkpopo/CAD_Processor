#pragma once
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>
#include "earcut.hpp"

struct Vertex
{
    float x, y, z;
    Vertex(float _x, float _y, float _z)
        : x(_x), y(_y), z(_z)
    {
    }
};

struct RawPoly
{
    std::vector<Vertex> pts; // 2D in x,y (z usually 0)
    double area;             // signed area (abs for magnitude)
};

struct Face
{
    int a, b, c;
};

bool pointInPoly(const std::vector<Vertex> &poly, double x, double y);
double polygonSignedArea(const std::vector<Vertex> &pts);
std::vector<Vertex> triangulateRingsToTris(const std::vector<std::vector<Vertex>> &polygonRings, float zTop, float zBottom);
std::vector<Vertex> generateSideTriangles(const std::vector<Vertex> &ring, float height);
void appendVerts(std::vector<Vertex> &dst, const std::vector<Vertex> &src);
void exportGroupToOBJ(const std::vector<Vertex> &verts, size_t index);