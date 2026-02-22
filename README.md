# CS 330 Final Project (Watering Can)

3D OpenGL scene built for SNHU CS-330 Computational Graphics and Visualization. Includes a textured watering can (composite object), ground plane, pyramid, green cube, and transparent water glass, with Phong lighting and camera navigation.

**Tech:** C++, OpenGL, GLFW, GLEW, GLM. Built with Visual Studio.

---

## CS 330 Module Eight Journal – Portfolio Submission

This repository is structured to meet the Module Eight Journal requirements. The two components of the project are submitted here for the portfolio:

| Required component | Location in this repo |
|--------------------|------------------------|
| **3D Scene ZIP folder** | [`3D_Scene_ZIP/`](3D_Scene_ZIP/) – Contains the 3D Scene ZIP file for portfolio submission. The full 3D scene source is also in this repo: `Source/`, `shaders/`, `textures/`, solution and project files. |
| **Design Decisions document** | [`DesignDecisions.md`](DesignDecisions.md) – Design rationale and technical decisions for the 3D scene. |
| **README with written reflection** | This file – Written answers to the journal reflection questions appear in the **Portfolio Reflection** section below. |

### What to submit

- **Submission:** Include a **link to this GitHub repository** in your journal submission for your instructor.
- **Repository must include:** (1) 3D Scene ZIP folder, (2) Design Decisions document, (3) README file with your written response (reflection).
- **Note:** Add your instructor as a collaborator to this repository (they should have accepted in the previous module). Email your instructor if you have trouble adding them.

## Run

- Open `7-1_FinalProjectMilestones.sln` in Visual Studio
- Build and run (F5)
- Ensure `shaders/` and `textures/` are present (post-build copies them to the output folder)

## Controls

| Input | Action |
|-------|--------|
| **W / S** | Forward / backward (depth) |
| **A / D** | Pan left / right |
| **Q / E** | Move down / up |
| **Mouse** | Look around (pitch and yaw; cursor captured) |
| **Scroll** | Increase / decrease movement speed |
| **P** | Perspective (3D) view |
| **O** | Orthographic (top-down) view |
| **Esc** | Close window |

## Scene

- **Ground** – Textured floor (pavers)
- **Watering can** – Composite (cylinder, cone, box, cylinder) with metal and wood textures
- **Pyramid** – Left of center
- **Green cube** – Right of pyramid
- **Transparent glass** – Left of pyramid (cylinder with alpha blending)

## Notes

- Shaders: `shaders/` (vertex + fragment); copied to build output via post-build step
- Textures: `textures/` (floor, metal, wood); same post-build copy
- Technical reflection: `Reflection.md` | Design rationale: `DesignDecisions.md`

---

## Portfolio Reflection

*Written response for the Module Eight Journal. Answers address each question below, using the sub-bullets to guide the reflection.*

---

### How do I approach designing software?

I start by understanding the goal (here, a 3D version of a 2D image) and the constraints (low-poly, specific primitives, textures, lighting). I break the scene into objects and decide which primitives map to each part—for example, the watering can as a composite of cylinder, cone, and box. I also think about how the user will interact with the result (camera, controls) so the design supports both the visual result and the interaction.

**What new design skills has your work on the project helped you to craft?**  
I got better at translating a 2D reference into a minimal 3D representation: choosing which shapes to use, where to put textures versus solid materials, and how to place objects in 3D space so the composition stays clear. I also thought more deliberately about reusability—shared materials, a single transform pipeline, and a dedicated function for the composite object so the design stays organized as the scene grows.

**What design process did you follow for your project work?**  
I followed a milestone-based process: get one object and the camera working, then add lighting, then textures, then more objects and refinement. For each step I checked the rubric (primitives, lights, textures, navigation) and adjusted the design so the code and the scene met those requirements without overcomplicating things.

**How could tactics from your design approach be applied in future work?**  
I would reuse: (1) breaking a problem into small, testable steps, (2) defining materials and resources by name/tag so they can be reused across objects, and (3) keeping a single place for transforms, lights, and drawing logic so changes are localized and the design stays understandable.

---

### How do I approach developing programs?

I develop incrementally: get something running (e.g., one shape on screen), then add features one at a time (lighting, textures, more objects) and fix issues as they show up. I rely on the existing structure (SceneManager, ViewManager, shaders) and extend it with clear functions instead of duplicating logic.

**What new development strategies did you use while working on your 3D scene?**  
I used a tag-based system for textures and materials (e.g., `SetShaderMaterial("metal")`, `SetShaderTexture("wood")`) so the same asset could be applied to multiple meshes. I added `TryLoadTexture` with primary and fallback paths so the app runs from either the build output or the project directory. I also drew the transparent glass last so blending works correctly, which required thinking about draw order as part of the pipeline.

**How did iteration factor into your development?**  
Each milestone added one layer: first the watering can and camera, then the ground and lighting, then textures and more objects (pyramid, cube, glass). I iterated on positions and scales after seeing the scene in the viewport and adjusted the camera orbit and light positions so every object stayed visible and well lit.

**How has your approach to developing code evolved throughout the milestones, which led you to the project's completion?**  
Early on I focused on getting a single object and basic navigation working. As the scene grew, I put more logic into reusable helpers (`SetTransformations`, `SetShaderMaterial`, `RenderComplexObject`) so adding a new object became a short, repeatable block of code. By the end, the workflow was: add a material if needed, set transform and material, then draw—which made the final additions (pyramid, cube, glass) straightforward and kept the code maintainable.

---

### How can computer science help me in reaching my goals?

Computer science gives me a way to build systems that solve real problems and to keep learning as tools and domains change. This project combined math (transforms, lighting), APIs (OpenGL, GLFW), and design (scene composition, UX) in one place, which is the kind of integration I want in my career—whether in graphics, simulation, or other software roles.

**How do computational graphics and visualizations give you new knowledge and skills that can be applied in your future educational pathway?**  
Graphics forced me to think in 3D (coordinates, normals, view and projection matrices) and to connect math to what appears on screen. Those skills transfer to other areas that use linear algebra and spatial reasoning—e.g., machine learning, physics simulation, or human–computer interaction—and give me a concrete context for continuing to learn those topics in future courses.

**How do computational graphics and visualizations give you new knowledge and skills that can be applied in your future professional pathway?**  
I can now implement a simple 3D scene with lighting, textures, and camera control, and explain the pipeline (vertices → transforms → shaders → framebuffer). That is directly useful in game dev, visualization, or tools that use 3D. More broadly, the habit of breaking a visual goal into data structures, algorithms, and reusable code is something I can apply in any role that involves building interactive or visual software.

