#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "learnopengl/shader_m.h"
#include "learnopengl/camera_plus.h"
#include "learnopengl/model.h"
#include "learnopengl/custom_helper.h"
#include "learnopengl/pbrmodel.h"
#include "learnopengl/pbbloom.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <functional>
#include <random>

// glfw windows call back function
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// Custom function
unsigned int LoadTexture(char const *path, bool gammaCorrection, bool flip_vertically = true, GLenum mode = GL_REPEAT);
unsigned int LoadCubemap(std::vector<std::string> faces, bool gammaCorrection, bool flip_vertically = false);
unsigned int CreateColorFramebuffer(const size_t numOfColorAttachment, unsigned int *frameColortextures, const unsigned int width, const unsigned int height, const bool multisample, const unsigned int samples, const unsigned int hdr);

// Screen Width and Height setting
const unsigned int SCR_WIDTH = 2560;
const unsigned int SCR_HEIGHT = 1440;

// Gamma value
// This time we do gamma correction in screen post-processing shader
float view_gamma = 2.2f;

// Camera
Camera camera(glm::vec3(0.0f, 1.0f, 3.0f));
float lastX = (float)SCR_WIDTH / 2.0f;
float lastY = (float)SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Timing
float deltaTime = 0.0f;    // Time between current frame and last frame
float lastFrame = 0.0f;    // Time of last frame

// Control vertical sync
bool needVerticalSync = true;
bool vSyncKeyPressed = false;

// Tone mapping options
bool  isEyeAdaptionEnable = true;   // Determine whether do the eye adaption for tone mapping
bool  eyeAdaptionKeyPressed = false;
float kMaxLuminance = 10.0f;
float kMinLuminance = 0.3f;
float kExposureAdjustSpeed = 50.0f;
float kAverageLuminance = 1.0f;
float current_luminance = 0.3f;
float last_luminance = 0.3f;
float exposure = 1.0f;              // Control the exposure value


// Determind whether activate bloom effect
int bloom_type = 2;
// Determine how many time to do blur
unsigned int bloom_blur_time = 5;
float bloom_mix_strength = 0.04f;


// Structure to store PBR materials textures
struct PbrTextures {
    unsigned int albedo, metallic, roughness, normal, height, ao;
};
// The set to store all the PBR textures
std::vector<PbrTextures> pbr_textures_set;
// The index of current showing texture
int current_pbr_textures = 0;
// Control switching PBR textures
bool rightKeyPressed = false, leftKeyPressed = false;

// Control the height mapping scale
float height_scale = 0.1f;


// Structure to store IBL textures
struct IblTextures {
    unsigned int environment, irradiance, prefiltered, preBrdf;
};
// The set to store all the IBL textures
std::vector<IblTextures> ibl_textures_set;
// The index of current showing texture
int current_ibl_textures = 0;
// Control switching IBL textures
bool upKeyPressed = false, downKeyPressed = false;

// Function used to bake IBL textures
IblTextures BakeIblTex(const char *hdr_path, const unsigned int cubemap_vao, const unsigned int screen_quad_vao);


static bool k_key_pressed = false;
static auto cursor_mode = GLFW_CURSOR_NORMAL;


// Control and initialization for lighting
static size_t current_control_dir_index   = 0;
static size_t current_control_point_index = 0;
static size_t current_control_spot_index  = 0;
static const size_t  flashlight_index    = 15;
static glm::vec3 dir_light_color = glm::vec3(220.0f, 200.0f, 200.0f);
static glm::vec3 dir_light_direction = glm::vec3(0.0f, -1.0f, 0.0f);
static CustomHelper::BlinnPhongLight_direct dir_light = {
    GL_FALSE,
    dir_light_direction,
    dir_light_color * glm::max(glm::dot(dir_light_direction, glm::vec3(0.0f, -1.0f, 0.0f)), 0.0f) * 0.002f,
    dir_light_color * glm::max(glm::dot(dir_light_direction, glm::vec3(0.0f, -1.0f, 0.0f)), 0.0f),
    dir_light_color * glm::max(glm::dot(dir_light_direction, glm::vec3(0.0f, -1.0f, 0.0f)), 0.0f)
};
static float sun_offset = 20.0f;
static std::vector<CustomHelper::BlinnPhongLight_point> point_lights(CustomHelper::MAX_NUM_SHADOWPOINTLIGHT);
static std::vector<CustomHelper::BlinnPhongLight_spot> spot_lights(CustomHelper::MAX_NUM_SHADOWSPOTLIGHT - 1);
static std::vector<std::string> point_lights_index_str_list;
static std::vector<std::string> spot_lights_index_str_list;
static std::vector<const char*> point_lights_index_list;
static std::vector<const char*> spot_lights_index_list;
static glm::vec3 flashlight_color(5.0f, 5.0f, 5.0f);
static CustomHelper::BlinnPhongLight_spot flashlight = {
    GL_FALSE,
    glm::vec3(0.0f, 1.0f, 1.0f),
    glm::vec3(-4.0f, -0.3f, 0.0f),
    45.5f,
    48.5f,
    0.0f,
    0.0f,
    1.0f,
    flashlight_color * 0.02f,
    flashlight_color * 0.8f,
    flashlight_color * 1.0f
};


static int tone_mapping_mode = 0;

// G-Buffer creation function
unsigned int createGBuffer(unsigned int width, unsigned int height, std::vector<unsigned int>& gTextures);

// Control SSAO
bool applySsao = true;
float ssao_power = 5.0f;
float ssao_sample_radius = 1.0f;
float ssao_sample_bias   = 0.0f;

// Control Screen Space Reflection
bool  applySsr = true;
float ssr_stepSize = 0.25f;
float ssr_maxDistance = 30.0f;
int   ssr_maxSteps = 90.0f;
float ssr_hitThreshold = 0.133f;
float ssr_reflectionStrength = 1.0f;

float sigmaS = 0.01f;
float sigmaR = 0.01f;

// Control Rain Wet Floor Effect
size_t rain_shadow_index = 3;
GLboolean isRainDay = GL_FALSE;
static CustomHelper::BlinnPhongLight_direct rain_dir_light = {
    GL_FALSE,
    glm::vec3(0.0f, -1.0f, 0.0f),
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 0.0f, 0.0f)
};

// Control FXAA
bool applyFxaa = true;

int main(void) {

    /*
    * glfw: Initialize and configure
    * --------------------------------------------------------------------------------------------------------------------
    */

    // Initialize the glfw
    glfwInit();

    // Tell the glfw what version of opengl do we want to use, this time we use version 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

    // Tell glfw what kind of profile we want to use, this time is core profile
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    // Tell glfw to create multisample buffer with 4 subsamples
    /*
    const unsigned int multiSamples = 4;
    glfwWindowHint(GLFW_SAMPLES, multiSamples);
    */


    /* If you are a Mac OS X user, you have to add this code: */
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif



    /*
    * glfw: Window creation
    * --------------------------------------------------------------------------------------------------------------------
    */

    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "PBR Sponza", NULL, NULL);

    // Check if the window open successfully
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Register the call back function
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // Tell GLFW to capture our mouse
    cursor_mode = GLFW_CURSOR_DISABLED;
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);



    /*
    * glad: load all OpenGL function pointers
    * --------------------------------------------------------------------------------------------------------------------
    */

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }



    /*
    * ImGui: Gui Initialize
    * --------------------------------------------------------------------------------------------------------------------
    */

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    // Somewhere in your initialization code
    // Load a font (.ttf) with desired pixel size, for example 20.0f
    io.Fonts->AddFontFromFileTTF("fonts/Play-Regular.ttf", 20.0f);

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
#ifdef __EMSCRIPTEN__
    ImGui_ImplGlfw_InstallEmscriptenCallbacks(window, "#canvas");
