#include <Renderer/Vulkan/vk_pipelines.h>
#include <Renderer/Vulkan/vk_initializers.h>

#include <fstream>

bool VkUtil::LoadShaderModule(const char* filePath,
                                const VkDevice device,
                                VkShaderModule* outShaderModule)
{
    // open the file. With cursor at the end
    std::ifstream file(filePath, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        return false;
    }

    // find what the size of the file is by looking up the location of the cursor
    // because the cursor is at the end, it gives the size directly in bytes
    const size_t fileSize = file.tellg();

    // spir-v expects the buffer to be on uint32, so make sure to reserve a int
    // vector big enough for the entire file
    std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

    // put file cursor at beginning
    file.seekg(0);

    // load the entire file into the buffer
    file.read(reinterpret_cast<char*>(buffer.data()), fileSize);

    // now that the file is loaded into the buffer, we can close it
    file.close();

    // create a new shader module, using the buffer we loaded
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.pNext = nullptr;

    // codeSize has to be in bytes, so multiply the ints in the buffer by size of
    // int to know the real size of the buffer
    createInfo.codeSize = buffer.size() * sizeof(uint32_t);
    createInfo.pCode = buffer.data();

    // check that the creation goes well.
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        return false;
    }
    *outShaderModule = shaderModule;
    return true;
}

void PipelineBuilder::Clear()
{
    // clear all the structs we need back to 0 with their correct .sType

    InputAssembly = {.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};

    Rasterizer = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};

    ColorBlendAttachment = {};

    Multisampling = {.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};

    PipelineLayout = {};

    DepthStencil = {.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};

    RenderInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};

    ShaderStages.clear();
}

VkPipeline PipelineBuilder::BuildPipeline(const VkDevice device) const
{
    // make viewport state from our stored viewport and scissor.
    // at the moment we won't support multiple viewports or scissors
    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.pNext = nullptr;

    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    // setup dummy color blending. We aren't using transparent objects yet
    // the blending is just "no blend", but we do write to the color attachment
    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.pNext = nullptr;

    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &ColorBlendAttachment;

    // completely clear VertexInputStateCreateInfo, as we have no need for it
    VkPipelineVertexInputStateCreateInfo _vertexInputInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
    };

    // build the actual pipeline
    // we now use all the info structs we have been writing into this one
    // to create the pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo = {.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
    // connect the renderInfo to the pNext extension mechanism
    pipelineInfo.pNext = &RenderInfo;

    pipelineInfo.stageCount = static_cast<uint32_t>(ShaderStages.size());
    pipelineInfo.pStages = ShaderStages.data();
    pipelineInfo.pVertexInputState = &_vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &InputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &Rasterizer;
    pipelineInfo.pMultisampleState = &Multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDepthStencilState = &DepthStencil;
    pipelineInfo.layout = PipelineLayout;

    VkDynamicState state[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamicInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
    dynamicInfo.pDynamicStates = &state[0];
    dynamicInfo.dynamicStateCount = 2;

    pipelineInfo.pDynamicState = &dynamicInfo;

    // its easy to error out on create graphics pipeline, so we handle it a bit
    // better than the common VK_CHECK case
    VkPipeline newPipeline;
    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo,
                                  nullptr, &newPipeline)
        != VK_SUCCESS)
    {
        printf("failed to create pipeline");
        return VK_NULL_HANDLE; // failed to create graphics pipeline
    }
    else
    {
        return newPipeline;
    }
}

void PipelineBuilder::SetShaders(const VkShaderModule vertexShader, const VkShaderModule fragmentShader)
{
    ShaderStages.clear();

    ShaderStages.push_back(
        VkInit::PipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, vertexShader));

    ShaderStages.push_back(
        VkInit::PipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, fragmentShader));
}

void PipelineBuilder::SetInputTopology(const VkPrimitiveTopology topology)
{
    InputAssembly.topology = topology;
    // we are not going to use primitive restart on the entire tutorial so leave
    // it on false
    InputAssembly.primitiveRestartEnable = VK_FALSE;
}

void PipelineBuilder::SetPolygonMode(const VkPolygonMode mode)
{
    Rasterizer.polygonMode = mode;
    Rasterizer.lineWidth = 1.f;
}

void PipelineBuilder::SetCullMode(const VkCullModeFlags cullMode, const VkFrontFace frontFace)
{
    Rasterizer.cullMode = cullMode;
    Rasterizer.frontFace = frontFace;
}

void PipelineBuilder::SetMultisamplingNone()
{
    Multisampling.sampleShadingEnable = VK_FALSE;
    // multisampling defaulted to no multisampling (1 sample per pixel)
    Multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    Multisampling.minSampleShading = 1.0f;
    Multisampling.pSampleMask = nullptr;
    // no alpha to coverage either
    Multisampling.alphaToCoverageEnable = VK_FALSE;
    Multisampling.alphaToOneEnable = VK_FALSE;
}

void PipelineBuilder::DisableBlending()
{
    // default write mask
    ColorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    // no blending
    ColorBlendAttachment.blendEnable = VK_FALSE;
}

void PipelineBuilder::SetDepthBias(VkBool32 enabled)
{
    Rasterizer.depthBiasEnable = enabled;
}

void PipelineBuilder::SetColorAttachmentFormat(const VkFormat format, int32_t colorAttachmentCount)
{
    ColorAttachmentFormat = format;
    // connect the format to the renderInfo  structure
    RenderInfo.colorAttachmentCount = colorAttachmentCount;
    RenderInfo.pColorAttachmentFormats = &ColorAttachmentFormat;
}

void PipelineBuilder::SetDepthFormat(VkFormat format)
{
    RenderInfo.depthAttachmentFormat = format;
}

void PipelineBuilder::DisableDepthTest()
{
    DepthStencil.depthTestEnable = VK_FALSE;
    DepthStencil.depthWriteEnable = VK_FALSE;
    DepthStencil.depthCompareOp = VK_COMPARE_OP_NEVER;
    DepthStencil.depthBoundsTestEnable = VK_FALSE;
    DepthStencil.stencilTestEnable = VK_FALSE;
    DepthStencil.front = {};
    DepthStencil.back = {};
    DepthStencil.minDepthBounds = 0.f;
    DepthStencil.maxDepthBounds = 1.f;
}

void PipelineBuilder::EnableDepthTest(VkBool32 depthTestEnable, VkBool32 depthWriteEnable, VkCompareOp op)
{
    DepthStencil.depthTestEnable = depthTestEnable;
    DepthStencil.depthWriteEnable = depthWriteEnable;
    DepthStencil.depthCompareOp = op;
    DepthStencil.depthBoundsTestEnable = VK_FALSE;
    DepthStencil.stencilTestEnable = VK_FALSE;
    DepthStencil.front = {};
    DepthStencil.back = {};
    DepthStencil.minDepthBounds = 0.f;
    DepthStencil.maxDepthBounds = 1.f;
}