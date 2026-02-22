# CS 330 Final Project – Reflection

## 1. Justify Development Choices for Your 3D Scene

**Why I chose these objects.**  
The scene is built around a 3D version of a 2D reference image (a watering can). I kept the **watering can** as the main object and built the rest to support it and meet the assignment requirements:

- **Ground plane** – A textured floor (pavers) so the scene has a clear “ground” and the can and other objects sit in a consistent space. It uses a custom plane (VAO/VBO) so the floor can be scaled and textured independently.
- **Watering can** – Built from **multiple primitives** (cylinder body, cone spout, box bracket, cylinder handle) so it counts as one composite object. It uses two textures (metal for body/spout/bracket, wood for handle) for clear material variation.
- **Pyramid** – A simple accent (e.g., planter) that uses the pyramid primitive and adds variety in shape and position (left side of the scene).
- **Green cube** – A box primitive to the right of the pyramid to balance the layout and to use the required “box” shape in an obvious way.
- **Transparent water glass** – A cylinder with alpha &lt; 1 and blending enabled, placed to the left of the pyramid. It demonstrates transparency and uses the same Phong lighting as the rest of the scene.

**How I programmed for the required functionality.**

- **Low-poly objects and primitives:** All objects use the existing `ShapeMeshes` (box, cylinder, cone, pyramid) or a simple custom plane. Triangle counts stay low; no object approaches 1,000 triangles.
- **Textures:** The floor and watering can use textures loaded via `CreateGLTexture` and `TryLoadTexture`, with UV scaling where needed. Textures are 1024×1024 or similar and applied with the shared shader pipeline.
- **Lighting:** Two point lights (key and fill) with pink/mauve tint are set in `SetSceneLights()`. The fragment shader implements the full Phong model (ambient, diffuse, specular) so every object is lit consistently.
- **Placement:** Each object has explicit scale, rotation, and position (X, Y, Z). Positions were chosen so the can is near the center, the pyramid and glass on the left, and the green cube on the right, matching the idea of the 2D layout while keeping everything visible from the default camera.

---

## 2. How a User Can Navigate the 3D Scene (Camera and Input Devices)

**Navigation and camera control** are implemented in `ViewManager` and the Camera class.

**Keyboard – movement (horizontal, vertical, depth):**

- **W / S** – Move the camera forward / backward (change distance to the orbit target). This gives depth motion (closer/farther).
- **A / D** – Pan left / right (move the look-at target horizontally). This gives horizontal motion.
- **Q / E** – Move the target down / up (vertical motion).

So the user can traverse all three axes: X (A/D), Y (Q/E), and Z/depth (W/S).

**Mouse – orientation (pitch and yaw):**

- The **mouse cursor** is captured (cursor disabled). Moving the mouse updates the camera’s yaw and pitch via `ProcessMouseMovement`, so the user can look left/right and up/down without changing the camera’s position. This gives full control over viewing direction.

**Mouse scroll – speed:**

- **Scroll wheel** adjusts the camera’s movement speed (`MovementSpeed`). Scroll up increases speed, scroll down decreases it, clamped between 1 and 20 so the scene stays controllable.

**Keyboard – projection (perspective vs. orthographic):**

- **P** – Switch to **perspective** projection (3D view, 45° FOV). If the user was in orthographic mode, the previous camera position and orientation are restored so they return to the same view.
- **O** – Switch to **orthographic** projection (2D-like top-down view). The camera is moved to a fixed position above the scene looking straight down. The current perspective view is saved so pressing **P** later restores it.

**Setup details:**

- In `CreateDisplayWindow`, the cursor is set to `GLFW_CURSOR_DISABLED` so all mouse motion is used for look.
- `glfwSetCursorPosCallback` is used for mouse position (pitch/yaw).
- `glfwSetScrollCallback` is used for scroll (speed).
- Every frame, `processInput()` reads the keyboard and updates the camera; `PrepareSceneView()` then uses the camera’s view matrix and the chosen projection (perspective or orthographic) and passes them to the shader. So the virtual camera is fully driven by these input devices.

