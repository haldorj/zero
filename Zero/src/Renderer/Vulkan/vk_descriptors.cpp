#include <Renderer/Vulkan/vk_descriptors.h>

void DescriptorLayoutBuilder::AddBinding(uint32_t binding, VkDescriptorType type)
{
    // Create a new descriptor set layout binding
    VkDescriptorSetLayoutBinding descriptorSetLayoutBinding{};
    descriptorSetLayoutBinding.binding = binding;
    descriptorSetLayoutBinding.descriptorCount = 1;
    descriptorSetLayoutBinding.descriptorType = type;

    // Add the new binding to the list of bindings
    Bindings.push_back(descriptorSetLayoutBinding);
}

void DescriptorLayoutBuilder::Clear()
{
    // Clear the list of bindings
    Bindings.clear();
}

VkDescriptorSetLayout DescriptorLayoutBuilder::Build(VkDevice device, VkShaderStageFlags shaderStages, void* pNext, VkDescriptorSetLayoutCreateFlags flags)
{
    // Update the shader stages for each binding
    for (auto& b : Bindings) {
        b.stageFlags |= shaderStages;
    }

    // Create the descriptor set layout create info
    VkDescriptorSetLayoutCreateInfo info = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
    info.pNext = pNext;

    info.pBindings = Bindings.data();
    info.bindingCount = static_cast<uint32_t>(Bindings.size());
    info.flags = flags;

    VkDescriptorSetLayout set;
    // Create the descriptor set layout
    VK_CHECK(vkCreateDescriptorSetLayout(device, &info, nullptr, &set));

    return set;
}

void DescriptorAllocator::InitPool(VkDevice device, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios)
{
    std::vector<VkDescriptorPoolSize> poolSizes;
    for (PoolSizeRatio ratio : poolRatios) {
        poolSizes.push_back(VkDescriptorPoolSize{
            .type = ratio.Type,
            .descriptorCount = static_cast<uint32_t>(ratio.Ratio) * maxSets
            });
    }

    VkDescriptorPoolCreateInfo pool_info = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
    pool_info.flags = 0;
    pool_info.maxSets = maxSets;
    pool_info.poolSizeCount = (uint32_t)poolSizes.size();
    pool_info.pPoolSizes = poolSizes.data();

    // Create the descriptor pool
    vkCreateDescriptorPool(device, &pool_info, nullptr, &Pool);
}

void DescriptorAllocator::ClearDescriptors(VkDevice device)
{
    // Reset the descriptor pool
    vkResetDescriptorPool(device, Pool, 0);
}

void DescriptorAllocator::DestroyPool(VkDevice device)
{
    // Destroy the descriptor pool
    vkDestroyDescriptorPool(device, Pool, nullptr);
}

VkDescriptorSet DescriptorAllocator::allocate(VkDevice device, VkDescriptorSetLayout layout)
{
    VkDescriptorSetAllocateInfo allocInfo = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
    allocInfo.pNext = nullptr;
    allocInfo.descriptorPool = Pool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &layout;

    VkDescriptorSet ds;
    // Allocate a descriptor set from the pool
    VK_CHECK(vkAllocateDescriptorSets(device, &allocInfo, &ds));

    return ds;
}

VkDescriptorPool DescriptorAllocatorGrowable::GetPool(VkDevice device)
{
    VkDescriptorPool newPool;
    if (m_ReadyPools.size() != 0) {
        newPool = m_ReadyPools.back();
        m_ReadyPools.pop_back();
    }
    else {
        //need to create a new pool
        newPool = CreatePool(device, m_SetsPerPool, m_Ratios);

        m_SetsPerPool = static_cast<uint32_t>(m_SetsPerPool * 1.5);
        if (m_SetsPerPool > 4092) {
            m_SetsPerPool = 4092;
        }
    }

    return newPool;
}

VkDescriptorPool DescriptorAllocatorGrowable::CreatePool(VkDevice device, uint32_t setCount, std::span<PoolSizeRatio> poolRatios)
{
    std::vector<VkDescriptorPoolSize> poolSizes;
    for (PoolSizeRatio ratio : poolRatios) {
        poolSizes.push_back(VkDescriptorPoolSize{
            .type = ratio.Type,
            .descriptorCount = static_cast<uint32_t>(ratio.Ratio) * setCount
            });
    }

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = 0;
    pool_info.maxSets = setCount;
    pool_info.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    pool_info.pPoolSizes = poolSizes.data();

    VkDescriptorPool newPool;
    vkCreateDescriptorPool(device, &pool_info, nullptr, &newPool);
    return newPool;
}

