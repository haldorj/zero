#include "VulkanBuffer.h"

AllocatedBuffer VulkanBufferManager::CreateBuffer(VmaAllocator& allocator, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
{
    // allocate buffer
    VkBufferCreateInfo bufferInfo = { .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufferInfo.pNext = nullptr;
    bufferInfo.size = allocSize;

    bufferInfo.usage = usage;

    VmaAllocationCreateInfo vmaallocInfo = {};
    vmaallocInfo.usage = memoryUsage;
    vmaallocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
    AllocatedBuffer newBuffer;

    // allocate the buffer
    VK_CHECK(vmaCreateBuffer(allocator, &bufferInfo, &vmaallocInfo, &newBuffer.Buffer, &newBuffer.Allocation,
        &newBuffer.Info));

    return newBuffer;
}

void VulkanBufferManager::DestroyBuffer(VmaAllocator& allocator, const AllocatedBuffer& buffer)
{
    vmaDestroyBuffer(allocator, buffer.Buffer, buffer.Allocation);
}