#endif
    ImGui_ImplOpenGL3_Init("#version 330");




    /*
    * OpenGL: Congifure OpenGL global state
    * --------------------------------------------------------------------------------------------------------------------
    */

    // Enable multisampling
    /*
    glEnable(GL_MULTISAMPLE);
    */



    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    // Disable writing to the depth buffer
    /*
    glDepthMask(GL_FALSE);
    */
    // Configure depth test function
    glDepthFunc(GL_LESS);


    // Enable stencil testing
    /*
    glEnable(GL_STENCIL_TEST);
    */
    // Set up stencil writting mask
    /*
    glStencilMask(0xFF); // each bit is written to the stencil buffer as is
    glStencilMask(0x00); // each bit ends up as 0 in the stencil buffer (disabling writes)
    */
    // Determines whether a fragment passes or is discarded.
    /*
    glStencilFunc(GL_EQUAL, 1, 0xFF);
    */
    // Set how we can actually update the buffer.
    /*
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    */


    // Enable blending
    glEnable(GL_BLEND);
    // Set up blending factors
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Set up blending constant color
    /*
    glBlendColor();
    */
    // Set up RGBA blending factors separately
    /*
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
    */
    // Change the operator between the source and destination part of the blending equation.
    /*
    glBlendEquation(GLenum mode);
    */


    // Enable face culling
    glEnable(GL_CULL_FACE);
    // Change the type of face we want to cull
    /*
    glCullFace(GL_FRONT);
    */
    // Change which winding order is the front face
    /*
    glFrontFace(GL_CW);  // The clockwise is the front face
    */


    // Enable rendering point size changes via the vertex shader
    /*
    glEnable(GL_PROGRAM_POINT_SIZE);
    */


    // Enable OpenGL's built-in sRGB framebuffer support.
    /*
    glEnable(GL_FRAMEBUFFER_SRGB);
    */


    // Enable more smooth cube map edge
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);



    /*
    * Extra function
    * --------------------------------------------------------------------------------------------------------------------
    */

    // You can enable this function to querying how many vertex attribute that your hardware allow
    /*
    int nrAttributes;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
    std::cout << "Maximum nr of vertex attributes supported: " << nrAttributes << std::endl;
    */



    /*
     * Model loading
     * --------------------------------------------------------------------------------------------------------------------
     */

    PbrModel pbr_sponza("models/sponza_pbr/sponza.obj", true, true);
    //PbrModel gun("models/Cerberus_by_Andrew_Maximov/Cerberus_LP.FBX", false, true);

     /*
     * Set up vertex data (and buffer(s)) and configure vertex attributes
     * --------------------------------------------------------------------------------------------------------------------
     */
    CustomHelper::VAOManager vaoManager;
    unsigned int cubemapVAO = vaoManager.getVAO(CustomHelper::VAO_SKYBOX);
    unsigned int quadVAO = vaoManager.getVAO(CustomHelper::VAO_QUAD);
    unsigned int cubeVAO = vaoManager.getVAO(CustomHelper::VAO_CUBE);
    unsigned int roomVAO = vaoManager.getVAO(CustomHelper::VAO_ROOM);
    // Special VAO
    // Sphere VAO : Using glDrawElements wiht GL_TRIANGLE_STRIP mod
    unsigned int sphereVAO = vaoManager.getVAO(CustomHelper::VAO_SPHERE);
    size_t sphere_index_count = vaoManager.getSphereIndexCount();



    /*
     * Instance data calculation
     * --------------------------------------------------------------------------------------------------------------------
     */



     /*
      * Texture loading
      * --------------------------------------------------------------------------------------------------------------------
      */

    PbrTextures pbr_textures;




    /*
     * Cubemap loading
     * --------------------------------------------------------------------------------------------------------------------
     */

    const std::string folder_path = "cubemaps/skybox/";
    std::vector<std::string> faces = {
        folder_path + "right.png",
        folder_path + "left.png",
        folder_path + "top.png",
        folder_path + "bottom.png",
        folder_path + "front.png",
        folder_path + "back.png"
    };

    unsigned int skyboxTexture = LoadCubemap(faces, true);



    /*
    * Build and compile shader program
    * --------------------------------------------------------------------------------------------------------------------
    */

    Shader screenShader("shaders/post-processing/vertex/regular_screen.vert", "shaders/post-processing/fragment/regular_screen.frag");
    Shader skyboxShader("shaders/others/vertex/skybox.vert", "shaders/others/fragment/skybox.frag");
    Shader lightCubeShader("shaders/others/vertex/light_cube.vert", "shaders/others/fragment/light_cube.frag");
    Shader dirDepthShader("shaders/bake/depth_map/vertex/dir-depth_map.vert", "shaders/bake/depth_map/fragment/dir-depth_map.frag");
    Shader cubeDepthShader("shaders/bake/depth_map/vertex/cube-depth_map.vert", "shaders/bake/depth_map/fragment/cube-depth_map.frag", "shaders/bake/depth_map/geometry/cube-depth_map.geom");

    // Shader for g-buffer
    Shader gbufferShader("shaders/deferred_shading/g-buffer/vertex/height_mapping.vert", "shaders/deferred_shading/g-buffer/fragment/pbr_model_geometry.frag");
    Shader gbufferRainShader("shaders/deferred_shading/g-buffer/vertex/height_mapping_rain.vert", "shaders/deferred_shading/g-buffer/fragment/pbr_model_geometry_rain.frag");
    
	// Shaders for SSAO
	Shader ssaoShader("shaders/deferred_shading/ao/vertex/regular_screen.vert", "shaders/deferred_shading/ao/fragment/ssao.frag");
	Shader ssaoBlurShader("shaders/deferred_shading/ao/vertex/regular_screen.vert", "shaders/deferred_shading/ao/fragment/ao_blur.frag");

    // Shader for deferred shading
    Shader deferredShadingShader("shaders/deferred_shading/lighting/vertex/regular_screen_fix.vert", "shaders/deferred_shading/lighting/fragment/pbr_global-image-based-lighting_texture.frag");

    // Shader for SSR
    Shader screenSpaceReflectionShader("shaders/deferred_shading/ssr/vertex/regular_screen.vert", "shaders/deferred_shading/ssr/fragment/screen_space_reflection.frag");
    Shader filterShader("shaders/post-processing/vertex/regular_screen.vert", "shaders/post-processing/fragment/bilateral_filter.frag");
    Shader ssrBlendShader("shaders/deferred_shading/ssr/vertex/regular_screen.vert", "shaders/deferred_shading/ssr/fragment/ssr_blend.frag");

    // Regular quad screen shader
    Shader regular_screenShader("shaders/post-processing/vertex/regular_screen.vert", "shaders/post-processing/fragment/regular_screen.frag");
    // HDR screen shader using Exposure tone mapping
    Shader hdr_attenuation_screenShader("shaders/post-processing/vertex/regular_screen.vert", "shaders/post-processing/fragment/hdr_linear_attenuation_screen.frag");
    // HDR screen shader using Reinhard tone mapping
    Shader hdr_gamma_correction_screenShader("shaders/post-processing/vertex/regular_screen.vert", "shaders/post-processing/fragment/hdr_gamma_correction.frag");
    // HDR screen shader using Reinhard tone mapping
    Shader hdr_reinhard_screenShader("shaders/post-processing/vertex/regular_screen.vert", "shaders/post-processing/fragment/hdr_reinhard_screen.frag");
    // HDR screen shader using ACES tone mapping
    Shader hdr_aces_screenShader("shaders/post-processing/vertex/regular_screen.vert", "shaders/post-processing/fragment/hdr_aces_screen.frag");
    // HDR AGX tone mapping
    Shader hdr_agx_screenShader("shaders/post-processing/vertex/regular_screen.vert", "shaders/post-processing/fragment/hdr_agx_screen.frag");
    // HDR screen shader using Filmic tone mapping
    Shader hdr_agx_punchy_screenShader("shaders/post-processing/vertex/regular_screen.vert", "shaders/post-processing/fragment/hdr_agx_punchy_screen.frag");
    // Shader just for render texture on simple shape
    Shader textureShader("shaders/lighting/vertex/lighting_texture.vert", "shaders/lighting/fragment/global-lighting_texture.frag");

    // Shader for FXAA
    Shader fxaaShader("shaders/post-processing/vertex/regular_screen.vert", "shaders/post-processing/fragment/simple_fxaa.frag");


    // Shaders for bloom
    Shader hdr_bloom_screenShader("shaders/post-processing/vertex/regular_screen.vert", "shaders/post-processing/fragment/hdr_bloom_extract.frag");
    Shader bloom_gaussian_blur_screenShader("shaders/post-processing/vertex/regular_screen.vert", "shaders/post-processing/fragment/bloom_gaussian_blur.frag");
    Shader hdr_bloom_blending_screenShader("shaders/post-processing/vertex/regular_screen.vert", "shaders/post-processing/fragment/hdr_bloom_blending.frag");
    Shader hdr_bloom_mixing_screenShader("shaders/post-processing/vertex/regular_screen.vert", "shaders/post-processing/fragment/hdr_bloom_blending_mix.frag");


    // Shader for PBR
    Shader pbrIblColorShader("shaders/pbr/lighting/vertex/regular.vert", "shaders/pbr/lighting/fragment/pbr_global-image-based-lighting_color.frag");
    Shader pbrIblTextureShader("shaders/pbr/lighting/vertex/height_mapping.vert", "shaders/pbr/lighting/fragment/pbr_global-image-based-lighting_texture.frag");



    /*
     * Uniform value setting
     * --------------------------------------------------------------------------------------------------------------------
     */

    skyboxShader.use();
    skyboxShader.setInt("cubemap", 0);


    screenShader.use();
    screenShader.setInt("screenTexture", 0);


    regular_screenShader.use();
    regular_screenShader.setInt("screenTexture", 0);

    hdr_attenuation_screenShader.use();
    hdr_attenuation_screenShader.setInt("screenTexture", 0);

    hdr_gamma_correction_screenShader.use();
    hdr_gamma_correction_screenShader.setInt("screenTexture", 0);

    hdr_reinhard_screenShader.use();
    hdr_reinhard_screenShader.setInt("screenTexture", 0);

    hdr_agx_punchy_screenShader.use();
    hdr_agx_punchy_screenShader.setInt("screenTexture", 0);

    hdr_aces_screenShader.use();
    hdr_aces_screenShader.setInt("screenTexture", 0);

    hdr_agx_screenShader.use();
    hdr_agx_screenShader.setInt("screenTexture", 0);


    hdr_bloom_screenShader.use();
    hdr_bloom_screenShader.setInt("screenTexture", 0);

    // Bloom
    bloom_gaussian_blur_screenShader.use();
    bloom_gaussian_blur_screenShader.setInt("image", 0);

    hdr_bloom_blending_screenShader.use();
    hdr_bloom_blending_screenShader.setInt("scene", 0);
    hdr_bloom_blending_screenShader.setInt("bloom", 1);


    hdr_bloom_mixing_screenShader.use();
    hdr_bloom_mixing_screenShader.setInt("scene", 0);
    hdr_bloom_mixing_screenShader.setInt("bloom", 1);
    hdr_bloom_mixing_screenShader.setFloat("blend_strength", bloom_mix_strength);


    textureShader.use();
    textureShader.setInt("material.diffuse", CustomHelper::SAMPLER_DIFFUSE);
    textureShader.setInt("material.specular", CustomHelper::SAMPLER_SPECULAR);
    textureShader.setFloat("material.shininess", 64.0f);


    pbrIblTextureShader.use();
    pbrIblTextureShader.setInt("material.albedo", 0);
    pbrIblTextureShader.setInt("material.metallic", 1);
    pbrIblTextureShader.setInt("material.roughness", 2);
    pbrIblTextureShader.setInt("material.normal", 3);
    pbrIblTextureShader.setInt("heightMap", 4);
    pbrIblTextureShader.setInt("material.ao", 5);
    pbrIblTextureShader.setInt("material.opacity", 6);
    pbrIblTextureShader.setInt("environment.irradiance", 7);
    pbrIblTextureShader.setInt("environment.prefiltered", 8);
    pbrIblTextureShader.setInt("environment.preBrdf", 9);


    gbufferShader.use();
    gbufferShader.setInt("material.albedo", 0);
    gbufferShader.setInt("material.metallic", 1);
    gbufferShader.setInt("material.roughness", 2);
    gbufferShader.setInt("material.normal", 3);
    gbufferShader.setInt("heightMap", 4);
    gbufferShader.setInt("material.ao", 5);
    gbufferShader.setInt("material.opacity", 6);

    gbufferRainShader.use();
    gbufferRainShader.setInt("material.albedo", 0);
    gbufferRainShader.setInt("material.metallic", 1);
    gbufferRainShader.setInt("material.roughness", 2);
    gbufferRainShader.setInt("material.normal", 3);
    gbufferRainShader.setInt("heightMap", 4);
    gbufferRainShader.setInt("material.ao", 5);
    gbufferRainShader.setInt("material.opacity", 6);
    gbufferRainShader.setBool("rainFloor", isRainDay);


	// SSAO
	unsigned int kernel_size = 64;
	ssaoShader.use();
	ssaoShader.setInt("gPosition", 0);
	ssaoShader.setInt("gNormal", 1);
	ssaoShader.setInt("texNoise", 2);
	ssaoShader.setInt("kernelSize", kernel_size);
	ssaoShader.setVec2("screenSize", glm::vec2(SCR_WIDTH, SCR_HEIGHT));
	ssaoShader.setFloat("radius", 0.5f);
	ssaoShader.setFloat("bias", 0.025f);
	ssaoShader.setFloat("power", ssao_power);

	ssaoBlurShader.use();
	ssaoBlurShader.setInt("ssaoInput", 0);

    deferredShadingShader.use();
    deferredShadingShader.setInt("gbuffer.albedo",   0);
    deferredShadingShader.setInt("gbuffer.normal",   1);
    deferredShadingShader.setInt("gbuffer.material", 2);
    deferredShadingShader.setInt("gbuffer.position", 3);
    deferredShadingShader.setInt("ssao_texture", 4);
    deferredShadingShader.setInt("environment.irradiance", 7);
    deferredShadingShader.setInt("environment.prefiltered", 8);
    deferredShadingShader.setInt("environment.preBrdf", 9);

    screenSpaceReflectionShader.use();
    screenSpaceReflectionShader.setInt("screenInfo.albedo",   0);
    screenSpaceReflectionShader.setInt("screenInfo.normal",   1);
    screenSpaceReflectionShader.setInt("screenInfo.material", 2);
    screenSpaceReflectionShader.setInt("screenInfo.position", 3);
    screenSpaceReflectionShader.setInt("screenInfo.scene", 4);
    screenSpaceReflectionShader.setInt("environment.irradiance", 7);
    screenSpaceReflectionShader.setInt("environment.prefiltered", 8);
    screenSpaceReflectionShader.setInt("environment.preBrdf", 9);

    filterShader.use();
    filterShader.setInt("screenTexture", 0);
    filterShader.setFloat("sigmaS", sigmaS);
    filterShader.setFloat("sigmaR", sigmaR);
    filterShader.setFloat("radius", 2.0);
    filterShader.setVec2("texSize", glm::vec2(SCR_WIDTH, SCR_HEIGHT));

    ssrBlendShader.use();
    ssrBlendShader.setInt("baseColor", 0);
    ssrBlendShader.setInt("ssrColor", 1);
    ssrBlendShader.setInt("aoTexture", 2);
    ssrBlendShader.setFloat("blendStrength", ssr_reflectionStrength);

    fxaaShader.use();
    fxaaShader.setInt("screenTexture", 0);

    pbrIblColorShader.use();
    pbrIblColorShader.setInt("environment.irradiance", 6);
    pbrIblColorShader.setInt("environment.prefiltered", 7);
    pbrIblColorShader.setInt("environment.preBrdf", 8);


    glUseProgram(0);



    /*
     * Uniform Block Object setting
     * --------------------------------------------------------------------------------------------------------------------
     */

     // Camera matrices uniform block
    CustomHelper::CameraMatricesManager cameraMatManager(CustomHelper::UBOPOINT_NAME_CAMERA_MATRICES, CustomHelper::UBOPOINT_CAMERA_MATRICES);
    cameraMatManager.registerShader(skyboxShader);
    cameraMatManager.registerShader(lightCubeShader);
    cameraMatManager.registerShader(textureShader);
    cameraMatManager.registerShader(pbrIblTextureShader);
    cameraMatManager.registerShader(pbrIblColorShader);
    cameraMatManager.registerShader(gbufferShader);
    cameraMatManager.registerShader(deferredShadingShader);
    cameraMatManager.registerShader(screenSpaceReflectionShader);
    cameraMatManager.registerShader(gbufferRainShader);

    // Global light uniform block
    CustomHelper::GlobalBlinnPongLightManager globalLightManager(CustomHelper::UBOPOINT_NAME_BLINPHONG_LIGHTING, CustomHelper::UBOPOINT_BLINPHONG_LIGHTING, CustomHelper::MAX_NUM_DIRECTIONALLIGHT, CustomHelper::MAX_NUM_POINTLIGHT, CustomHelper::MAX_NUM_SPOTLIGHT);
    globalLightManager.registerShader(textureShader);
    //globalLightManager.registerShader(pbrIblTextureShader);
    globalLightManager.registerShader(pbrIblColorShader);

    // Global light with shadow uniform block
    CustomHelper::GlobalBlinnPongShadowLightManager globalShadowLightManager(
        CustomHelper::UBOPOINT_NAME_BLINPHONG_SHADOWLIGHTING,
        CustomHelper::UBOPOINT_NAME_BLINPHONG_SHADOWMATRICES,
        CustomHelper::UBOPOINT_NAME_BLINPHONG_SHADOWFARPLANE,
        CustomHelper::UBOPOINT_BLINPHONG_SHADOWLIGHTING,
        CustomHelper::UBOPOINT_BLINPHONG_SHADOWMATRICES,
        CustomHelper::UBOPOINT_BLINPHONG_SHADOWFARPLANE,
        CustomHelper::MAX_NUM_SHADOWDIRECTIONALLIGHT,
        CustomHelper::MAX_NUM_SHADOWPOINTLIGHT,
        CustomHelper::MAX_NUM_SHADOWSPOTLIGHT,
        4096,
        512,
        1024
    );
    globalShadowLightManager.registerShader(pbrIblTextureShader);
    globalShadowLightManager.registerShader(deferredShadingShader);
    globalShadowLightManager.registerShader(gbufferRainShader);

    // Gamma Correction uniform block
    CustomHelper::GammaManager gammaManager(CustomHelper::UBOPOINT_NAME_GAMMA_CORRECTION, CustomHelper::UBOPOINT_GAMMA_CORRECTION);
    // Move gamma correction to post-processing part
    gammaManager.registerShader(hdr_gamma_correction_screenShader);
    gammaManager.registerShader(hdr_reinhard_screenShader);
    gammaManager.registerShader(hdr_attenuation_screenShader);
    gammaManager.registerShader(hdr_aces_screenShader);
    gammaManager.registerShader(hdr_agx_screenShader);
    gammaManager.registerShader(hdr_agx_punchy_screenShader);



    /*
    * Frambuffers creations
    * --------------------------------------------------------------------------------------------------------------------
    */
    // Multisample framebuffer for render first scene
    // Disable MSAA since we're using deferred shading
    unsigned int hdr_ms_render_screen_texture;
    unsigned int hdr_ms_render_screen_framebuffer = CreateColorFramebuffer(1, &hdr_ms_render_screen_texture, SCR_WIDTH, SCR_HEIGHT, true, 8, 1);

    // Framebuffer for deferred shading
    unsigned int hdr_deferred_shading_screen_texture;
    unsigned int hdr_deferred_shading_screen_framebuffer = CreateColorFramebuffer(1, &hdr_deferred_shading_screen_texture, SCR_WIDTH, SCR_HEIGHT, false, 0, 1);

    // Framebuffer for ssr
    unsigned int hdr_ssr_screen_texture;
    unsigned int hdr_ssr_screen_framebuffer = CreateColorFramebuffer(1, &hdr_ssr_screen_texture, SCR_WIDTH, SCR_HEIGHT, false, 0, 1);

    // Framebuffer for ssr blur
    unsigned int hdr_ssr_screen_filter_texture;
    unsigned int hdr_ssr_screen_filter_framebuffer = CreateColorFramebuffer(1, &hdr_ssr_screen_filter_texture, SCR_WIDTH, SCR_HEIGHT, false, 0, 1);

    // Framebuffer for ssr blur
    unsigned int hdr_ssr_screen_blend_texture;
    unsigned int hdr_ssr_screen_blend_framebuffer = CreateColorFramebuffer(1, &hdr_ssr_screen_blend_texture, SCR_WIDTH, SCR_HEIGHT, false, 0, 1);

    // Initial framebuffer for post-processing
    unsigned int hdr_initial_screen_texture;
    unsigned int hdr_initial_screen_framebuffer = CreateColorFramebuffer(1, &hdr_initial_screen_texture, SCR_WIDTH, SCR_HEIGHT, false, 0, 1);

    // Framebuffer to store result image
    unsigned int ldr_fxaa_screen_texture;
    unsigned int ldr_fxaa_screen_framebuffer = CreateColorFramebuffer(1, &ldr_fxaa_screen_texture, SCR_WIDTH, SCR_HEIGHT, false, 0, 1);

    // Framebuffer to store result image
    unsigned int ldr_final_screen_texture;
    unsigned int ldr_final_screen_framebuffer = CreateColorFramebuffer(1, &ldr_final_screen_texture, SCR_WIDTH, SCR_HEIGHT, false, 0, 1);

    // Framebuffer for tone mapping
    unsigned int hdr_tone_mapping_screen_texture;
    unsigned int hdr_tone_mapping_screen_framebuffer = CreateColorFramebuffer(1, &hdr_tone_mapping_screen_texture, SCR_WIDTH, SCR_HEIGHT, false, 0, 1);

    // Framebuffer for storing image after post-processing 
    unsigned int hdr_process_screen_texture;
    unsigned int hdr_process_screen_framebuffer = CreateColorFramebuffer(1, &hdr_process_screen_texture, SCR_WIDTH, SCR_HEIGHT, false, 0, 1);

    // Framebuffer for bloom
    unsigned int hdr_bloom_screen_textures[2];
    unsigned int hdr_bloom_screen_framebuffer = CreateColorFramebuffer(2, hdr_bloom_screen_textures, SCR_WIDTH, SCR_HEIGHT, false, 0, 1);

    // Framebuffer for gaussian blur
    unsigned int pingpong_textures[2];
    unsigned int pingpong_framebuffer[2];
    for (int i = 0; i < 2; i++) {
        pingpong_framebuffer[i] = CreateColorFramebuffer(1, &pingpong_textures[i], SCR_WIDTH, SCR_HEIGHT, false, 0, 1);
    }


	// ** Deferred shading **
	// G-buffer creation
    std::vector<unsigned int> gbuffer_textures;
	unsigned int gbuffer;
    gbuffer = createGBuffer(SCR_WIDTH, SCR_HEIGHT, gbuffer_textures);

    std::vector<unsigned int> rain_gbuffer_textures;
	unsigned int rain_gbuffer;
    rain_gbuffer = createGBuffer(SCR_WIDTH, SCR_HEIGHT, rain_gbuffer_textures);

	// Framebuffer for SSAO
	unsigned int ssao_framebuffer;
	glGenFramebuffers(1, &ssao_framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, ssao_framebuffer);

	unsigned int ssao_color_texture;
	glGenTextures(1, &ssao_color_texture);
	glBindTexture(GL_TEXTURE_2D, ssao_color_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SCR_WIDTH, SCR_HEIGHT, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssao_color_texture, 0);

	// Check whether the framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

    // Framebuffer for ambient occlusion blur
	unsigned int ssao_blur_framebuffer;
	glGenFramebuffers(1, &ssao_blur_framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, ssao_blur_framebuffer);

	unsigned int ssao_blur_color_texture;
	glGenTextures(1, &ssao_blur_color_texture);
	glBindTexture(GL_TEXTURE_2D, ssao_blur_color_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SCR_WIDTH, SCR_HEIGHT, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssao_blur_color_texture, 0);

	// Check whether the framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;


    BloomRenderer hdr_physic_bloom_renderer(quadVAO);
    hdr_physic_bloom_renderer.init(SCR_WIDTH, SCR_HEIGHT);



    /*
     * Baking
     * --------------------------------------------------------------------------------------------------------------------
     */

     // Generate IBL textures
    //ibl_textures_set.push_back(BakeIblTex("textures/hdr/sponza.hdr", cubemapVAO, quadVAO));
    ibl_textures_set.push_back(BakeIblTex("textures/hdr/evening_road_01_puresky_1k.hdr", cubemapVAO, quadVAO));
    ibl_textures_set.push_back(BakeIblTex("textures/hdr/overcast_soil_puresky_2k.hdr", cubemapVAO, quadVAO));
    ibl_textures_set.push_back(BakeIblTex("textures/black.jpg", cubemapVAO, quadVAO));



	/*
	 * SSAO Pre-calculate datas
	 * --------------------------------------------------------------------------------------------------------------------
	 */
	
	// ** SSAO normal-oriented hemisphere random samples generation **
	// Create random float generator
	std::uniform_real_distribution<float> random_floats(0.0, 1.0); // Random float between [0.0, 1.0]
	std::default_random_engine generator;
	std::vector<glm::vec3> ssao_kernel;

	// Generate the sample kernel
	const unsigned int kNumOfSsaoSample = 64;
	for (unsigned int i = 0; i < kNumOfSsaoSample; i++) {
		glm::vec3 sample(
			random_floats(generator) * 2.0f - 1.0f,
			random_floats(generator) * 2.0f - 1.0f,
			random_floats(generator)
		);
		sample = glm::normalize(sample);
		sample *= random_floats(generator);

		// scale samples s.t. they're more aligned to center of kernel
		float scale = static_cast<float>(i) / 64.0f;
		scale = CustomHelper::lerp(0.1f, 1.0f, scale * scale);
		sample *= scale;

		ssao_kernel.push_back(sample);
	}

	// Send samples into shader
	ssaoShader.use();
	for (unsigned int i = 0; i < kNumOfSsaoSample; i++) {
		ssaoShader.setVec3("samples[" + std::to_string(i) + "]", ssao_kernel[i]);
	}
	glUseProgram(0);


	// Create random kernel rotations noise map
	std::vector<glm::vec3> ssao_noise;
	for (unsigned int i = 0; i < 16; i++) {
		glm::vec3 noise(
			random_floats(generator) * 2.0f - 1.0f,
			random_floats(generator) * 2.0f - 1.0f,
			0.0f
		);
		ssao_noise.push_back(noise);
	}

	// Fill the noise on a 4x4 texture
	unsigned int noise_texture;
	glGenTextures(1, &noise_texture);
	glBindTexture(GL_TEXTURE_2D, noise_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssao_noise[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);



    /*
     * Others data calculation
     * --------------------------------------------------------------------------------------------------------------------
     */

    point_lights_index_str_list.reserve(CustomHelper::MAX_NUM_SHADOWPOINTLIGHT);
    point_lights_index_list.reserve(CustomHelper::MAX_NUM_SHADOWPOINTLIGHT);

    spot_lights_index_str_list.reserve(CustomHelper::MAX_NUM_SHADOWPOINTLIGHT);
    spot_lights_index_list.reserve(CustomHelper::MAX_NUM_SHADOWPOINTLIGHT);

    for (int i = 0; i < CustomHelper::MAX_NUM_SHADOWPOINTLIGHT; ++i) {
        point_lights_index_str_list.push_back(std::to_string(i));
        point_lights_index_list.push_back(point_lights_index_str_list[i].c_str());
    }

    for (int i = 0; i < CustomHelper::MAX_NUM_SHADOWSPOTLIGHT - 1; ++i) {
        spot_lights_index_str_list.push_back(std::to_string(i));
        spot_lights_index_list.push_back(spot_lights_index_str_list[i].c_str());
    }



    /*
     * Light setting
     * --------------------------------------------------------------------------------------------------------------------
     */



    /*
     * Local Light setting
     * --------------------------------------------------------------------------------------------------------------------
     */



     /*
      * Gamma setting
      * --------------------------------------------------------------------------------------------------------------------
      */

    gammaManager.updateGamma(view_gamma);



    /*
     * Render type setting
     * --------------------------------------------------------------------------------------------------------------------
     */

     /*Enable this if you wnat to draw triangle in wireframe mode*/
     //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);



    /*
     * Render loop
     * --------------------------------------------------------------------------------------------------------------------
     */

     /* Disable v-sync */
     //glfwSwapInterval(0);

     // FPS time record
    double fps_previous_time = glfwGetTime(), fps_current_time, fps_delta_time = 0;
    // Record how many frames has passed
    unsigned int fps_passframe_count = 0;
    unsigned int fps = 0;

    while (!glfwWindowShouldClose(window)) {

        // FPS presentation
        //--------------------------------------------------
        // Refresh fps recorder
        fps_current_time = glfwGetTime();
        fps_delta_time = fps_current_time - fps_previous_time;
        fps_passframe_count++;
        // Update the fps presentation
        if (fps_delta_time >= 1.0) {
            fps = fps_passframe_count;
            // glfwSetWindowTitle(window, ("LearnOpenGL    FPS:" + std::to_string(fps)).c_str());
            fps_previous_time = fps_current_time;
            fps_passframe_count = 0;
        }

        // Calculate delta time
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;


        // Input
        //--------------------------------------------------
        processInput(window);


        // Create transformations
        //--------------------------------------------------
        // --model matrix--
        // Since each cube has its own position, we declare the matrix variable here
        glm::mat4 model;
        // --view matrix--
        glm::mat4 view = camera.GetViewMatrix();
        // --projection matrix--
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

        // --normal matrix--
        glm::mat3 normalMat(1.0f);


        // Fill the uniform buffer
        //--------------------------------------------------
        // Camera view
        cameraMatManager.updateView(view);
        // Camera projection
        cameraMatManager.updateProjection(projection);


        // Render scene
        //----------------------------------------------------------------------------------------------------------------

        glEnable(GL_DEPTH_TEST);



        // Render Shadow
        //--------------------------

        glm::vec3 sponza_position = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 sponza_scale = glm::vec3(0.01f);
        std::function<void(Shader &)> shadowDrawFunction;
        std::function<void(Shader &)> rain_shadowDrawFunction;
        shadowDrawFunction = [&](Shader &shader) {
                // Draw Boxes
                // Avoid peter panning by cull the front faces
                //glCullFace(GL_FRONT);
                model = glm::mat4(1.0f);
                model = glm::translate(model, sponza_position);
                model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                model = glm::scale(model, sponza_scale);
                shader.setMat4("model", model);
                pbr_sponza.Draw(shader);
                //glCullFace(GL_BACK);
        };
        rain_shadowDrawFunction = [&](Shader &shader) {
                // Draw Boxes
                // Avoid peter panning by cull the front faces
                //glCullFace(GL_FRONT);
                model = glm::mat4(1.0f);
                model = glm::translate(model, sponza_position);
                model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                model = glm::scale(model, sponza_scale);
                shader.setMat4("model", model);
                pbr_sponza.Draw(shader);
                //glCullFace(GL_BACK);
        };
        globalShadowLightManager.updateDirLight(dir_light, current_control_dir_index, sun_offset, 35.0f, 20.0f, sponza_position + glm::vec3(0.0f, 5.0f, 0.0f), shadowDrawFunction);
        globalShadowLightManager.updateDirLight(rain_dir_light, rain_shadow_index, sun_offset, 35.0f, 20.0f, sponza_position + glm::vec3(0.0f, 5.0f, 0.0f), rain_shadowDrawFunction);    // This is for rain effect
        globalShadowLightManager.updatePointLight(point_lights[current_control_point_index], current_control_point_index, 30.0f, shadowDrawFunction);
        globalShadowLightManager.updateSpotLight(spot_lights[current_control_spot_index], current_control_spot_index, 2.0f * spot_lights[current_control_spot_index].outerCutOff, 30.0f, shadowDrawFunction);
        flashlight.position = camera.Position
                                + camera.Right * (0.2f + static_cast<float>((glm::sin(glfwGetTime() * 0.25f) + 1.0f) * 0.1f))
                                - camera.Up * (0.2f + static_cast<float>((glm::cos(glfwGetTime() * 0.75f) + 1.0f) * 0.12f))
                                - glm::vec3(0.0f, 0.0001f, 0.0f);
        flashlight.direction = glm::normalize((camera.Position + 15.0f * camera.SightFront) - flashlight.position);
        globalShadowLightManager.updateSpotLight(flashlight, flashlight_index, 2.0f * flashlight.outerCutOff, 30.0f, shadowDrawFunction);
        globalShadowLightManager.bindShadowMaps();


        // Update global lighting
        //--------------------------



        // Render Objects
        //--------------------------


        // *** DEFERRED SHADING PASS ***
        //-------------------------------------------------------------------------------------
        // **Geometry pass**
        //----------------------------------------------------------------------


        // **Lighting pass**
        //----------------------------------------------------------------------


        // *** FORWARD SHADING PASS ***
        //-------------------------------------------------------------------------------------

        // Bind Environment textures

        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_CUBE_MAP, ibl_textures_set[current_ibl_textures].irradiance);
        glActiveTexture(GL_TEXTURE8);
        glBindTexture(GL_TEXTURE_CUBE_MAP, ibl_textures_set[current_ibl_textures].prefiltered);
        glActiveTexture(GL_TEXTURE9);
        glBindTexture(GL_TEXTURE_2D, ibl_textures_set[current_ibl_textures].preBrdf);

        {
            // Bind framebuffer
            glBindFramebuffer(GL_FRAMEBUFFER, rain_gbuffer);

            // Disable blending so alpha won't affect the result
            glDisable(GL_BLEND);

            // Rescale the view port to the size of the screen
            glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

            // Render command
            //---------------
            // Clear Buffer
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);	// keep it black so it doesn't leak into g-buffer
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            gbufferRainShader.use();
            
            model = glm::mat4(1.0f);
            model = glm::translate(model, sponza_position);
            model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, sponza_scale);
            normalMat = CustomHelper::CalculateNormalMat(view * model);
            glm::mat3 normalMatW = CustomHelper::CalculateNormalMat(model);

            gbufferRainShader.setMat4("model", model);
            gbufferRainShader.setMat3("normalMat", normalMat);
            gbufferRainShader.setMat3("normalMatW", normalMatW);
            gbufferRainShader.setFloat("height_scale", height_scale);
            gbufferRainShader.setBool("rainFloor", isRainDay);
                
            pbr_sponza.Draw(gbufferRainShader);
        }

        // Calculate the screen-space ambient occlusion
        glBindFramebuffer(GL_FRAMEBUFFER, ssao_framebuffer);
        glClearColor(1.0, 1.0, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        if (applySsao) {
            ssaoShader.use();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, rain_gbuffer_textures[3]);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, rain_gbuffer_textures[1]);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, noise_texture);

            // Set up SSAO factors
            ssaoShader.setInt  ("kernelSize", kernel_size);
            ssaoShader.setVec2 ("screenSize", glm::vec2(SCR_WIDTH, SCR_HEIGHT));
            ssaoShader.setFloat("radius"    , ssao_sample_radius);
            ssaoShader.setFloat("bias"      , ssao_sample_bias);
            ssaoShader.setFloat("power"     , ssao_power);

            glBindVertexArray(quadVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        // Blur the ao image
        glBindFramebuffer(GL_FRAMEBUFFER, ssao_blur_framebuffer);
        glClearColor(1.0, 1.0, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        if (applySsao) {

                ssaoBlurShader.use();
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, ssao_color_texture);

                glBindVertexArray(quadVAO);
                glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        // Bind framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, hdr_deferred_shading_screen_framebuffer);
        glDisable(GL_BLEND);
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);	// keep it black so it doesn't leak into g-buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        {

            deferredShadingShader.use();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, rain_gbuffer_textures[0]);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, rain_gbuffer_textures[1]);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, rain_gbuffer_textures[2]);
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, rain_gbuffer_textures[3]);
            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_2D, ssao_blur_color_texture);
            
            glm::mat3 vec_world_to_view = CustomHelper::CalculateNormalMat(view);
            glm::mat3 vec_view_to_world = CustomHelper::CalculateNormalMat(glm::inverse(view));
            deferredShadingShader.setMat3("vecWorldToView", vec_world_to_view);
            deferredShadingShader.setMat3("vecViewToWorld", vec_view_to_world);
            deferredShadingShader.setMat4("viewToWorld", glm::inverse(view));

            glBindVertexArray(quadVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        
        // Bind framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, hdr_ssr_screen_framebuffer);
        glDisable(GL_BLEND);
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        if (applySsr) {

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, rain_gbuffer_textures[0]);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, rain_gbuffer_textures[1]);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, rain_gbuffer_textures[2]);
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, rain_gbuffer_textures[3]);
            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_2D, hdr_deferred_shading_screen_texture);

            screenSpaceReflectionShader.use();
            screenSpaceReflectionShader.setFloat("stepSize", ssr_stepSize);
            screenSpaceReflectionShader.setFloat("maxDistance", ssr_maxDistance);
            screenSpaceReflectionShader.setInt("maxSteps", ssr_maxSteps);
            screenSpaceReflectionShader.setFloat("hitThreshold", ssr_hitThreshold);
            screenSpaceReflectionShader.setFloat("reflectionStrength", ssr_reflectionStrength);

            glBindVertexArray(quadVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        // Bind framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, hdr_ssr_screen_filter_framebuffer);
        glDisable(GL_BLEND);
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        if (applySsr) {

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, hdr_ssr_screen_texture);

                filterShader.use();
                filterShader.setFloat("sigmaS", sigmaS);
                filterShader.setFloat("sigmaR", sigmaR);

                glBindVertexArray(quadVAO);
                glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        {
                // Bind framebuffer
                glBindFramebuffer(GL_FRAMEBUFFER, hdr_ssr_screen_blend_framebuffer);
                
                // Disable blending so alpha won't affect the result
                glDisable(GL_BLEND);

                // Rescale the view port to the size of the screen
                glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
                // Clear Buffer
                glClearColor(0.0f, 0.0f, 0.0f, 0.0f);	// keep it black so it doesn't leak into g-buffer
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, hdr_deferred_shading_screen_texture);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, hdr_ssr_screen_filter_texture);
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, ssao_blur_color_texture);

                ssrBlendShader.use();
                ssrBlendShader.setFloat("blendStrength", ssr_reflectionStrength);

                glBindVertexArray(quadVAO);
                glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        
        
        // First copy the depth buffer info from geometry pass to current working framebuffer
		glBindFramebuffer(GL_READ_FRAMEBUFFER, rain_gbuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, hdr_ssr_screen_blend_framebuffer);
		glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

        // Draw light cube
        CustomHelper::DrawGlobalPointLightSphere(globalShadowLightManager.getLightManager(), sphereVAO, sphere_index_count, lightCubeShader, 0.025f);
        CustomHelper::DrawGlobalSpotLightCube(globalShadowLightManager.getLightManager(), cubeVAO, lightCubeShader, 0.025f);



        // skybox
        //---------------
        // Since the default value in depth buffer is 1.0, so the fragment should pass the depth test when the depth of fragment is less or equal to
        // the value store in the depth buffer. This can avoid the depth fighting.

        glDepthFunc(GL_LEQUAL);

        // Activate the shader
        skyboxShader.use();

        // Bind cubemap
        glActiveTexture(GL_TEXTURE0);
        if (isRainDay) {
            glBindTexture(GL_TEXTURE_CUBE_MAP, ibl_textures_set[1].environment);
        }
        else {
            glBindTexture(GL_TEXTURE_CUBE_MAP, ibl_textures_set[0].environment);
        }

        glBindVertexArray(cubemapVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        // Set the depth function and culling face to default
        glDepthFunc(GL_LESS);
        glCullFace(GL_BACK);





        // **POST-PROCESSING PASS**
        //-------------------------------------------------------------------------------------

        // Transfer the color from multisamples framebuffer to normal framebuffer
        // Since we're using deferred shading now, we disable MSAA
        glBindFramebuffer(GL_READ_FRAMEBUFFER, hdr_ssr_screen_blend_framebuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, hdr_initial_screen_framebuffer);
        glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_COLOR_BUFFER_BIT, GL_LINEAR);


        // Also copy the image to the framebuffer used to store post-processing effects
        glBindFramebuffer(GL_READ_FRAMEBUFFER, hdr_initial_screen_framebuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, hdr_process_screen_framebuffer);
        glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_COLOR_BUFFER_BIT, GL_LINEAR);



        // Post-processing scene
        //-------------------------
        glDisable(GL_DEPTH_TEST);

        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);


        // Bloom
        //--------------------
        glBindFramebuffer(GL_FRAMEBUFFER, hdr_bloom_screen_framebuffer);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);


        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, hdr_process_screen_texture);

        hdr_bloom_screenShader.use();

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        if (bloom_type == 1) {
            // Do gaussian blur
            bool horizontal = true, first_iteration = true;
            // How many times Gaussian blur to do
            bloom_gaussian_blur_screenShader.use();
            // Do two-pass Gaussian blur
            for (unsigned int i = 0; i < bloom_blur_time * 2; i++) {
                glBindFramebuffer(GL_FRAMEBUFFER, pingpong_framebuffer[horizontal]);
                bloom_gaussian_blur_screenShader.setBool("horizontal", horizontal);
                // Use previous extract image for first time blur, then swap ping pong texture each iteration
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, (first_iteration) ? hdr_bloom_screen_textures[1] : pingpong_textures[!horizontal]);
                // Blur
                glBindVertexArray(quadVAO);
                glDrawArrays(GL_TRIANGLES, 0, 6);
                // Swap horizontal state each iteration
                horizontal = !horizontal;

                if (first_iteration) {
                    first_iteration = !first_iteration;
                }
            }

            // Combine bloom to scene
            glBindFramebuffer(GL_FRAMEBUFFER, hdr_process_screen_framebuffer);
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            hdr_bloom_blending_screenShader.use();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, hdr_bloom_screen_textures[0]);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, pingpong_textures[1]);
            // Combine
            glBindVertexArray(quadVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        else if (bloom_type == 2) {
            hdr_physic_bloom_renderer.renderBloomTexture(hdr_bloom_screen_textures[0], 0.005f);

            // Combine bloom to scene
            glBindFramebuffer(GL_FRAMEBUFFER, hdr_process_screen_framebuffer);
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            hdr_bloom_mixing_screenShader.use();
            hdr_bloom_mixing_screenShader.setFloat("blend_strength", bloom_mix_strength);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, hdr_bloom_screen_textures[0]);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, hdr_physic_bloom_renderer.getBloomTexture());
            // Combine
            glBindVertexArray(quadVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }


        // Tone mapping
        //--------------------


        glBindFramebuffer(GL_FRAMEBUFFER, hdr_tone_mapping_screen_framebuffer);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, hdr_process_screen_texture);

        hdr_attenuation_screenShader.use();

        if (isEyeAdaptionEnable) {
            // Generate mipmap
            glGenerateMipmap(GL_TEXTURE_2D);
            // Get maximum texture mipmap level
            size_t max_mipmap_level = static_cast<size_t>(std::log2(std::max(SCR_WIDTH, SCR_HEIGHT)));

            //
            // The automatic exposure algorithm is from https://blog.csdn.net/coldkaweh/article/details/62893076
            // And the average luminace is based on lowest mipmap level generate by OpenGL itself, so it is very inefficient
            //
            // Get average color by get lowest mipmap level pixel color
            glm::vec3 average_color;
            glGetTexImage(GL_TEXTURE_2D, max_mipmap_level, GL_RGB, GL_FLOAT, &average_color);
            if (std::isnan(average_color.r) || std::isnan(average_color.g) || std::isnan(average_color.b)) {
                average_color = glm::vec3(1.0f);
            }
            // Calculate the real luminance in the scene
            float real_luminance = 0.2126f * average_color.r + 0.7152 * average_color.g + 0.0722 * average_color.b;

            // Calculate adapted luminance
            float adapted_luminance = last_luminance + (real_luminance - last_luminance) * (1.0 - std::pow(0.98f, kExposureAdjustSpeed * (1.0f / fps)));
            // Clamp the luminance
            adapted_luminance = std::clamp(adapted_luminance, kMinLuminance, kMaxLuminance);
            adapted_luminance = std::max(adapted_luminance, 0.0001f);

            // Record the adpated luminance to last luminace
            last_luminance = adapted_luminance;
            // Calculate the exposure
            exposure = kAverageLuminance / adapted_luminance;

            // Transfer exposure value to shader
            hdr_attenuation_screenShader.setFloat("attenuation", exposure);

            gammaManager.updateGamma(1.0f);
            glBindVertexArray(quadVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            gammaManager.updateGamma(view_gamma);
        }
        else {
            regular_screenShader.use();
            glBindVertexArray(quadVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            gammaManager.updateGamma(view_gamma);
        }


        glBindFramebuffer(GL_FRAMEBUFFER, hdr_process_screen_framebuffer);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, hdr_tone_mapping_screen_texture);
        // Determine whether to do automatic exposure adjustment. Using Reinhard tone mapping if not
        if (tone_mapping_mode == 0) {
            kMaxLuminance = 100.0f;
            kMinLuminance = 0.1f;
            kAverageLuminance = 0.2f;
            hdr_gamma_correction_screenShader.use();
        }
        else if (tone_mapping_mode == 1) {
            kMaxLuminance = 100.0f;
            kMinLuminance = 0.1f;
            kAverageLuminance = 0.2f;
            // Transfer exposure value to shader
            hdr_reinhard_screenShader.use();
        }
        else if (tone_mapping_mode == 2) {
            kMaxLuminance = 100.0f;
            kMinLuminance = 0.1f;
            kAverageLuminance = 0.2f;
            // Transfer exposure value to shader
            hdr_aces_screenShader.use();
        }
        else if (tone_mapping_mode == 3) {
            kMaxLuminance = 100.0f;
            kMinLuminance = 0.1f;
            kAverageLuminance = 0.2f;
            // Transfer exposure value to shader
            hdr_agx_screenShader.use();
        }
        else if (tone_mapping_mode == 4) {
            kMaxLuminance = 100.0f;
            kMinLuminance = 0.1f;
            kAverageLuminance = 0.2f;
            // Transfer exposure value to shader
            hdr_agx_punchy_screenShader.use();
        }

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);


        // Transfer result to process image
        //glBindFramebuffer(GL_READ_FRAMEBUFFER, hdr_tone_mapping_screen_framebuffer);
        //glBindFramebuffer(GL_DRAW_FRAMEBUFFER, hdr_process_screen_framebuffer);
        //glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_COLOR_BUFFER_BIT, GL_LINEAR);



        // Render final scene
        //-------------------------

        // Transfer final image to ldr framebuffer
        glBindFramebuffer(GL_READ_FRAMEBUFFER, hdr_process_screen_framebuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ldr_final_screen_framebuffer);
        glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_COLOR_BUFFER_BIT, GL_LINEAR);

        // Disable depth test
        glDisable(GL_DEPTH_TEST);


        // Render final result to screen framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        if (applyFxaa) {
            fxaaShader.use();            
        }
        else {
            screenShader.use();
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ldr_final_screen_texture);

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);


        // Unbind VAO
        glBindVertexArray(0);


        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        if (cursor_mode == GLFW_CURSOR_NORMAL) {
            ImGuiIO& io = ImGui::GetIO();
            io.WantCaptureMouse = true;
            io.WantCaptureKeyboard = true;

            ImGui::Begin("Editor");
            
            if (ImGui::CollapsingHeader("Atmosphere")) {
                ImGui::Indent();

                ImGui::PushID("Atmosphere");
                ImGui::Checkbox("Rain Wet Floor", reinterpret_cast<bool*>(&isRainDay));
                ImGui::PopID();

                ImGui::Unindent();
            }
            
            if (ImGui::CollapsingHeader("Light Sources")) {

                ImGui::Indent();

                if (ImGui::CollapsingHeader("Sun Light")) {
                    ImGui::PushID("Sun Light");
                    ImGui::Checkbox("Activate", reinterpret_cast<bool*>(&dir_light.activate));
                    ImGui::DragFloat3("Direction", glm::value_ptr(dir_light_direction), 0.0025f, -1.0f, 1.0f);
                    if (dir_light_direction == glm::vec3(0.0f)) dir_light_direction = glm::vec3(0.0f, 1.0f, 0.0f);
                    dir_light_direction = glm::normalize(dir_light_direction);
                    dir_light.direction = dir_light_direction;
                    ImGui::DragFloat3("Color", glm::value_ptr(dir_light_color), 0.25f, 0.0f, 5000.0f);
                    dir_light.ambient = dir_light_color * glm::max(glm::dot(dir_light_direction, glm::vec3(0.0f, -1.0f, 0.0f)), 0.0f) * 0.002f;
                    dir_light.diffuse = dir_light_color * glm::max(glm::dot(dir_light_direction, glm::vec3(0.0f, -1.0f, 0.0f)), 0.0f);
                    dir_light.specular = dir_light_color * glm::max(glm::dot(dir_light_direction, glm::vec3(0.0f, -1.0f, 0.0f)), 0.0f);
                    ImGui::SliderFloat("Bake Area Offset", &sun_offset, 0.0f, 100.0f);
                    ImGui::Spacing();
                    ImGui::PopID();                    
                }

                if (ImGui::CollapsingHeader("Point Light")) {
                    ImGui::PushID("Point Light");
                    ImGui::Combo("Select", reinterpret_cast<int*>(&current_control_point_index), point_lights_index_list.data(), point_lights_index_list.size());
                    CustomHelper::BlinnPhongLight_point& point_ref = point_lights[current_control_point_index];
                    ImGui::Checkbox("Activate", reinterpret_cast<bool*>(&point_ref.activate));
                    ImGui::DragFloat3("Position", glm::value_ptr(point_ref.position), 0.01f);
                    ImGui::DragFloat3("Color", glm::value_ptr(point_ref.specular), 0.25f, 0.0f, 500.0f);
                    point_ref.ambient = point_ref.specular * 0.02f;
                    point_ref.diffuse = point_ref.specular * 0.8f;
                    ImGui::Spacing();
                    ImGui::PopID();
                }

                if (ImGui::CollapsingHeader("Spot Light")) {
                    ImGui::PushID("Spot Light");
                    ImGui::Combo("Select", reinterpret_cast<int*>(&current_control_spot_index), spot_lights_index_list.data(), spot_lights_index_list.size());
                    CustomHelper::BlinnPhongLight_spot& spot_ref = spot_lights[current_control_spot_index];
                    ImGui::Checkbox("Activate", reinterpret_cast<bool*>(&spot_ref.activate));
                    ImGui::DragFloat3("Position", glm::value_ptr(spot_ref.position), 0.01f);
                    ImGui::DragFloat3("Direction", glm::value_ptr(spot_ref.direction), 0.0025f, -1.0f, 1.0f);
                    spot_ref.direction = glm::normalize(spot_ref.direction);
                    if (spot_ref.direction == glm::vec3(0.0f)) dir_light_direction = glm::vec3(0.0f, 1.0f, 0.0f);
                    ImGui::DragFloat3("Color", glm::value_ptr(spot_ref.specular), 0.25f, 0.0f, 500.0f);
                    spot_ref.ambient = spot_ref.specular * 0.002f;
                    spot_ref.diffuse = spot_ref.specular * 0.8f;
                    ImGui::SliderFloat("Inner-Cutoff", &spot_ref.innerCutOff, 0.0f, spot_ref.outerCutOff);
                    ImGui::SliderFloat("Outer-Cutoff", &spot_ref.outerCutOff, spot_ref.innerCutOff, 70.0f);
                    ImGui::Spacing();
                    ImGui::PopID();                    
                }

                if (ImGui::CollapsingHeader("Flashlight")) {
                    ImGui::PushID("Flashlight");
                    ImGui::Checkbox("Activate", reinterpret_cast<bool*>(&flashlight.activate));
                    if (flashlight.direction == glm::vec3(0.0f)) dir_light_direction = glm::vec3(0.0f, 1.0f, 0.0f);
                    ImGui::DragFloat3("Color", glm::value_ptr(flashlight.specular), 0.25f, 0.0f, 500.0f);
                    flashlight.ambient = flashlight.specular * 0.002f;
                    flashlight.diffuse = flashlight.specular * 0.8f;
                    ImGui::SliderFloat("Inner-Cutoff", &flashlight.innerCutOff, 0.0f, flashlight.outerCutOff);
                    ImGui::SliderFloat("Outer-Cutoff", &flashlight.outerCutOff, flashlight.innerCutOff, 70.0f);
                    ImGui::PopID();
                }

                ImGui::Unindent();
            }

            if (ImGui::CollapsingHeader("Bloom")) {
                ImGui::Indent();

                ImGui::PushID("Bloom");
                ImGui::RadioButton("Off"         , &bloom_type, 0);
                ImGui::RadioButton("Gaussian"    , &bloom_type, 1);
                ImGui::RadioButton("Physic Based", &bloom_type, 2);
                ImGui::SliderFloat("Bloom Strength", &bloom_mix_strength, 0.0f, 1.0f);
                ImGui::PopID();

                ImGui::Unindent();
            }

            if (ImGui::CollapsingHeader("SSAO")) {
                ImGui::Indent();

                ImGui::PushID("SSAO");
                ImGui::Checkbox("Activate", &applySsao);
                ImGui::SliderFloat("Strength", & ssao_power, 1.0f, 10.0f);
                ImGui::SliderFloat("Sample Radius", & ssao_sample_radius, 0.25f, 2.0f);
                ImGui::SliderFloat("Sample Bias", & ssao_sample_bias, 0.0f, 1.0f);
                ImGui::PopID();

                ImGui::Unindent();
            }

            if (ImGui::CollapsingHeader("Screen Space Reflection")) {
                ImGui::Indent();

                ImGui::PushID("SSR");
                ImGui::Checkbox("Activate", &applySsr);
                ImGui::SliderFloat("StepSize", &ssr_stepSize, 0.0f, 2.0f);
                ImGui::SliderInt("Max Steps", &ssr_maxSteps, 1, 100);
                ImGui::SliderFloat("Max Distance", &ssr_maxDistance, 0.0f, 50.0f);
                ImGui::SliderFloat("Hit Threshold", &ssr_hitThreshold, 0.0f, 1.0f);
                ImGui::SliderFloat("Strength", &ssr_reflectionStrength, 0.0f, 10.0f);
                ImGui::SeparatorText("Filter");
                ImGui::SliderFloat("sigmaS", &sigmaS, 0.0f, 5.0f);
                ImGui::SliderFloat("sigmaR", &sigmaR, 0.0f, 5.0f);
                ImGui::PopID();

                ImGui::Unindent();
            }

            if (ImGui::CollapsingHeader("Tone mapping")) {
                ImGui::Indent();

                ImGui::PushID("Tone mapping");
                ImGui::Checkbox("Auto Adaptation", &isEyeAdaptionEnable);
                ImGui::Spacing();
                ImGui::RadioButton("Off", &tone_mapping_mode, 0);
                ImGui::RadioButton("Reinhard", &tone_mapping_mode, 1);
                ImGui::RadioButton("Aces", &tone_mapping_mode, 2);
                ImGui::RadioButton("Agx", &tone_mapping_mode, 3);
                ImGui::RadioButton("Agx Punchy", &tone_mapping_mode, 4);
                ImGui::PopID();

                ImGui::Unindent();
            }

            if (ImGui::CollapsingHeader("FXAA")) {
                ImGui::Indent();

                ImGui::PushID("FXAA");
                ImGui::Checkbox("Activate", &applyFxaa);
                ImGui::PopID();

                ImGui::Unindent();
            }

            ImGui::End();
            

            ImGui::Begin("Information");
            ImGui::Text("FPS: %u,  %.2f ms/frame", fps, 1.0f / static_cast<float>(fps) * 1000.0f);
            ImGui::End();
            
        }
        else {
            ImGuiIO& io = ImGui::GetIO();

            io.WantCaptureMouse = false;
            io.WantCaptureKeyboard = false;
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


        // glfw: Swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        //-----------------------------------------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose (I give up):
    // --------------------------------------------------------------------------------------------------------------------
    vaoManager.clean();
    screenShader.clear();
    skyboxShader.clear();
    dirDepthShader.clear();
    cubeDepthShader.clear();
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();


    // glfw: terminate, clearing all previously allocated GLFW resources.
    // --------------------------------------------------------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// --------------------------------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// --------------------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)                     // Press the ESC
        glfwSetWindowShouldClose(window, true);

    if (cursor_mode == GLFW_CURSOR_DISABLED) {

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.ProcessKeyboard(CAMERA_FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.ProcessKeyboard(CAMERA_BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.ProcessKeyboard(CAMERA_LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.ProcessKeyboard(CAMERA_RIGHT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            camera.ProcessKeyboard(CAMERA_UP, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            camera.ProcessKeyboard(CAMERA_DOWN, deltaTime);
    }


    // Control vertical async
    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS && !vSyncKeyPressed) {
        needVerticalSync = !needVerticalSync;
        if (needVerticalSync) {
            glfwSwapInterval(1);
        }
        else {
            glfwSwapInterval(2);
        }
        vSyncKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_RELEASE) {
        vSyncKeyPressed = false;
    }

    // Control eye adaption
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS && !eyeAdaptionKeyPressed) {
        isEyeAdaptionEnable = !isEyeAdaptionEnable;
        eyeAdaptionKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_RELEASE) {
        eyeAdaptionKeyPressed = false;
    }

    // Adjust height scale
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
        if (height_scale > 0.0f) {
            height_scale -= 0.5f * deltaTime;
        }
        else {
            height_scale = 0.0f;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) {
        if (height_scale < 1.0f) {
            height_scale += 0.5f * deltaTime;
        }
        else {
            height_scale = 1.0f;
        }
    }


    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS && !k_key_pressed) {
        cursor_mode = cursor_mode == GLFW_CURSOR_NORMAL ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL;
        glfwSetInputMode(window, GLFW_CURSOR, cursor_mode);
        k_key_pressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_RELEASE) {
        k_key_pressed = false;
    }


    // Switch IBL textures
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS && !upKeyPressed) {
        current_ibl_textures = std::abs(current_ibl_textures + 1) % ibl_textures_set.size();
        upKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS && !downKeyPressed) {
        current_ibl_textures = (current_ibl_textures == 0) ? ibl_textures_set.size() - 1 : current_ibl_textures - 1;
        downKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_RELEASE) {
        upKeyPressed = false;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_RELEASE) {
        downKeyPressed = false;
    }

}

// glfw: whenever the mouse moves, this callback is called
// --------------------------------------------------------------------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xposIn, double yposIn) {

    if (cursor_mode == GLFW_CURSOR_DISABLED) {
        float xpos = static_cast<float>(xposIn);
        float ypos = static_cast<float>(yposIn);

        // Check if the window is first time be clicked
        if (firstMouse) {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        // Calculate the x-axis and y-axis offset
        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos;  // reversed since y-coordinates go from top to bottom
        lastX = xpos;
        lastY = ypos;

        camera.ProcessMouseMovement(xoffset, yoffset);        
    }
    else {
        firstMouse = true;
    }

}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// --------------------------------------------------------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    if (cursor_mode == GLFW_CURSOR_DISABLED) {
        camera.ProcessMouseScroll(static_cast<float>(yoffset));
    }
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int LoadTexture(char const *path, bool gammaCorrection, bool flip_vertically, GLenum mode) {
    // Set whether filp vertical axis
    stbi_set_flip_vertically_on_load(flip_vertically);

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum internalFormat;
        GLenum dataFormat;
        if (nrComponents == 1) {
            internalFormat = dataFormat = GL_RED;
        }
        else if (nrComponents == 3) {
            internalFormat = (gammaCorrection ? GL_SRGB : GL_RGB);
            dataFormat = GL_RGB;
        }
        else if (nrComponents == 4) {
            internalFormat = (gammaCorrection ? GL_SRGB_ALPHA : GL_RGBA);
            dataFormat = GL_RGBA;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        if (dataFormat == GL_RGBA) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mode);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mode);
        }
        else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mode);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mode);
        }

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Anisotropy
        GLfloat value, max_anisotropy = 8.0f; /* don't exceed this value...*/
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, & value);
        value = (value > max_anisotropy) ? max_anisotropy : value;
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, value);

        glBindTexture(GL_TEXTURE_2D, 0);
        stbi_image_free(data);
    }
    else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }
    // Set whether flip vertically to false
    stbi_set_flip_vertically_on_load(false);

    return textureID;
}

