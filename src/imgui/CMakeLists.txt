# Simple CMakeLists for imgui
cmake_minimum_required(VERSION 3.18)

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

target_compile_options(
    imgui
    PRIVATE
    $<$<AND:${gcc_like_cxx},$<CONFIG:Release>>:-O3>
)

target_link_libraries(
    imgui
    PRIVATE
    project_compile_flags
)

target_include_directories(
    imgui
    PRIVATE
    ${glfw3_INCLUDE_DIRS}
)