void DescriptorAllocatorGrowable::Init(VkDevice device, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios)
{
    m_Ratios.clear();

    for (auto r : poolRatios) {
        m_Ratios.push_back(r);
    }

    VkDescriptorPool newPool = CreatePool(device, maxSets, poolRatios);

    m_SetsPerPool = static_cast<uint32_t>(maxSets * 1.5); //grow it next allocation

    m_ReadyPools.push_back(newPool);
}

void DescriptorAllocatorGrowable::ClearPools(VkDevice device)
{
    for (auto p : m_ReadyPools) {
        vkResetDescriptorPool(device, p, 0);
    }
    for (auto p : m_FullPools) {
        vkResetDescriptorPool(device, p, 0);
        m_ReadyPools.push_back(p);
    }
    m_FullPools.clear();
}

void DescriptorAllocatorGrowable::DestroyPools(VkDevice device)
{
    for (auto p : m_ReadyPools) {
        vkDestroyDescriptorPool(device, p, nullptr);
    }
    m_ReadyPools.clear();
    for (auto p : m_FullPools) {
        vkDestroyDescriptorPool(device, p, nullptr);
    }
    m_FullPools.clear();
}

VkDescriptorSet DescriptorAllocatorGrowable::Allocate(VkDevice device, VkDescriptorSetLayout layout, void* pNext)
{
    //get or create a pool to allocate from
    VkDescriptorPool poolToUse = GetPool(device);

    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.pNext = pNext;
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = poolToUse;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &layout;

    VkDescriptorSet ds;
    VkResult result = vkAllocateDescriptorSets(device, &allocInfo, &ds);

    //allocation failed. Try again
    if (result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL) {

        m_FullPools.push_back(poolToUse);

        poolToUse = GetPool(device);
        allocInfo.descriptorPool = poolToUse;

        VK_CHECK(vkAllocateDescriptorSets(device, &allocInfo, &ds));
    }

    m_ReadyPools.push_back(poolToUse);
    return ds;
}

void DescriptorWriter::WriteBuffer(int binding, VkBuffer buffer, size_t size, size_t offset, VkDescriptorType type)
{
    VkDescriptorBufferInfo& info = BufferInfos.emplace_back(VkDescriptorBufferInfo{
        .buffer = buffer,
        .offset = offset,
        .range = size
        });

    VkWriteDescriptorSet write = { .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };

    write.dstBinding = binding;
    write.dstSet = VK_NULL_HANDLE; //left empty for now until we need to write it
    write.descriptorCount = 1;
    write.descriptorType = type;
    write.pBufferInfo = &info;

    Writes.push_back(write);
}

void DescriptorWriter::WriteImage(int binding, VkImageView image, VkSampler sampler, VkImageLayout layout, VkDescriptorType type)
{
    VkDescriptorImageInfo& info = ImageInfos.emplace_back(VkDescriptorImageInfo{
        .sampler = sampler,
        .imageView = image,
        .imageLayout = layout
        });

    VkWriteDescriptorSet write = { .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };

    write.dstBinding = binding;
    write.dstSet = VK_NULL_HANDLE; //left empty for now until we need to write it
    write.descriptorCount = 1;
    write.descriptorType = type;
    write.pImageInfo = &info;

    Writes.push_back(write);
}

void DescriptorWriter::Clear()
{
    ImageInfos.clear();
    Writes.clear();
    BufferInfos.clear();
}

void DescriptorWriter::UpdateSet(VkDevice device, VkDescriptorSet set)
{
    if (Writes.empty()) {
        printf("DescriptorWriter::UpdateSet called with no writes\n");
        return;
    }

    for (VkWriteDescriptorSet& write : Writes) {
        write.dstSet = set;
    }

    vkUpdateDescriptorSets(device, (uint32_t)Writes.size(), Writes.data(), 0, nullptr);
}
