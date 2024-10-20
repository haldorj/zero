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


#define VK_CHECK(x)                                                     \
    do {                                                                \
        VkResult err = x;                                               \
        if (err) {                                                      \
            printf ("Detected Vulkan error: %s", string_VkResult(err)); \
            abort();                                                    \
        }                                                               \
    } while (0)

struct Vertex {
    glm::vec3 Position;
    float UvX;
    glm::vec3 Normal;
    float UvY;
    glm::vec4 Color;
};

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

// push constants for our mesh object draws
struct GPUDrawPushConstants {
    glm::mat4 WorldMatrix;
    VkDeviceAddress VertexBuffer;
};