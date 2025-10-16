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
    std::string filename = "sample.dxf";
    MyDXFReader reader(100.0, "F:/workspace/CAD_Processor/obj_res");
    dxfRW dxf(filename.c_str()); // 创建 DXF 读取对象

    std::cout << "Reading file: " << filename << std::endl;

    if (!dxf.read(&reader, false)) { // false 表示不保留块引用
        std::cerr << "Failed to read file.\n";
        return 1;
    }

     // ------------------ 归一化 DXF 顶点 ------------------
    float x_min = 1e4, x_max = -1e4;
    float y_min = 1e4, y_max = -1e4;
    float z_min = 1e4, z_max = -1e4;

    for (auto& shape : reader.shapes)
        for (auto& v : shape.vertices) {
            x_min = std::min(x_min, v.x);
            x_max = std::max(x_max, v.x);
            y_min = std::min(y_min, v.y);
            y_max = std::max(y_max, v.y);
            z_min = std::min(z_min, v.z);
            z_max = std::max(z_max, v.z);
        }

    float scale = 2.0f / std::max({ x_max - x_min, y_max - y_min, z_max - z_min });
    glm::vec3 center((x_max + x_min) / 2, (y_max + y_min) / 2, (z_max + z_min) / 2);

    for (auto& shape : reader.shapes)
        for (auto& v : shape.vertices) {
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

    Shader shader("vertex_shader.glsl", "fragment_shader.glsl");

    // ------------------ 创建 VAO/VBO ------------------
    std::vector<GLuint> vaos, vbos;
    for (auto& shape : reader.shapes) {
        GLuint vao, vbo;
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, shape.vertices.size() * sizeof(Vertex),
            shape.vertices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(0);

        vaos.push_back(vao);
        vbos.push_back(vbo);
    }

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
        for (size_t i = 0; i < reader.shapes.size(); ++i) {
            const auto& shape = reader.shapes[i];
            glBindVertexArray(vaos[i]);

            // 画底面
            glDrawArrays(GL_TRIANGLES, 0, shape.vertices.size());

        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
    
}
