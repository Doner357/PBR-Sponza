cmake_minimum_required(VERSION 3.27)

##
# @brief Main project configuration for the OpenGL_Course_Project.
#        Uses separate interface libraries for compile/link flags and
#        optionally applies static linking for system libraries on both
#        GCC/Clang and MSVC compilers.
#
project(OpenGL_Course_Project VERSION 1.0.0)

##
# @brief Appends a debug postfix (for example, "xxxd.exe" in Debug mode).
#
set(CMAKE_DEBUG_POSTFIX d)

##
# @brief Option to link system libraries (runtime) statically or dynamically.
#        If set to ON, static linking will be used for both MSVC and GCC/Clang.
#
option(STATIC_SYSTEM_LINK "Link system libraries (runtime) statically" ON)

##
# @brief Separate interface libraries to manage compile-time and link-time flags.
#
add_library(project_compile_flags INTERFACE)
add_library(project_link_flags    INTERFACE)

##
# @brief Enforce C++20 features for all targets that link against project_compile_flags.
#
target_compile_features(project_compile_flags INTERFACE cxx_std_20)

##
# @brief Generator expressions to detect the compiler family for warning flags, etc.
#        gcc_like_cxx = ARMClang, AppleClang, Clang, GNU, LCC
#        msvc_like_cxx = MSVC
#
set(gcc_like_cxx  "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU,LCC>")
set(msvc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,MSVC>")

##
# @brief Additional expression to check for "real" GNU (GCC) only.
#        We only apply -static-libgcc/-static-libstdc++ under actual GCC, not Clang.
#
set(real_gcc "$<STREQUAL:$<CXX_COMPILER_ID>,GNU>")


##
# @brief Common compile options:
#        - For GCC/Clang: add common warning flags (-Wall, -Wextra, etc.).
#        - For MSVC: add default warning level (/W3).
#        - For real GCC only: -static-libgcc, -static-libstdc++.
#        - For Clang only: -static-libc++ -static-libgcc (if using libc++).
#        - For MSVC static runtime, /MT or /MTd must be specified at compile time.
#
target_compile_options(project_compile_flags
    INTERFACE
    # GCC/Clang warning flags
    $<$<AND:${gcc_like_cxx},$<CONFIG:Debug>>:$<BUILD_INTERFACE:-Wall -Wextra -Wshadow -Wformat=2 -Wunused>>
    $<$<AND:${gcc_like_cxx},$<CONFIG:Release>>:$<BUILD_INTERFACE:-O3>>
    
    # Example optimization for Release builds under GCC/Clang
    # $<$<AND:${gcc_like_cxx},$<CONFIG:Release>>:-O2>
    
    # MSVC warning flags
    $<$<AND:${msvc_like_cxx}>:$<BUILD_INTERFACE:-W3>>

    # Only in real GCC + STATIC_SYSTEM_LINK=ON do we add -static-libgcc / -static-libstdc++
    $<$<AND:${real_gcc},$<BOOL:${STATIC_SYSTEM_LINK}>>:-static-libgcc -static-libstdc++>
    
    # MSVC static runtime for Debug/Release
    $<$<AND:${msvc_like_cxx},$<BOOL:${STATIC_SYSTEM_LINK}>,$<CONFIG:Debug>>:/MTd>
    $<$<AND:${msvc_like_cxx},$<BOOL:${STATIC_SYSTEM_LINK}>,$<CONFIG:Release>>:/MT>
)

target_compile_definitions(project_compile_flags
    INTERFACE
    $<$<AND:${gcc_like_cxx},$<CONFIG:Debug>>:"NDEBUG">
)

##
# @brief Common link options:
#        - For GCC/Clang: use -static for fully static linking.
#        - For Clang, also apply -static if desired.
#        - For MSVC: /MT or /MTd at link stage, consistent with compile stage.
#
target_link_options(project_link_flags
    INTERFACE
    # GCC/Clang static linking (Clang generally accepts -static, so no warning here)
    $<$<AND:${gcc_like_cxx},$<BOOL:${STATIC_SYSTEM_LINK}>>:-static>

    # MSVC static linking for Debug/Release
    $<$<AND:${msvc_like_cxx},$<BOOL:${STATIC_SYSTEM_LINK}>,$<CONFIG:Debug>>:/MTd>
    $<$<AND:${msvc_like_cxx},$<BOOL:${STATIC_SYSTEM_LINK}>,$<CONFIG:Release>>:/MT>
)

##
# @brief Collect and copy asset files into the output directory.
#
file(GLOB ASSET_FILES
    "${CMAKE_SOURCE_DIR}/assets/*"
    "${CMAKE_SOURCE_DIR}/assets/**/*"
)

##
# @brief Custom command to remove old bin folder, create a new one, copy all assets,
#        and then move them up one level (removing the intermediate folder).
#
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

##
# @brief Custom target to ensure the copy_assets step runs at build time.
#
add_custom_target(
    copy_assets ALL
    DEPENDS "${CMAKE_BINARY_DIR}/.copied_dir"
)

##
# @brief Create an OBJECT library for appended sources like glad and stb_image_define.
#
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

##
# @brief Link both compile-time flags and link-time flags into the appended library.
#
target_link_libraries(
    appended
    PRIVATE
    project_compile_flags
    project_link_flags
)

##
# @brief Add subdirectory for imgui sources.
#
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/src/imgui")

##
# @brief Main application executable.
#
add_executable(
    ${CMAKE_PROJECT_NAME}
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Source.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/learnopengl/custom_helper.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/learnopengl/ddsloader.cpp"
)

target_include_directories(
    ${CMAKE_PROJECT_NAME}
    PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

##
# @brief Locate ASSIMP and GLFW libraries.
#
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

##
# @brief Link libraries and interface flags to the main project executable.
#
target_link_libraries(
    ${CMAKE_PROJECT_NAME}
    PRIVATE
    project_compile_flags
    project_link_flags
    appended
    imgui
    ${ASSIMP_LIB}
    ${GLFW_LIB}
)

##
# @brief Set output directories for Debug and Release builds.
#
set_target_properties(
    ${CMAKE_PROJECT_NAME}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY         "${CMAKE_CURRENT_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_Debug   "${CMAKE_CURRENT_BINARY_DIR}/bin/Debug"
    RUNTIME_OUTPUT_DIRECTORY_Release "${CMAKE_CURRENT_BINARY_DIR}/bin/Release"
)

##
# @brief Find the Doxygen package (required).
#
find_package(Doxygen)

if (DOXYGEN_FOUND)
    message("Doxygen Found!")

    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
        ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile
        @ONLY
    )

    make_directory("${CMAKE_CURRENT_BINARY_DIR}/doxygen")

    doxygen_add_docs(
        doxygen
        ALL
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generate Doxygen files."
        CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile"
    )
endif()