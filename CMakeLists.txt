cmake_minimum_required(VERSION 3.15)
project(CADProcessor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ------------------ 编译选项 ------------------
if(MSVC)
    add_definitions(-DUNICODE -D_UNICODE)
    add_compile_options(/utf-8)
endif()
add_link_options(/IGNORE:4099)

# ------------------ 包含目录 ------------------
include_directories(
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "E:/3dpart"
    "E:/3dpart/OpenGL/include"
    "E:/3dpart/libdxfrw/include"
)

# ------------------ 库目录 ------------------
link_directories(
    "E:/3dpart/OpenGL/lib"
    "E:/3dpart/libdxfrw/lib"
)

# ------------------ 源文件 ------------------
set(SOURCES
    "src/main.cpp"
    "src/utils.cpp"
    "include/glad.c"
    "include/MyDxf_reader.hpp"
)

set(HEADERS
    "include/utils.h"
    "include/shader_s.h"
)

# ------------------ 生成可执行文件 ------------------
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# ------------------ 链接库 ------------------
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    dxfrwd
    glfw3
    opengl32
)

# ------------------ 调试与运行时文件复制 ------------------
foreach(config IN ITEMS Debug)
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${config}/data)
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${config}/shaders)
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${config}/obj_res)

    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data/sample.dxf
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/${config}/data)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders/fragment_shader.glsl
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/${config}/shaders)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders/vertex_shader.glsl
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/${config}/shaders)
endforeach()

# ------------------ Debug / Release 设置 ------------------
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}/Debug/bin
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR}/Release/bin
)
