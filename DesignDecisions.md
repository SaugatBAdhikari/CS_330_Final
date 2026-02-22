# Design Decisions Document  
## CS 330 Final Project – 3D Scene

This document describes the design rationale and key technical decisions for the 3D watering can scene.

---

## 1. Object and Scene Design

**Reference and goals.**  
The scene is based on a 2D reference image of a watering can. The goal was to create a low-poly 3D approximation suitable for a client concept (e.g., 3D printing), using basic shapes and a clear layout.

**Object choices.**

| Object        | Purpose | Primitives / approach |
|---------------|---------|------------------------|
| **Ground plane** | Floor for the scene; supports all other objects | Custom plane (VAO/VBO), textured (pavers) |
| **Watering can** | Main object from the 2D image | Composite: cylinder (body), cone (spout), box (bracket), cylinder (handle) |
| **Pyramid**   | Accent; uses pyramid primitive | Single pyramid mesh, matte material |
| **Green cube** | Balance and use of box primitive | Single box mesh, green material |
| **Water glass** | Transparency and variety | Cylinder with alpha blending, glass material |

**Placement.**  
Objects are positioned along X, Y, and Z so the watering can is near the center, with the pyramid and glass to the left and the green cube to the right. This keeps the layout readable and ensures everything is visible from the default camera and under both point lights.

---

## 2. Textures and Materials

- **Textured objects:** Ground (pavers) and watering can (metal for body/spout/bracket, wood for handle). Textures are loaded once and referenced by tag; UV scaling is used where needed (e.g., floor tiling).
- **Untextured objects:** Pyramid, green cube, and glass use solid colors and Phong materials (ambient, diffuse, specular, shininess) for a consistent lit look. The glass uses alpha &lt; 1 and blending for transparency.

---

## 3. Lighting

- **Two point lights:** A key light (pink-tinted) and a fill light (softer mauve) so no part of the scene is in complete shadow from any camera angle.
- **Phong model:** Ambient, diffuse, and specular components are implemented in the fragment shader for all objects. Materials are defined by tag (e.g., metal, wood, glass) and applied in a single pipeline.

---

## 4. Camera and Navigation

- **Orbit-style camera:** The camera orbits a target point. W/S change distance (depth); A/D pan the target horizontally; Q/E move the target vertically. Mouse controls pitch and yaw; scroll adjusts movement speed.
- **Projection toggle:** P switches to perspective (3D); O switches to orthographic (top-down). Camera state is saved and restored when switching so the user can return to the same view.

---

## 5. Code Organization and Reuse

- **Transforms:** `SetTransformations()` builds and uploads the model matrix from scale, rotation, and position so every object uses the same pipeline.
- **Materials and textures:** `SetShaderMaterial(tag)` and `SetShaderTexture(tag)` look up by name, so materials and textures are defined once and reused.
- **Lights:** `SetSceneLights()` sets all light uniforms once per frame so lighting changes in one place affect the whole scene.
- **Composite object:** `RenderComplexObject()` draws the entire watering can (all parts and materials) so the main render loop stays simple and the composite is easy to change.
- **Resource loading:** `TryLoadTexture(primaryPath, fallbackPath, tag)` tries exe-relative then project-relative paths so the app runs from different working directories.

These decisions keep the scene maintainable, meet the assignment requirements (primitives, textures, lighting, navigation, projection), and demonstrate a structured approach to building a small 3D world.
