#pragma once 
#include <Renderer/Vulkan/vk_types.h>

namespace VkUtil {
	bool LoadShaderModule(const char* filePath, VkDevice device, VkShaderModule* outShaderModule);
};

class PipelineBuilder {
public:
    std::vector<VkPipelineShaderStageCreateInfo> ShaderStages;

    VkPipelineInputAssemblyStateCreateInfo InputAssembly;
    VkPipelineRasterizationStateCreateInfo Rasterizer;
    VkPipelineColorBlendAttachmentState ColorBlendAttachment;
    VkPipelineMultisampleStateCreateInfo Multisampling;
    VkPipelineLayout PipelineLayout;
    VkPipelineDepthStencilStateCreateInfo DepthStencil;
    VkPipelineRenderingCreateInfo RenderInfo;
    VkFormat ColorAttachmentFormat;

    PipelineBuilder() { Clear(); }

    void Clear();

    VkPipeline BuildPipeline(VkDevice device) const;
    void SetShaders(VkShaderModule vertexShader, VkShaderModule fragmentShader);
    void SetInputTopology(VkPrimitiveTopology topology);
    void SetPolygonMode(VkPolygonMode mode);
    void SetCullMode(VkCullModeFlags cullMode, VkFrontFace frontFace);
    void SetMultisamplingNone();
    void DisableBlending();
    void SetColorAttachmentFormat(VkFormat format);
    void SetDepthFormat(VkFormat format);
    void DisableDepthTest();
    void EnableDepthTest(VkBool32 depthTestEnable, VkBool32 depthWriteEnable, VkCompareOp op);
};