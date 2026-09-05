# Third-Party Notices

This repository is a learning and experimentation renderer. It contains or derives from third-party code and assets that remain subject to their own licenses and attribution requirements.

This file documents known provenance. It does **not** relicense third-party material and does not grant rights beyond the corresponding upstream terms.

## LearnOpenGL-derived code and tutorial material

**Upstream:** LearnOpenGL / Joey de Vries  
**URL:** https://github.com/JoeyDeVries/LearnOpenGL  
**License:** Creative Commons Attribution-NonCommercial 4.0 International (CC BY-NC 4.0), unless explicitly stated otherwise upstream.

Portions of this project were developed from LearnOpenGL tutorial material and code samples. This includes code under `src/learnopengl/` and rendering/shader techniques that were initially implemented while following LearnOpenGL material.

The project-specific extensions described in the repository README should not be interpreted as a claim that tutorial-covered algorithms are original work.

Because LearnOpenGL-derived code is present, no blanket permissive license for the entire repository should be assumed.

## DDSLoader-derived / adapted code

**Historical upstream URL:** https://github.com/Hydroque/DDSLoader  
**Current upstream repository:** https://github.com/tilkinsc/DDSLoader  
**License:** MIT  
**Copyright:** Copyright (c) 2021 Cody Tilkins

The historical `Hydroque/DDSLoader` GitHub URL now redirects to the same GitHub repository, currently named `tilkinsc/DDSLoader`.

The DDS-loading implementation under:

- `src/learnopengl/ddsloader.cpp`
- `src/learnopengl/ddsloader.hpp`

was developed with that DDSLoader implementation and the other references listed in the source comments as implementation references. To preserve the upstream MIT notice for any copied or adapted portions, the upstream license text is included at:

`src/learnopengl/DDSLoader_LICENSE.txt`

## Dear ImGui

**Upstream:** Dear ImGui  
**URL:** https://github.com/ocornut/imgui  
**Bundled version:** 1.91.4  
**License:** MIT  
**Copyright:** Copyright (c) 2014-2024 Omar Cornut

Dear ImGui source and OpenGL/GLFW backends are bundled under `src/imgui/`.

The upstream MIT license text is included at:

`src/imgui/LICENSE.txt`

## Play font

**File:** `assets/fonts/Play-Regular.ttf`  
**Family:** Play  
**Source:** Google Fonts / Play  
**License:** SIL Open Font License 1.1 (OFL-1.1)  
**Copyright:** Copyright (c) 2011, Jonas Hecksher, Playtypes, e-types AS, with Reserved Font Names `Play`, `Playtype`, and `Playtype Sans`.

The OFL license text is included at:

`assets/fonts/OFL.txt`

## doxygen-awesome-css

**Upstream:** doxygen-awesome-css by jothepro  
**URL:** https://github.com/jothepro/doxygen-awesome-css  
**Integration:** Git submodule at `doxygen-awesome-css/`  
**License:** MIT  
**Copyright:** Copyright (c) 2021-2023 jothepro

The submodule retains its own upstream licensing information.

## Poly Haven assets

**Source:** https://polyhaven.com  
**License:** CC0

Poly Haven states that all downloadable assets on the site, including HDRIs, textures, and 3D models, are released under CC0 and may be redistributed.

Known Poly Haven material in this repository includes:

- HDR environment maps under `assets/textures/hdr/`;
- the six skybox cubemap images under `assets/cubemaps/skybox/`.

Two HDRIs used directly by the current renderer are:

- `assets/textures/hdr/evening_road_01_puresky_1k.hdr`
- `assets/textures/hdr/overcast_soil_puresky_2k.hdr`

The exact Poly Haven asset page from which the six standalone skybox PNG faces were derived was not retained. Their site-level origin is known, and Poly Haven's site-wide asset license is CC0.

## Sponza model and PBR textures

**Immediate source used for this project:**  
https://github.com/andrejnau/SponzaPbr

**Source path:**  
`assets/model/sponza_pbr/` in the upstream repository

The copy used in this project was subsequently modified in Blender by the author of this repository before being used here.

The `andrejnau/SponzaPbr` repository does not currently contain a root license file or an asset-specific license notice for its `sponza_pbr` directory. Its public availability on GitHub therefore does not, by itself, establish redistribution rights.

The underlying classic Sponza asset has a complicated licensing history. Public archives and sample repositories currently give conflicting or differently scoped licensing information for Crytek-derived Sponza versions.

For that reason, this repository records the exact immediate provenance but does **not** assign an unverified license to the redistributed Sponza model / texture bundle.

Blender modifications made in this project do not replace or override the rights that may apply to the upstream model and textures.

### Recommended long-term cleanup

For the cleanest public portfolio repository, replace the redistributed raw Sponza bundle with a version whose redistribution terms are explicit, then reapply any required Blender edits, or otherwise retain only rendered screenshots while documenting the historical immediate source.

## Conan-managed dependencies

The project currently declares the following dependencies through Conan:

- GLAD
- GLFW
- OpenGL system package
- GLM
- stb
- Assimp

These packages are resolved by Conan and remain subject to their respective upstream licenses.

## Repository license note

No blanket repository-level license should be interpreted as overriding any third-party license listed above.

If a license is later chosen for the project's original code, its scope should explicitly exclude third-party and derived material that remains under separate terms.
