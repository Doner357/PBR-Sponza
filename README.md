# PBR Sponza

A native **C++23 / OpenGL 4.6** renderer built around the Sponza scene, used as a graphics-programming playground for integrating PBR, deferred rendering, shadows, screen-space effects, HDR post-processing, and runtime experimentation.

> **Project context**
>
> This project grew out of my LearnOpenGL study. Some individual techniques—such as PBR/IBL, deferred shading, SSAO, bloom, and shadow mapping—were first learned from tutorial material.  
> I then integrated them into a larger Sponza renderer and added project-specific extensions such as **screen-space reflections, a rain / wet-floor material effect, expanded tone mapping with automatic exposure, and an ImGui-based runtime editor**.
>
> I present this repository as an **independent extension and integration project built on tutorial foundations**, not as an original rendering-research contribution.

## Demo

[YouTube showcase — PBR Sponza Demo](https://youtu.be/V9zPx25o9wY)

## Screenshots

### Main render

![Main Sponza render](docs/screenshots/main-render.png)

### Runtime editor

![Lighting and material controls](docs/screenshots/editor-controls.png)

### Rain / wet-floor and reflections

![Rain and reflection pass](docs/screenshots/rain-reflection.png)

### Tone-mapping comparison

![Tone mapping comparison](docs/screenshots/tone-mapping.png)

---

## What I Extended Beyond the Tutorial Foundation

### Screen-Space Reflections

The renderer includes an SSR pass that:

- reconstructs reflection rays in view space;
- ray-marches against the deferred position buffer;
- performs screen-space hit testing;
- samples reflected scene color on a hit;
- applies bilateral filtering before the reflection contribution is blended back into the scene.

Relevant shaders:

```text
assets/shaders/deferred_shading/ssr/
assets/shaders/post-processing/fragment/bilateral_filter.frag
```

### Rain / Wet-Floor Material Effect

The rain mode uses an additional directional shadow pass to estimate which surfaces are exposed to the virtual rain.

For exposed surfaces, the G-buffer pass:

- reduces roughness;
- adjusts surface normals for sufficiently upward-facing surfaces;
- preserves the underlying PBR material instead of replacing it with a separate wet material.

This produces a more reflective wet-floor appearance while reusing the existing deferred PBR pipeline.

Relevant shader:

```text
assets/shaders/deferred_shading/g-buffer/fragment/pbr_model_geometry_rain.frag
```

### Expanded Tone Mapping and Automatic Exposure

The post-processing pipeline supports multiple tone-mapping modes:

- Reinhard
- ACES
- AgX
- AgX Punchy

An experimental eye-adaptation path also estimates average scene luminance from the lowest generated mip level and gradually adapts the exposure value over time.

The current implementation intentionally favors experimentation over efficiency: it reads the lowest mip level back with `glGetTexImage`, so this path should not be treated as an optimized production implementation.

### Runtime Rendering Editor

Dear ImGui is integrated as a runtime control surface for experimenting with renderer parameters.

The editor exposes controls for:

- directional, point, spot, and flashlight parameters;
- shadow-related lighting settings;
- SSAO;
- SSR step size, maximum steps, hit threshold, reflection strength, and filtering;
- Gaussian and physically based bloom paths;
- tone mapping and automatic exposure;
- FXAA;
- rain / wet-floor mode.

This makes the project useful as an interactive rendering playground rather than a fixed demonstration.

### Shadow-Light Management

The renderer manages directional, point, and spot shadow-casting lights through a shared shadow-light manager.

The manager keeps the previous parameters for each light and can skip rebaking a shadow map when the relevant light state has not changed.

---

## Rendering Pipeline

At a high level, the renderer combines:

```text
Sponza PBR Assets
        |
        v
Deferred G-buffer
(albedo / normal / material / position)
        |
        +----> SSAO
        |
        +----> Shadow Maps
        |
        v
Deferred PBR Lighting + IBL
        |
        +----> Screen-Space Reflections
        |
        +----> Bloom
        |
        +----> HDR Tone Mapping / Auto Exposure
        |
        +----> FXAA
        |
        v
Final Frame
```

The IBL path bakes an HDR environment map into:

- environment cubemap;
- irradiance map;
- prefiltered environment map;
- BRDF integration LUT.

---

## Feature Summary

### Rendering

- OpenGL 4.6 core profile
- Deferred shading
- Physically based rendering
- Image-based lighting
- Directional / point / spot lighting
- Directional and cubemap shadow maps
- SSAO
- Screen-space reflections
- Rain / wet-floor material effect

### Post-Processing

- HDR rendering
- Reinhard tone mapping
- ACES tone mapping
- AgX
- AgX Punchy
- Experimental automatic exposure / eye adaptation
- Gaussian bloom
- Physically based bloom path
- Bilateral filtering
- FXAA

### Tooling / Interaction

- Dear ImGui runtime editor
- FPS display
- Camera navigation
- Runtime lighting and post-processing controls
- Optional Doxygen generation

---

## Project Layout

```text
assets/
├─ models/                 Sponza model and PBR textures
├─ shaders/
│  ├─ bake/                IBL and shadow-map baking
│  ├─ deferred_shading/    G-buffer, SSAO, lighting, SSR
│  ├─ pbr/                 PBR shaders
│  ├─ post-processing/     HDR, bloom, filters, FXAA
│  └─ shadow-lighting/     Shadow-aware lighting shaders
├─ textures/
└─ ...

docs/screenshots/          README / portfolio screenshots

src/
├─ Source.cpp              Main application and renderer orchestration
├─ imgui/                  Bundled Dear ImGui sources/backends
└─ learnopengl/            Camera, model/mesh loading, DDS, bloom, and helper code
```

---

## Dependencies

The project uses **Conan 2** for dependency resolution and CMake integration.

- CMake 3.27+
- Conan 2
- Ninja
- A C++23-capable compiler
- OpenGL 4.6-capable GPU and driver
- GLAD
- GLFW
- GLM
- stb
- Assimp
- Dear ImGui — bundled under `src/imgui`
- Doxygen — optional

The Conan recipe currently requests:

```text
glad/0.1.36
glfw/3.4
glm/1.0.1
stb/cci.20230920
assimp/5.4.3
opengl/system
```

### Tested build environment

A clean-clone build was verified with:

```text
Windows 11
Conan 2.32.0
MinGW GCC 16.2.0
Ninja
CMake generated through the Conan toolchain workflow
Release configuration
```

Other toolchains may work, but they have not been verified as part of this repository cleanup.

---

## Build

This project relies on the **Conan-generated CMake presets** produced by `CMakeToolchain` and `cmake_layout()`.

The repository therefore does not require a project-specific Conan profile name such as `gcc-default`.

### 1. Prepare a local Conan profile

If this is your first Conan 2 setup on the machine:

```powershell
conan profile detect
```

If your local compiler/toolchain changed since the profile was created, regenerate it:

```powershell
conan profile detect --force
```

The Conan profile belongs to the local machine and is intentionally **not committed to this repository**.

You can inspect the detected profile with:

```powershell
conan profile show
```

### 2. Install dependencies and generate the CMake presets

Release:

```powershell
conan install . --build=missing -s build_type=Release -s compiler.cppstd=23
```

Debug:

```powershell
conan install . --build=missing -s build_type=Debug -s compiler.cppstd=23
```

Conan generates `CMakeUserPresets.json` and the corresponding build-tree preset files. These generated files are ignored by Git.

### 3. Configure and build

Release:

```powershell
cmake --preset conan-release
cmake --build --preset conan-release
```

Debug:

```powershell
cmake --preset conan-debug
cmake --build --preset conan-debug
```

### Why no committed `CMakePresets.json`?

The previous project preset duplicated Conan's generated preset workflow and referenced a toolchain path that no longer matched the output produced by `cmake_layout()`.

Using Conan's generated `conan-release` / `conan-debug` presets keeps the build directory and toolchain path consistent with the active Conan version and local profile.

---

## Run

For a Release build, the executable is generated under:

```text
build/Release/bin/
```

Run:

```powershell
.\build\Release\bin\OpenGL_Course_Project.exe
```

The build also copies the runtime assets into the generated binary directory.

The default window size is currently fixed at **2560 × 1440** in `src/Source.cpp`.

---

## Controls

| Input | Action |
|---|---|
| `W` `A` `S` `D` | Move camera |
| `Space` | Move up |
| `Left Shift` | Move down |
| Mouse | Look around while cursor is captured |
| Mouse wheel | Adjust zoom |
| `K` | Toggle cursor capture / editor interaction |
| `V` | Toggle swap interval setting |
| `E` | Toggle automatic eye adaptation |
| `G` / `H` | Decrease / increase height-mapping scale |
| `Up` / `Down` | Switch baked IBL environment map |
| `Esc` | Exit |

---

## Learning and Ownership Notes

For clarity:

- The project was built **after and on top of my LearnOpenGL learning work**.
- I do not claim tutorial-covered rendering algorithms as original work.
- The value of this repository is the transition from isolated tutorial exercises to a larger integrated renderer that I could modify and experiment with.
- Project-specific work visible in this repository includes the SSR path, rain / wet-floor effect, expanded tone mapping / automatic exposure, runtime editor integration, and broader renderer / shadow-light integration.

For graduate-application use, this is best described as:

> **An independent graphics extension and integration project built on LearnOpenGL foundations.**

---

## Known Limitations

- The default render resolution is fixed in source code.
- The automatic-exposure experiment performs a CPU readback from the lowest mip level and is not an optimized implementation.
- The project is primarily a learning / experimentation renderer rather than a production engine.
- The clean build was verified with the Windows + MinGW GCC toolchain listed above; other configurations remain unverified.

---

## Third-Party Material

This repository contains or depends on third-party libraries, helper code, models, textures, and other assets. Their original licenses and attribution requirements remain applicable.

Before redistributing the repository as a packaged release, the third-party asset/code licenses should be reviewed and documented explicitly.
