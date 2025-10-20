cmake_minimum_required(VERSION 3.15)
project(CADProcessor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ------------------ 依赖管理 ------------------
include(FetchContent)

# 先检查系统里是否已经有 dxfrw
find_package(dxfrw QUIET)


FetchContent_GetProperties(libdxfrw)
if (NOT dxfrw_FOUND)
    # ------------------ 获取 libdxfrw ------------------
    FetchContent_Declare(
        libdxfrw
        GIT_REPOSITORY https://github.com/LibreCAD/libdxfrw.git
        GIT_TAG master
    )

    FetchContent_GetProperties(libdxfrw)
    if(NOT libdxfrw_POPULATED)
        message(STATUS "Fetching libdxfrw from GitHub...")
        FetchContent_Populate(libdxfrw)
        add_subdirectory(${libdxfrw_SOURCE_DIR} ${libdxfrw_BINARY_DIR})
    endif()
endif()

# ------------------ 源文件 ------------------
set(SOURCES
    src/main.cpp
    src/utils.cpp
)

set(HEADERS
    include/utils.h
    include/MyDxf_reader.hpp
)

# ------------------ 生成可执行文件 ------------------
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# ------------------ 链接库 ------------------
if (TARGET dxfrw)
    target_link_libraries(${PROJECT_NAME} PRIVATE dxfrw)
else()
    # 如果 FetchContent_MakeAvailable 生成的是 dxfrwd（静态库）
    target_link_libraries(${PROJECT_NAME} PRIVATE dxfrwd)
endif()

# ------------------ 包含目录 ------------------
target_include_directories(${PROJECT_NAME}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${libdxfrw_SOURCE_DIR}/src
)