// utility function for loading a cubemap textures from file
// ------------------------------------------------------------
unsigned int LoadCubemap(std::vector<std::string> faces, bool gammaCorrection, bool flip_vertically) {
    // Set whether filp vertical axis
    stbi_set_flip_vertically_on_load(flip_vertically);

    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++) {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            GLenum internalFormat;
            GLenum dataFormat;
            if (nrChannels == 1) {
                internalFormat = dataFormat = GL_RED;
            }
            else if (nrChannels == 3) {
                internalFormat = (gammaCorrection ? GL_SRGB : GL_RGB);
                dataFormat = GL_RGB;
            }
            else if (nrChannels == 4) {
                internalFormat = (gammaCorrection ? GL_SRGB_ALPHA : GL_RGBA);
                dataFormat = GL_RGBA;
            }

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // Set whether flip vertically to false
    stbi_set_flip_vertically_on_load(false);

    return textureID;
}

// Generate a framebuffer attach the given texture as the color attachment.
unsigned int CreateColorFramebuffer(const size_t numOfColorAttachment, unsigned int *frameColortextures, const unsigned int width, const unsigned int height, const bool multisample, const unsigned int samples, const unsigned int hdr) {

    unsigned int framebuffer;
    // Generate a framebuffer and get its ID
    glGenFramebuffers(1, &framebuffer);
    // Bind framebuffers
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);


    // Attach texture to framebuffer
    // Create a texture to store the scene's image
    glGenTextures(numOfColorAttachment, frameColortextures);

    // Determine whether use the multisampling texture
    GLenum texformat = (multisample) ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;

    // Determine whether use the float color attachment
    GLenum texture_color_format = GL_RGB;
    switch (hdr) {
    case 0:
        texture_color_format = GL_RGB;
        break;

    case 1:
        texture_color_format = GL_RGB16F;
        break;

    case 2:
        texture_color_format = GL_RGB32F;
        break;
    
    default:
        break;
    }
    GLenum texture_data_type = (hdr) ? GL_FLOAT : GL_UNSIGNED_BYTE;

    for (unsigned int i = 0; i < numOfColorAttachment; i++) {
        glBindTexture(texformat, frameColortextures[i]);
        if (multisample)
            glTexImage2DMultisample(texformat, samples, texture_color_format, width, height, GL_TRUE);
        else {
            glTexImage2D(texformat, 0, texture_color_format, width, height, 0, GL_RGBA, texture_data_type, NULL);
            glTexParameteri(texformat, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(texformat, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(texformat, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(texformat, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        glBindTexture(texformat, 0);

        // Attach the texture to currently bound framebuffer object
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, texformat, frameColortextures[i], 0);
    }


    // Attach Render Buffer (RBO) to framebuffer
    unsigned int RBO;
    // Generate renderbuffer and get its ID
    glGenRenderbuffers(1, &RBO);
    // Bind render buffer
    glBindRenderbuffer(GL_RENDERBUFFER, RBO);
    // Creating a depth and stencil renderbuffer object
    // Determine whether use the multisampling render buffer
    if (multisample)
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH24_STENCIL8, width, height);
    else
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
    // Unbind renderbuffer to default
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    // Actually attach the renderbuffer to the framebuffer
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

    // Activate the number of render targets
    std::vector<GLenum> attachments;
    for (size_t i = 0; i < numOfColorAttachment; i++) {
        attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
    }
    glDrawBuffers(numOfColorAttachment, attachments.data());


    // Check whether the framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

    // Bind framebuffer to default
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    return framebuffer;
}

unsigned int createGBuffer(unsigned int width, unsigned int height, std::vector<unsigned int>& gTextures) {
    // Make sure gTextures has size 4
    gTextures.resize(4);

    // 1) Create and bind the framebuffer
    unsigned int gBuffer;
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

    // 2) Create the Albedo texture (GL_COLOR_ATTACHMENT0)
    glGenTextures(1, &gTextures[0]);
    glBindTexture(GL_TEXTURE_2D, gTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gTextures[0], 0);

    // 3) Create the Normal texture (GL_COLOR_ATTACHMENT1)
    glGenTextures(1, &gTextures[1]);
    glBindTexture(GL_TEXTURE_2D, gTextures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0,
                 GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gTextures[1], 0);

    // 4) Create the Material texture (GL_COLOR_ATTACHMENT2)
    glGenTextures(1, &gTextures[2]);
    glBindTexture(GL_TEXTURE_2D, gTextures[2]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gTextures[2], 0);

    // 5) Create the Position texture (GL_COLOR_ATTACHMENT3)
    glGenTextures(1, &gTextures[3]);
    glBindTexture(GL_TEXTURE_2D, gTextures[3]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0,
                 GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gTextures[3], 0);

    // 6) Tell OpenGL which color attachments we'll use for this framebuffer
    unsigned int attachments[4] = {
        GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1,
        GL_COLOR_ATTACHMENT2,
        GL_COLOR_ATTACHMENT3
    };
    glDrawBuffers(4, attachments);

    // 7) Create a renderbuffer for depth
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    // 8) Check if the framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: G-Buffer is not complete!" << std::endl;

    // Unbind the framebuffer to avoid unwanted state changes
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Return the framebuffer ID
    return gBuffer;
}

// Bake given hdr equirectangular map into IBL textures
IblTextures BakeIblTex(const char *hdr_path, const unsigned int cubemap_vao, const unsigned int screen_quad_vao) {

    // Load equirectangular map
    stbi_set_flip_vertically_on_load(true);
    int width, height, nrComponents;
    float *data = stbi_loadf(hdr_path, &width, &height, &nrComponents, 0);
    unsigned int hdr_map;
    if (data) {
        glGenTextures(1, &hdr_map);
        glBindTexture(GL_TEXTURE_2D, hdr_map);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, data); // note how we specify the texture's data value to be float

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else {
        std::cout << "Failed to load HDR image." << std::endl;
    }
    stbi_set_flip_vertically_on_load(false);

    // Prepare baking shader
    static Shader equirToCubeShader("shaders/bake/ibl/vertex/cubemap.vert", "shaders/bake/ibl/fragment/equir_to_cube.frag");
    static Shader irradianceMapShader("shaders/bake/ibl/vertex/cubemap.vert", "shaders/bake/ibl/fragment/irradiance_map.frag");
    static Shader prefilteredMapShader("shaders/bake/ibl/vertex/cubemap.vert", "shaders/bake/ibl/fragment/pre-filtered_env.frag");
    static Shader integratedBrdfMapShader("shaders/bake/ibl/vertex/regular_screen.vert", "shaders/bake/ibl/fragment/brdf_integration.frag");

    // 
    // ** Bake equirectangular map to environment cube map **
    // 
    IblTextures ibl_textures;

    const unsigned int kEnvResolution = 512;
    unsigned int captureFBO, captureRBO;
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);
    // Creat render buffer storage
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, kEnvResolution, kEnvResolution);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

    // Generate cube map
    glGenTextures(1, &ibl_textures.environment);
    glBindTexture(GL_TEXTURE_CUBE_MAP, ibl_textures.environment);
    for (unsigned int i = 0; i < 6; i++) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, kEnvResolution, kEnvResolution, 0, GL_RGB, GL_FLOAT, NULL);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Six face camera
    static glm::mat4 capture_projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    static glm::mat4 capture_views[] = {
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };

    equirToCubeShader.use();
    equirToCubeShader.setInt("equirectangular_map", 0);
    equirToCubeShader.setMat4("projection", capture_projection);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdr_map);
    glBindVertexArray(cubemap_vao);
    glViewport(0, 0, kEnvResolution, kEnvResolution);
    for (unsigned int i = 0; i < 6; i++) {
        equirToCubeShader.setMat4("view", capture_views[i]);
        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            ibl_textures.environment,
            0
        );
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    // Generate mipmap
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // 
    // ** Bake irradiance map **
    // 
    const unsigned int kIrrResolution = 32;
    glGenTextures(1, &ibl_textures.irradiance);
    glBindTexture(GL_TEXTURE_CUBE_MAP, ibl_textures.irradiance);
    for (unsigned int i = 0; i < 6; i++) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, kIrrResolution, kIrrResolution, 0, GL_RGB, GL_FLOAT, NULL);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Rescale the framebuffer to fit irradiance map's resolution
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, kIrrResolution, kIrrResolution);

    irradianceMapShader.use();
    irradianceMapShader.setInt("environment_map", 0);
    irradianceMapShader.setMat4("projection", capture_projection);

    // Start baking irradiance map
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, ibl_textures.environment);
    glBindVertexArray(cubemap_vao);
    glViewport(0, 0, kIrrResolution, kIrrResolution);
    for (unsigned int i = 0; i < 6; i++) {
        equirToCubeShader.setMat4("view", capture_views[i]);
        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            ibl_textures.irradiance,
            0
        );
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    //
    // ** Bake pre-filtered map
    //
    const unsigned int kFilteredBasicResolution = 256;

    glGenTextures(1, &ibl_textures.prefiltered);
    glBindTexture(GL_TEXTURE_CUBE_MAP, ibl_textures.prefiltered);
    for (unsigned int i = 0; i < 6; i++) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, kFilteredBasicResolution, kFilteredBasicResolution, 0, GL_RGB, GL_FLOAT, NULL);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    prefilteredMapShader.use();
    prefilteredMapShader.setInt("environment_map", 0);
    prefilteredMapShader.setInt("env_resolution", kEnvResolution);
    prefilteredMapShader.setMat4("projection", capture_projection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, ibl_textures.environment);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    unsigned int max_mip_level = 5;
    glBindVertexArray(cubemap_vao);
    for (unsigned int mip = 0; mip < max_mip_level; ++mip) {
        // Resize framebuffer according to mip-level size
        unsigned int mip_width = kFilteredBasicResolution * std::pow(0.5, mip);
        unsigned int mip_height = kFilteredBasicResolution * std::pow(0.5, mip);
        glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mip_width, mip_height);
        glViewport(0, 0, mip_width, mip_height);

        float roughness = static_cast<float>(mip) / static_cast<float>(max_mip_level - 1);
        prefilteredMapShader.setFloat("roughness", roughness);
        for (unsigned int i = 0; i < 6; ++i) {
            prefilteredMapShader.setMat4("view", capture_views[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, ibl_textures.prefiltered, mip);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // Pre-allocate enough memory for the LUT texture.
    const unsigned int kPreBrdfResolution = 512;

    glGenTextures(1, &ibl_textures.preBrdf);
    glBindTexture(GL_TEXTURE_2D, ibl_textures.preBrdf);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, kPreBrdfResolution, kPreBrdfResolution, 0, GL_RG, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, kPreBrdfResolution, kPreBrdfResolution);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ibl_textures.preBrdf, 0);

    integratedBrdfMapShader.use();
    glViewport(0, 0, kPreBrdfResolution, kPreBrdfResolution);
    // Disable blend is important
    glDisable(GL_BLEND);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(screen_quad_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);



    glDeleteRenderbuffers(1, &captureRBO);
    glDeleteFramebuffers(1, &captureFBO);
    glDeleteTextures(1, &hdr_map);

    return ibl_textures;
}