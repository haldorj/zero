#pragma once

#include <shared/vk_types.h>
#include <unordered_map>
#include <filesystem>

struct GeoSurface {
    uint32_t StartIndex;
    uint32_t Count;
};

struct MeshAsset {
    std::string Name;

    std::vector<GeoSurface> Surfaces;
    GPUMeshBuffers MeshBuffers;
};

//forward declaration
class VulkanRenderer;