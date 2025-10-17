#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <shader_s.h>
#include <shader_s.h>
#include "MyDxf_reader.hpp"

// ------------------ 矩阵生成简化 ------------------
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "utils.h"

// ------------------ 键盘交互 ------------------
float rotY = 0.0f;
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        rotY -= 0.02f;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        rotY += 0.02f;
}

int main(int argc, char** argv)
{
    std::string filename = "../data/sample.dxf";
    MyDXFReader reader(100.0, "../obj_res");
    dxfRW dxf(filename.c_str()); // 创建 DXF 读取对象

    std::cout << "Reading file: " << filename << std::endl;

    if (!dxf.read(&reader, false)) { // false 表示不保留块引用
        std::cerr << "Failed to read file.\n";
        return 1;
    }
    std::cout<<"Parsed polygons: "<<reader.polys.size()<<"\n";

    auto groups = reader.groupOuterWithHoles();
    std::cout<<"Groups (outer with holes): "<<groups.size()<<"\n";

     // For each group, build polygonRings (outer then holes), extrude and triangulate (earcut)
    const float height = reader.defaultHeight;
    std::vector<Vertex> allTris; // all triangles for export / rendering
    size_t groupIdx = 0;
    for (auto &g : groups) {
        std::vector<std::vector<Vertex>> rings;
        // outer ring: ensure proper winding: earcut accepts either, but typical is outer CCW, holes CW (earcut can handle)
        rings.push_back(g.first.pts);
        for (auto &hole : g.second) rings.push_back(hole.pts);

        // 利用earcut生成三角网格
        auto tris = triangulateRingsToTris(rings, height, 0.0f);
        appendVerts(allTris, tris);
        // sides: for each ring generate side tris
        for (auto &ring : rings) {
            auto side = generateSideTriangles(ring, height);
            appendVerts(allTris, side);
            appendVerts(tris, side);
        }

        // 导出OBJ
         exportGroupToOBJ(tris, groupIdx++);
    }

    if (allTris.empty()) {
        std::cerr<<"No triangles generated, nothing to render.\n";
        return -1;
    }

    // normalize coords to [-1,1]
    float xmin=1e9, xmax=-1e9, ymin=1e9, ymax=-1e9, zmin=1e9, zmax=-1e9;
    for (auto &v: allTris) {
        xmin = std::min(xmin, v.x); xmax = std::max(xmax, v.x);
        ymin = std::min(ymin, v.y); ymax = std::max(ymax, v.y);
        zmin = std::min(zmin, v.z); zmax = std::max(zmax, v.z);
    }
    float scale = 2.0f / std::max({xmax - xmin, ymax - ymin, zmax - zmin, 1e-6f});
    glm::vec3 center{ (xmax+xmin)/2.0f, (ymax+ymin)/2.0f, (zmax+zmin)/2.0f };
    for (auto &v: allTris) {
        v.x = (v.x - center.x) * scale;
        v.y = (v.y - center.y) * scale;
        v.z = (v.z - center.z) * scale;
    }

    std::cout << "DXF parsing finished.\n";

    // ------------------ 初始化 OpenGL ------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "DXF Extrude Viewer", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    Shader shader("../shaders/vertex_shader.glsl", "../shaders/fragment_shader.glsl");
        
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, allTris.size()*sizeof(Vertex), allTris.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)0);

     // ------------------ 渲染循环 ------------------
    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        glClearColor(0.1, 0.1, 0.1, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();

        glm::mat4 model = glm::rotate(glm::mat4(1.0f), rotY, glm::vec3(0, 1, 0));
        glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 3), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.f / 600.f, 0.1f, 10.0f);
        shader.setMat4("model", model);
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);
        
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)allTris.size());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
    
}
