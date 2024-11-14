#pragma once
#include <shared/vk_types.h>

class VulkanBufferManager
{
public:
	static AllocatedBuffer CreateBuffer(VmaAllocator& allocator, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
	static void DestroyBuffer(VmaAllocator& allocator, const AllocatedBuffer& buffer);
};
