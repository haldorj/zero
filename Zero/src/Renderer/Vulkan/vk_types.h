// vulkan_guide.h : Include file for standard system include files,
// or project specific include files.
#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <span>
#include <array>
#include <functional>
#include <deque>

#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>
#include <vk_mem_alloc.h>

// #include <fmt/core.h>

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

#include <Renderer/OpenGL/OpenGLUtils.h>

#define VK_CHECK(x)                                                     \
    do {                                                                \
        VkResult err = x;                                               \
        if (err) {                                                      \
            printf ("Detected Vulkan error: %s", string_VkResult(err)); \
            abort();                                                    \
        }                                                               \
    } while (0)

struct AllocatedBuffer {
    VkBuffer Buffer;
    VmaAllocation Allocation;
    VmaAllocationInfo Info;
};

// holds the resources needed for a mesh
struct GPUMeshBuffers {
    AllocatedBuffer IndexBuffer;
    AllocatedBuffer VertexBuffer;
    VkDeviceAddress VertexBufferAddress;
};

struct AllocatedImage
{
    VkImage Image{};
    VkImageView ImageView{};
    VmaAllocation Allocation{};
    VkExtent3D ImageExtent{};
    VkFormat ImageFormat{};
};

// push constants for our mesh object draws
struct GPUDrawPushConstants 
{
    glm::mat4 ModelMatrix;
    glm::vec3 CameraPos;
    //int Animated;
    //std::vector<glm::mat4> BoneMatrices;
    VkDeviceAddress VertexBuffer;
};

struct LightData
{
    glm::vec3 Color;
    float AmbientIntensity;
    float DiffuseIntensity;
};

struct DirectionalLightData
{
    LightData Base;
    glm::vec3 Direction;
};

struct PointLightData
{
    LightData Base;
    glm::vec3 Position;
    float Constant;
    float Linear;
    float Exponent;
};

struct SpotLightData
{
    PointLightData Base;
	glm::vec3 Direction;
	float Edge;
};

struct MaterialData
{
    float SpecularIntensity;
    float Shininess;
};

struct GPUSceneData
{
    glm::mat4 View;
    glm::mat4 Proj;
    glm::mat4 Viewproj;

    int PointLightCount;
    int SpotLightCount;

    DirectionalLightData DirectionalLight;
    PointLightData PointLights[MAX_POINT_LIGHTS];
	SpotLightData SpotLights[MAX_SPOT_LIGHTS];
    MaterialData Material;
};

struct GPUObjectData
{
	int Animated{ 0 };
	// std::vector<glm::mat4> BoneMatrices; // max 100 bones
};
