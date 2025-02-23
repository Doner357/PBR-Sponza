cmake_minimum_required(VERSION 3.18)

project(OpenGL_Course_Project VERSION 1.0.0)

set(CMAKE_DEBUG_POSTFIX d)

add_library(project_compile_flags INTERFACE)
target_compile_features(project_compile_flags INTERFACE cxx_std_20)

set(gcc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU,LCC>")
set(msvc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,MSVC>")
target_compile_options(
    project_compile_flags INTERFACE
    $<${gcc_like_cxx}:$<BUILD_INTERFACE:-Wall;-Wextra;-Wshadow;-Wformat=2;-Wunused>>
    $<${msvc_like_cxx}:$<BUILD_INTERFACE:-W3>>
)


# For asset files
file(GLOB ASSET_FILES
    "${CMAKE_SOURCE_DIR}/assets/*"
    "${CMAKE_SOURCE_DIR}/assets/**/*"
)
add_custom_command(
  OUTPUT "${CMAKE_BINARY_DIR}/.copied_dir"
  DEPENDS ${ASSET_FILES}
  COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_BINARY_DIR}/bin"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/bin"
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${CMAKE_SOURCE_DIR}/assets"
          "${CMAKE_BINARY_DIR}/bin/assets"
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${CMAKE_BINARY_DIR}/bin/assets"
          "${CMAKE_BINARY_DIR}/bin"
  COMMAND ${CMAKE_COMMAND} -E remove_directory
          "${CMAKE_BINARY_DIR}/bin/assets"
  COMMAND ${CMAKE_COMMAND} -E touch
          "${CMAKE_BINARY_DIR}/.copied_dir"
  COMMENT "Copy assets/* to out/ (build-time) skipping top-level directory"
)

add_custom_target(
    copy_assets ALL
    DEPENDS "${CMAKE_BINARY_DIR}/.copied_dir"
)

# For appended library
add_library(
    appended OBJECT
    "${CMAKE_CURRENT_SOURCE_DIR}/src/glad.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/stb_image_define.cpp"
)

target_include_directories(
    appended
    PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(
    appended
    PRIVATE
    project_compile_flags
)

# For imgui
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/src/imgui")

# For main program
add_executable(
    ${CMAKE_PROJECT_NAME}                         # Project Name
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Source.cpp"  # Source Files
    "${CMAKE_CURRENT_SOURCE_DIR}/src/learnopengl/custom_helper.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/learnopengl/ddsloader.cpp"
)

target_include_directories(
    ${CMAKE_PROJECT_NAME}
    PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

find_library(
    ASSIMP_LIB
    NAMES libassimp.dll assimp.dll
    HINTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/assimp"
    REQUIRED
)

find_library(
    GLFW_LIB
    NAMES libglfw3 glfw3
    HINTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/glfw"
    REQUIRED
)

target_link_libraries(
    ${CMAKE_PROJECT_NAME}
    PRIVATE
    project_compile_flags
    appended
    imgui
    ${ASSIMP_LIB}
    ${GLFW_LIB}
)

set_target_properties(
    ${CMAKE_PROJECT_NAME}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY         "${CMAKE_CURRENT_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_Debug   "${CMAKE_CURRENT_BINARY_DIR}/bin/Debug"
    RUNTIME_OUTPUT_DIRECTORY_Release "${CMAKE_CURRENT_BINARY_DIR}/bin/Release"
)