# PBR-Sponza Third-Party / Asset License Audit — v2

**Audit date:** 2026-09-05  
**Repository:** `Doner357/PBR-Sponza`  
**Purpose:** Graduate-application portfolio repository hygiene and provenance cleanup.

> Practical repository-hygiene review; not legal advice.

## Updated result after source clarification

Three previously unresolved provenance questions were revisited:

| Item | New information | Status |
|---|---|---|
| Sponza PBR model/textures | Immediate source is `andrejnau/SponzaPbr`; local model was later modified in Blender | Provenance identified; redistribution license still unresolved |
| DDS loader | Historical `Hydroque/DDSLoader` URL redirects to the same GitHub repository now named `tilkinsc/DDSLoader` | **Resolved: MIT** |
| Skybox cubemap | Source site is Poly Haven | **Resolved: CC0**, exact asset page not retained |

## 1. Sponza PBR

Immediate upstream:

`https://github.com/andrejnau/SponzaPbr`

The upstream repository contains:

`assets/model/sponza_pbr/sponza.obj`  
`assets/model/sponza_pbr/sponza.mtl`  
`assets/model/sponza_pbr/textures/`

The repository README describes the renderer but does not provide an asset license, and the repository root currently has no `LICENSE` file.

The local project copy was modified in Blender. That is worth documenting for project ownership, but it does not erase upstream rights.

### Why this is still the only major unresolved licensing item

The classic Crytek Sponza ecosystem has conflicting historical license signals:

- Morgan McGuire's Computer Graphics Archive currently labels its Crytek Sponza distribution **CC BY 3.0**.
- Khronos' current glTF Sponza sample metadata labels the Crytek-derived model under the **CryEngine Limited License Agreement**.
- A Khronos licensing issue explicitly discusses this inconsistency.

Because the exact bundle used here came from an upstream GitHub repository that itself does not state a license, the safest record is:

> provenance known; exact redistribution grant not verified.

### Best cleanup options

1. **Best:** migrate to a Sponza source with explicit redistribution terms, then reapply the Blender modifications needed by this renderer.
2. Keep the current local asset only for private reproduction and omit the raw model/textures from the public repository.
3. If reliable historical license documentation for the exact `andrejnau/SponzaPbr` asset bundle is later recovered, preserve that documentation beside the asset.

For graduate applications, rendered screenshots and the renderer code remain valid evidence of the work even if the raw third-party model is removed from public redistribution.

## 2. DDS loader — resolved

The source comments referenced:

`https://github.com/Hydroque/DDSLoader`

That URL now returns a GitHub permanent repository redirect. The same repository ID currently resolves to:

`https://github.com/tilkinsc/DDSLoader`

The current repository contains an MIT license:

Copyright (c) 2021 Cody Tilkins.

Therefore the earlier concern that the DDSLoader source had disappeared was caused by a repository rename / transfer rather than disappearance.

### Action

Keep the current adapted loader if desired, but preserve the upstream MIT notice by adding:

`src/learnopengl/DDSLoader_LICENSE.txt`

and referencing it from `THIRD_PARTY_NOTICES.md`.

There is no longer a licensing reason to rewrite the loader solely because the old URL stopped resolving normally.

## 3. Skybox — resolved at license level

The six PNG cubemap faces came from Poly Haven.

Poly Haven's official license page states that **all site assets**—HDRIs, textures, and 3D models—are CC0 and may be redistributed.

Therefore the skybox is no longer a license blocker.

### Remaining provenance-quality note

The exact Poly Haven asset page/name is unknown. That is not a CC0 compliance problem, but recording the exact asset name would improve reproducibility if it can ever be recovered.

## 4. Other already-resolved items

- Dear ImGui 1.91.4 — MIT
- Play-Regular.ttf — SIL Open Font License 1.1
- doxygen-awesome-css — MIT
- Poly Haven HDRIs — CC0
- Conan-managed dependencies — upstream licenses apply

## 5. Recommended cleanup still worth doing

Separate from licensing, the repository can still be made smaller and cleaner:

- delete unused HDRIs, especially the ~91 MB `sponza.hdr` if it remains unused;
- remove legacy bundled DLLs if the executable still runs after a clean Conan build;
- remove unused demo textures;
- regenerate simple fallback textures (black / white / flat normal / error) procedurally or as self-created tiny images.

## 6. Current overall assessment

**Build reproducibility:** PASS  
**README / ownership clarity:** PASS  
**Dear ImGui / font / DDS loader / Poly Haven attribution:** can be closed with prepared notices  
**Sponza raw asset redistribution:** still the only significant unresolved provenance/license issue

This is sufficient to continue preparing the repository as a graduate-application portfolio project, but I would not apply a blanket MIT license to the whole repository.
