# About

Zero is a 3D rendering/game project built with C++ with rendering backends for OpenGL and Vulkan.

The initial motivation for starting the project was to learn the Vulkan API by implementing a Vulkan renderer alongside an OpenGL renderer.

Below is a video showcasing the development of the project, and some of the main features that were implimented.

https://github.com/user-attachments/assets/40a32017-76d1-4368-88ec-305903ebf882

# Build

Requires the Vulkan SDK (minimum ver. 1.3).

## Project:

Start by cloning the repository with git clone ```--recursive https://github.com/haldorj/zero```.

If the repository was cloned non-recursively previously, use ```git submodule update --init``` to clone the necessary submodules.

Run ```GenerateProjects.bat```

# Compiling shaders:

If you have made changes to Vulkan shaders, you will have to run ```CompileShaders.bat``` to compile to the intermediate SPIR-V format.

# Features

## Model Loading

Support for 3D model loading using assimp.

## Skeletal Animation

Support for skeletal animation. 
Bones and animations are loaded using assimp.

## Directional Shadow mapping 

Directional shadowmapping with PCF filtering.

Depth buffer debugging.

## Lights

Lighting is currently done using the phong shading model.

The project has implementations for directional lights, point lights and spot lights.

## Skybox

Rendering a skybox using a cubemap texture. 

## Physics

The beginnings of a Physics engine. 

Collision detection and physics solvers.

## Character controller

Simple character controller with an animated character utilizing different animations. 

## Camera System

Editor and perspective camera implementations.

## Rendering:

### Vulkan

**1.3 features:**
Dynamic Rendering and Synchronization 2.

**1.2 features:** 
Buffer Device Address, Descriptor Indexing and Scalar Block Layout.

### OpenGL

Using OpenGL ver. 4.6 with glad.

### Glsl

Both renderers are using glsl ver. 4.5. The Vulkan renderer requires pre-compiled SPIR-V files generated from the corresponding glsl files.

# Dependencies

**Assimp:** model loading.

**Glad:** OpenGL support.

**GLFW:** window and input.

**GLM:** math.

**Dear ImGui:** UI debugging and tools.

**STB Image:** texture/image loading.

**Vk-Bootstrap:** for Vulkan setup.

**VMA:** Vulkan memory allocation.
