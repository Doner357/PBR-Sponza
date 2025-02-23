# Simple CMakeLists for imgui
cmake_minimum_required(VERSION 3.15)

add_library(
    imgui OBJECT
    imgui_demo.cpp
    imgui_draw.cpp
    imgui_impl_glfw.cpp
    imgui_impl_opengl3.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
    imgui.cpp
)

target_link_libraries(
    imgui
    PRIVATE
    project_compile_flags
)

target_include_directories(
    imgui
    PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../../include"
)