from conan import ConanFile
from conan.tools.cmake import cmake_layout
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps

class CompressorRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        self.requires("glad/0.1.36")
        self.requires("opengl/system")
        self.requires("glfw/3.4")
        self.requires("glm/1.0.1")
        self.requires("stb/cci.20230920")
        self.requires("assimp/5.4.3")

    def layout(self):
        cmake_layout(self)
        
    def configure(self):
        self.options["glad"].spec = "gl"
        self.options["glad"].extensions = "GL_EXT_texture_compression_s3tc,GL_EXT_texture_sRGB,GL_ARB_texture_storage,GL_ARB_clear_texture,GL_EXT_texture_filter_anisotropic,GL_ARB_texture_cube_map_array"
        self.options["glad"].gl_profile = "core"
        self.options["glad"].gl_version = "4.1"
        