---

## 3. Custom Functions for Modularity and Organization

The project uses several custom functions so that rendering and setup are reusable and easy to change.

**SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ)**  
*What it does:* Builds a single model matrix from scale, rotations around X/Y/Z (in degrees), and translation, then uploads it to the shader as the `model` uniform.  
*Reusable:* Every object that needs to be placed in the scene can call this once with its own scale, rotation, and position. The same function is used for the ground plane, pyramid, green cube, and glass; only the parameters change. This avoids repeating matrix math and keeps rendering code consistent.

**SetShaderMaterial(materialTag)**  
*What it does:* Looks up a material by name (e.g., `"floor"`, `"metal"`, `"glass"`) in the scene’s material list and sends its ambient strength, ambient/diffuse/specular colors, and shininess to the shader.  
*Reusable:* Any number of objects can share the same material by tag. Adding a new material (e.g., `"greenbox"`, `"glass"`) only requires defining it once in `PrepareScene()`; then any mesh can use it with one call. This keeps lighting behavior consistent and avoids duplicating material data.

**SetShaderColor(red, green, blue, alpha)**  
*What it does:* Turns off texture usage in the shader and sets the `objectColor` uniform. Used for untextured or transparent objects (e.g., pyramid, green cube, glass).  
*Reusable:* Any object that is drawn with a solid or transparent color can call this before drawing. The same pipeline is used for opaque and transparent objects; only the color (and alpha for the glass) changes.

**SetShaderTexture(textureTag)**  
*What it does:* Finds the texture slot for the given tag and sets the shader to use that texture for the next draw.  
*Reusable:* Multiple objects can use the same texture (e.g., `"metal"` on body, spout, and bracket), and new textures are added once via `CreateGLTexture`/`TryLoadTexture` and then referenced by tag everywhere.

**SetSceneLights()**  
*What it does:* Sets all Phong light source uniforms (position, ambient, diffuse, specular, focal strength, specular intensity) for the key and fill lights, and zeros out unused light slots.  
*Reusable:* Called once per frame in `RenderScene()`. Changing the number of lights, their positions, or colors only requires editing this function; all lit objects automatically use the new setup. No per-object light code is needed.

**TryLoadTexture(primaryPath, fallbackPath, tag)**  
*What it does:* Tries to load a texture from `primaryPath` (e.g., next to the executable); if that fails, tries `fallbackPath` (e.g., project directory). Registers the texture under `tag` on success.  
*Reusable:* The same call works whether the app is run from the build output folder or the project folder. Every texture load uses this so the project is robust to different run locations.

**RenderComplexObject()**  
*What it does:* Draws the entire watering can by setting the model matrix, material, and texture for each part (body cylinder, cone spout, box bracket, handle cylinder) and calling the appropriate `Draw*Mesh` for each.  
*Reusable:* The main render loop only calls `RenderComplexObject()` once; all watering-can logic is in one place. To add or change a part (e.g., another cylinder or different scale), only this function is edited. This keeps `RenderScene()` short and makes composite objects easy to maintain.

**CreateGroundPlaneMesh()** and **DestroyGroundPlaneMesh()**  
*What they do:* `CreateGroundPlaneMesh()` builds a custom plane VAO/VBO (positions, normals, UVs) for the floor. `DestroyGroundPlaneMesh()` frees those buffers (e.g., in the destructor).  
*Reusable:* The ground is a special case (custom geometry, not from `ShapeMeshes`). Isolating creation and destruction in these two functions keeps buffer management in one place and avoids leaking VAOs/VBOs if the scene is reinitialized. Other custom meshes could follow the same pattern.

Together, these functions make the code modular: adding a new object usually means calling `SetTransformations`, `SetShaderMaterial`, and either `SetShaderColor` or `SetShaderTexture`, then one draw call. Changing lighting, materials, or projection is done in a few central places instead of scattered across the whole scene.
