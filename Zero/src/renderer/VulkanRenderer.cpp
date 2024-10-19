#include "VulkanRenderer.h"

#include "core/Engine.h"

#include <shared/vk_initializers.h>
#include <shared/vk_types.h>

#include "VkBootstrap.h"

#include <chrono>
#include <iostream>
#include <thread>
#include <shared/vk_images.h>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"
#include <shared/vk_pipelines.h>

#include <GLFW/glfw3.h>

#ifdef NDEBUG
constexpr bool bUseValidationLayers = false;
#else
constexpr bool bUseValidationLayers = true;
#endif

bool ResizeRequested{};

void VulkanRenderer::Init()
{
    std::cout << "ZeroEngine Vulkan \n";

    InitVulkan();
    InitSwapchain();
    InitCommands();
    InitSyncStructures();
    InitDescriptors();
    InitPipelines();
}

void VulkanRenderer::InitObject(std::span<uint32_t> indices, std::span<Vertex> vertices)
{
    m_Rectangle = UploadMesh(indices, vertices);

    //delete the rectangle data on engine shutdown
    m_MainDeletionQueue.PushFunction([&]()
    {
        DestroyBuffer(m_Rectangle.indexBuffer);
        DestroyBuffer(m_Rectangle.vertexBuffer);
    });
}

void VulkanRenderer::Draw()
{
    if (ResizeRequested)
        ResizeSwapchain();

    // Wait for the render fence: 
    // This ensures that the previous frame's rendering is complete before starting the next frame.
    VK_CHECK(vkWaitForFences(m_Device, 1, &GetCurrentFrame().RenderFence, true, 1000000000));

    // The deletion queue is flushed to clean up any resources that were marked for deletion in the previous frame.
    GetCurrentFrame().DeletionQueue.Flush();

    // The fence used for rendering is reset using vkResetFences() to prepare it for the current frame.
    VK_CHECK(vkResetFences(m_Device, 1, &GetCurrentFrame().RenderFence));

    // Retrieve the index of the image that will be rendered to in the current frame.
    uint32_t swapchainImageIndex;
    VkResult e = vkAcquireNextImageKHR(m_Device, m_Swapchain, 1000000000, GetCurrentFrame().SwapchainSemaphore, nullptr, &swapchainImageIndex);
    if (e == VK_ERROR_OUT_OF_DATE_KHR) {
        ResizeRequested = true;
        return;
    }

    VkCommandBuffer cmd = GetCurrentFrame().MainCommandBuffer;

    VK_CHECK(vkResetCommandBuffer(cmd, 0));

    // This informs Vulkan that the command buffer will be used only once for this frame.
    VkCommandBufferBeginInfo cmdBeginInfo = vkinit::command_buffer_begin_info(
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    m_DrawExtent.width = m_DrawImage.ImageExtent.width;
    m_DrawExtent.height = m_DrawImage.ImageExtent.height;

    // The command buffer recording begins by calling vkBeginCommandBuffer().
    VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

    // The main draw image is transitioned into the general layout using vkutil::transition_image().This allows writing into the image.
    vkutil::transition_image(cmd, m_DrawImage.Image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

    // The DrawBackground() function is called to clear the draw image with a specified clear color.
    DrawBackground(cmd);

    // vkutil::transition_image(cmd, _drawImage.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    DrawGeometry(cmd);

    // vkCmdEndRendering(cmd);

    // Both the draw image and the swapchain image are transitioned into their respective transfer layouts using vkutil::transition_image(). 
    // This prepares them for the copy operation.
    vkutil::transition_image(cmd, m_DrawImage.Image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    vkutil::transition_image(cmd, m_SwapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED,
                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    // The vkutil::copy_image_to_image() function is called to copy the contents of the draw image to the swapchain image. 
    // This effectively renders the frame onto the swapchain image.
    vkutil::copy_image_to_image(cmd, m_DrawImage.Image, m_SwapchainImages[swapchainImageIndex], m_DrawExtent,
                                m_SwapchainExtent);

    // The swapchain image layout is set to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR using vkutil::transition_image(). 
    // This prepares the image to be presented on the screen.
    vkutil::transition_image(cmd, m_SwapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                             VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    // The command buffer recording is finalized using vkEndCommandBuffer(). 
    // At this point, no more commands can be added to the buffer, but it is ready for execution.
    VK_CHECK(vkEndCommandBuffer(cmd));

    // The command buffer is submitted to the graphics queue for execution using vkQueueSubmit2().
    // The _renderFence is used to block until the graphic commands finish execution.
    VkCommandBufferSubmitInfo cmdinfo = vkinit::command_buffer_submit_info(cmd);

    VkSemaphoreSubmitInfo waitInfo = vkinit::semaphore_submit_info(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
                                                                   GetCurrentFrame().SwapchainSemaphore);
    VkSemaphoreSubmitInfo signalInfo = vkinit::semaphore_submit_info(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
                                                                     GetCurrentFrame().RenderSemaphore);

    VkSubmitInfo2 submit = vkinit::submit_info(&cmdinfo, &signalInfo, &waitInfo);

    VK_CHECK(vkQueueSubmit2(m_GraphicsQueue, 1, &submit, GetCurrentFrame().RenderFence));

    // The present operation is prepared by setting up the VkPresentInfoKHR structure. 
    // The _renderSemaphore is waited upon to ensure that the drawing commands have finished before presenting the image to the user.
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;
    presentInfo.pSwapchains = &m_Swapchain;
    presentInfo.swapchainCount = 1;

    presentInfo.pWaitSemaphores = &GetCurrentFrame().RenderSemaphore;
    presentInfo.waitSemaphoreCount = 1;

    // The image index of the swapchain image is specified in the VkPresentInfoKHR structure.
    presentInfo.pImageIndices = &swapchainImageIndex;

    // The image is presented to the screen by calling vkQueuePresentKHR().
    VkResult presentResult = vkQueuePresentKHR(m_GraphicsQueue, &presentInfo);
    if (presentResult == VK_ERROR_OUT_OF_DATE_KHR) {
        ResizeRequested = true;
    }

    m_FrameNumber++;
}

void VulkanRenderer::DrawBackground(VkCommandBuffer cmd)
{
    VkClearColorValue clearValue{m_ClearColor.r, m_ClearColor.g, m_ClearColor.b, m_ClearColor.a};
    VkImageSubresourceRange clearRange = vkinit::image_subresource_range(VK_IMAGE_ASPECT_COLOR_BIT);
    vkCmdClearColorImage(cmd, m_DrawImage.Image, VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);
}

void VulkanRenderer::DrawComputeBackground(VkCommandBuffer cmd)
{
    // Bind the gradient drawing compute pipeline
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_GradientPipeline);

    // Bind the descriptor set containing the draw image for the compute pipeline
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_GradientPipelineLayout, 0, 1,
                            &m_DrawImageDescriptors, 0, nullptr);

    // Execute the compute pipeline dispatch. We are using 16x16 workgroup size so we need to divide by it
    vkCmdDispatch(cmd, std::ceil(m_DrawExtent.width / 16.0), std::ceil(m_DrawExtent.height / 16.0), 1);
}

void VulkanRenderer::DrawGeometry(VkCommandBuffer cmd)
{
    //begin a render pass  connected to our draw image
    VkRenderingAttachmentInfo colorAttachment = vkinit::attachment_info(m_DrawImage.ImageView, nullptr,
                                                                        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    VkRenderingInfo renderInfo = vkinit::rendering_info(m_DrawExtent, &colorAttachment, nullptr);
    vkCmdBeginRendering(cmd, &renderInfo);

    //set dynamic viewport and scissor
    VkViewport viewport = {};
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = m_DrawExtent.width;
    viewport.height = m_DrawExtent.height;
    viewport.minDepth = 0.f;
    viewport.maxDepth = 1.f;

    vkCmdSetViewport(cmd, 0, 1, &viewport);

    VkRect2D scissor = {};
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent.width = m_DrawExtent.width;
    scissor.extent.height = m_DrawExtent.height;

    vkCmdSetScissor(cmd, 0, 1, &scissor);

    // launch a draw command to draw 3 vertices
    // vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _plainPipeline);
    // vkCmdDraw(cmd, 3, 1, 0, 0);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_TexturedPipeline);

    GPUDrawPushConstants push_constants;
    push_constants.worldMatrix = glm::mat4{1.f};
    push_constants.vertexBuffer = m_Rectangle.vertexBufferAddress;

    vkCmdPushConstants(cmd, m_TexturedPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants),
                       &push_constants);
    vkCmdBindIndexBuffer(cmd, m_Rectangle.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(cmd, 6, 1, 0, 0, 0);

    vkCmdEndRendering(cmd);
}

void VulkanRenderer::Shutdown()
{
    vkDeviceWaitIdle(m_Device);

    // Free per-frame structures and deletion queue
    for (int i = 0; i < FRAME_OVERLAP; i++)
    {
        vkDestroyCommandPool(m_Device, m_Frames[i].CommandPool, nullptr);

        // Destroy sync objects
        vkDestroyFence(m_Device, m_Frames[i].RenderFence, nullptr);
        vkDestroySemaphore(m_Device, m_Frames[i].RenderSemaphore, nullptr);
        vkDestroySemaphore(m_Device, m_Frames[i].SwapchainSemaphore, nullptr);

        m_Frames[i].DeletionQueue.Flush();
    }

    // Flush the global deletion queue
    m_MainDeletionQueue.Flush();

    DestroySwapchain();

    vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
    vkDestroyDevice(m_Device, nullptr);

    vkb::destroy_debug_utils_messenger(m_Instance, m_DebugMessenger);
    vkDestroyInstance(m_Instance, nullptr);
}

void VulkanRenderer::InitVulkan()
{
    // VulkanBootstrap instance builder abstracts the creation of a Vulkan Instance
    vkb::InstanceBuilder builder;

    // Application creation info, debug features
    auto inst_ret = builder
                    .set_app_name("Zero")
                    .request_validation_layers(bUseValidationLayers)
                    .use_default_debug_messenger()
                    .require_api_version(1, 3, 0)
                    .build();

    vkb::Instance vkb_inst = inst_ret.value();

    // store instance handle
    m_Instance = vkb_inst.instance;
    m_DebugMessenger = vkb_inst.debug_messenger;

    // Create the surface
    VK_CHECK(glfwCreateWindowSurface(m_Instance, Engine::Get().GetWindow(), nullptr, &m_Surface));

    //vulkan 1.3 features
    VkPhysicalDeviceVulkan13Features features{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
    features.dynamicRendering = true;
    features.synchronization2 = true;

    //vulkan 1.2 features
    VkPhysicalDeviceVulkan12Features features12{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
    features12.bufferDeviceAddress = true;
    features12.descriptorIndexing = true;

    // Use vkbootstrap to select a gpu. 
    // We want a gpu that can write to the surface and supports vulkan 1.3 with the correct features
    vkb::PhysicalDeviceSelector selector{vkb_inst};
    vkb::PhysicalDevice physicalDevice = selector
                                         .set_minimum_version(1, 3)
                                         .set_required_features_13(features)
                                         .set_required_features_12(features12)
                                         .set_surface(m_Surface)
                                         .select()
                                         .value();

    //create the final vulkan device
    vkb::DeviceBuilder deviceBuilder{physicalDevice};

    vkb::Device vkbDevice = deviceBuilder.build().value();

    // Store the VkDevice handle used in the rest of a vulkan application
    m_Device = vkbDevice.device;
    m_PhysicalDevice = physicalDevice.physical_device;

    VkPhysicalDeviceProperties Properties{};

    vkGetPhysicalDeviceProperties(m_PhysicalDevice, &Properties);
    std::cout << "Chosen GPU: " << "\n";
    std::cout << "\t" << Properties.deviceName << "\n";

    // use vkbootstrap to get a Graphics queue
    m_GraphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
    m_GraphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

    // initialize the memory allocator
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = m_PhysicalDevice;
    allocatorInfo.device = m_Device;
    allocatorInfo.instance = m_Instance;
    allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    vmaCreateAllocator(&allocatorInfo, &m_Allocator);

    m_MainDeletionQueue.PushFunction([&]()
    {
        vmaDestroyAllocator(m_Allocator);
    });
}

void VulkanRenderer::InitSwapchain()
{
    CreateSwapchain(m_WindowExtent.width, m_WindowExtent.height);

    // Define the drawImageExtent variable, which represents the extent (size) of the image that will be used for drawing. 
    // The width and height of the image are set to _windowExtent.width and _windowExtent.height, respectively. 
    // The 1 represents the depth of the image, which is set to 1 because it's a 2D image.
    VkExtent3D drawImageExtent = {
        m_WindowExtent.width,
        m_WindowExtent.height,
        1
    };

    // Set the format of the drawImage to VK_FORMAT_R16G16B16A16_SFLOAT, 
    // which represents a 16-bit floating-point format with 4 color channels (red, green, blue, and alpha).
    m_DrawImage.ImageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
    // Set the extent of the drawImage to the previously defined drawImageExtent.
    m_DrawImage.ImageExtent = drawImageExtent;

    // Define the drawImageUsages variable, which represents the usage flags for the drawImage. 
    // These flags specify how the image will be used, 
    // such as for transferring data, storing data, or as a color attachment for rendering.
    VkImageUsageFlags drawImageUsages{};
    drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
    drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // Create a VkImageCreateInfo structure rimg_info with the specified format, usage flags, and extent for the drawImage.
    VkImageCreateInfo rimg_info = vkinit::image_create_info(m_DrawImage.ImageFormat, drawImageUsages, drawImageExtent);

    VmaAllocationCreateInfo rimg_allocinfo = {};
    rimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    rimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vmaCreateImage(m_Allocator, &rimg_info, &rimg_allocinfo, &m_DrawImage.Image, &m_DrawImage.Allocation, nullptr);

    // Create a VkImageViewCreateInfo structure rview_info to specify the format, image, and aspect (how the image will be viewed) for the drawImage.
    VkImageViewCreateInfo rview_info = vkinit::imageview_create_info(m_DrawImage.ImageFormat, m_DrawImage.Image,
                                                                     VK_IMAGE_ASPECT_COLOR_BIT);

    // Calls vkCreateImageView() to create an image view for the drawImage using the specified image view info.
    // The image view is stored in the _drawImage.imageView variable.
    VK_CHECK(vkCreateImageView(m_Device, &rview_info, nullptr, &m_DrawImage.ImageView));

    // Push a function to the _mainDeletionQueue to destroy the image view 
    // and deallocate the memory for the drawImage when the renderer is shut down.
    m_MainDeletionQueue.PushFunction([=]()
    {
        vkDestroyImageView(m_Device, m_DrawImage.ImageView, nullptr);
        vmaDestroyImage(m_Allocator, m_DrawImage.Image, m_DrawImage.Allocation);
    });
}

void VulkanRenderer::InitCommands()
{
    // Create a command pool for commands submitted to the graphics queue.
    // We also want the pool to allow for resetting of individual command buffers
    VkCommandPoolCreateInfo commandPoolInfo = vkinit::command_pool_create_info(
        m_GraphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    for (auto& m_Frame : m_Frames)
    {
        VK_CHECK(vkCreateCommandPool(m_Device, &commandPoolInfo, nullptr, &m_Frame.CommandPool));

        // Allocate the default command buffer that we will use for rendering
        VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::command_buffer_allocate_info(m_Frame.CommandPool, 1);

        VK_CHECK(vkAllocateCommandBuffers(m_Device, &cmdAllocInfo, &m_Frame.MainCommandBuffer));
    }

    VK_CHECK(vkCreateCommandPool(m_Device, &commandPoolInfo, nullptr, &m_ImmCommandPool));

    // allocate the command buffer for immediate submits
    VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::command_buffer_allocate_info(m_ImmCommandPool, 1);

    VK_CHECK(vkAllocateCommandBuffers(m_Device, &cmdAllocInfo, &m_ImmCommandBuffer));

    m_MainDeletionQueue.PushFunction([=]()
    {
        vkDestroyCommandPool(m_Device, m_ImmCommandPool, nullptr);
    });
}

void VulkanRenderer::InitSyncStructures()
{
    // Create syncronization structures
    // One fence to control when the gpu has finished rendering the frame,
    // And 2 semaphores to syncronize rendering with swapchain
    // We want the fence to start signalled so we can wait on it on the first frame
    VkFenceCreateInfo fenceCreateInfo = vkinit::fence_create_info(VK_FENCE_CREATE_SIGNALED_BIT);
    VkSemaphoreCreateInfo semaphoreCreateInfo = vkinit::semaphore_create_info();

    for (int i = 0; i < FRAME_OVERLAP; i++)
    {
        VK_CHECK(vkCreateFence(m_Device, &fenceCreateInfo, nullptr, &m_Frames[i].RenderFence));

        VK_CHECK(vkCreateSemaphore(m_Device, &semaphoreCreateInfo, nullptr, &m_Frames[i].SwapchainSemaphore));
        VK_CHECK(vkCreateSemaphore(m_Device, &semaphoreCreateInfo, nullptr, &m_Frames[i].RenderSemaphore));
    }

    VK_CHECK(vkCreateFence(m_Device, &fenceCreateInfo, nullptr, &m_ImmFence));
    m_MainDeletionQueue.PushFunction([=]() { vkDestroyFence(m_Device, m_ImmFence, nullptr); });
}

void VulkanRenderer::InitDescriptors()
{
    // Create a descriptor pool that will hold 10 sets with 1 image each
    std::vector<DescriptorAllocator::PoolSizeRatio> sizes =
    {
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1}
    };

    m_GlobalDescriptorAllocator.init_pool(m_Device, 10, sizes);

    // Make the descriptor set layout for our compute draw
    {
        DescriptorLayoutBuilder builder;
        builder.add_binding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
        m_DrawImageDescriptorLayout = builder.build(m_Device, VK_SHADER_STAGE_COMPUTE_BIT);
    }

    // Allocate a descriptor set for our draw image
    m_DrawImageDescriptors = m_GlobalDescriptorAllocator.allocate(m_Device, m_DrawImageDescriptorLayout);

    VkDescriptorImageInfo imgInfo{};
    imgInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    imgInfo.imageView = m_DrawImage.ImageView;

    VkWriteDescriptorSet drawImageWrite = {};
    drawImageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    drawImageWrite.pNext = nullptr;

    drawImageWrite.dstBinding = 0;
    drawImageWrite.dstSet = m_DrawImageDescriptors;
    drawImageWrite.descriptorCount = 1;
    drawImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    drawImageWrite.pImageInfo = &imgInfo;

    vkUpdateDescriptorSets(m_Device, 1, &drawImageWrite, 0, nullptr);

    // Make sure both the descriptor allocator and the new layout get cleaned up properly
    m_MainDeletionQueue.PushFunction([&]()
    {
        m_GlobalDescriptorAllocator.destroy_pool(m_Device);

        vkDestroyDescriptorSetLayout(m_Device, m_DrawImageDescriptorLayout, nullptr);
    });
}

void VulkanRenderer::InitPipelines()
{
    InitBackgroundPipelines();

    // init_plain_pipeline();
    InitTexturedPipeline();
}

void VulkanRenderer::InitBackgroundPipelines()
{
    VkPipelineLayoutCreateInfo computeLayout{};
    computeLayout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    computeLayout.pNext = nullptr;
    computeLayout.pSetLayouts = &m_DrawImageDescriptorLayout;
    computeLayout.setLayoutCount = 1;

    VK_CHECK(vkCreatePipelineLayout(m_Device, &computeLayout, nullptr, &m_GradientPipelineLayout));

    VkShaderModule computeDrawShader;
    if (!vkutil::load_shader_module("../shaders/compiled/gradient.comp.spv", m_Device, &computeDrawShader))
    {
        std::cout << "Error when building the compute shader \n";
    }

    VkPipelineShaderStageCreateInfo stageinfo{};
    stageinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageinfo.pNext = nullptr;
    stageinfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    stageinfo.module = computeDrawShader;
    stageinfo.pName = "main";

    VkComputePipelineCreateInfo computePipelineCreateInfo{};
    computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    computePipelineCreateInfo.pNext = nullptr;
    computePipelineCreateInfo.layout = m_GradientPipelineLayout;
    computePipelineCreateInfo.stage = stageinfo;

    VK_CHECK(
        vkCreateComputePipelines(m_Device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &m_GradientPipeline))
    ;

    vkDestroyShaderModule(m_Device, computeDrawShader, nullptr);

    m_MainDeletionQueue.PushFunction([&]()
    {
        vkDestroyPipelineLayout(m_Device, m_GradientPipelineLayout, nullptr);
        vkDestroyPipeline(m_Device, m_GradientPipeline, nullptr);
    });
}

void VulkanRenderer::InitPlainPipeline()
{
    VkShaderModule triangleFragShader;
    if (!vkutil::load_shader_module("../shaders/compiled/plain_shader.frag.spv", m_Device, &triangleFragShader))
    {
        printf("Error when building the triangle fragment shader module ");
    }
    else
    {
        printf("Triangle fragment shader succesfully loaded \n");
    }

    VkShaderModule triangleVertexShader;
    if (!vkutil::load_shader_module("../shaders/compiled/plain_shader.vert.spv", m_Device, &triangleVertexShader))
    {
        printf("Error when building the triangle vertex shader module \n");
    }
    else
    {
        printf("Triangle vertex shader succesfully loaded \n");
    }

    //build the pipeline layout that controls the inputs/outputs of the shader
    //we are not using descriptor sets or other systems yet, so no need to use anything other than empty default
    VkPipelineLayoutCreateInfo pipeline_layout_info = vkinit::pipeline_layout_create_info();
    VK_CHECK(vkCreatePipelineLayout(m_Device, &pipeline_layout_info, nullptr, &m_PlainPipelineLayout));

    PipelineBuilder pipelineBuilder;

    //use the triangle layout we created
    pipelineBuilder._pipelineLayout = m_PlainPipelineLayout;
    //connecting the vertex and pixel shaders to the pipeline
    pipelineBuilder.set_shaders(triangleVertexShader, triangleFragShader);
    //it will draw triangles
    pipelineBuilder.set_input_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    //filled triangles
    pipelineBuilder.set_polygon_mode(VK_POLYGON_MODE_FILL);
    //no backface culling
    pipelineBuilder.set_cull_mode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
    //no multisampling
    pipelineBuilder.set_multisampling_none();
    //no blending
    pipelineBuilder.disable_blending();
    //no depth testing
    pipelineBuilder.disable_depthtest();

    //connect the image format we will draw into, from draw image
    pipelineBuilder.set_color_attachment_format(m_DrawImage.ImageFormat);
    pipelineBuilder.set_depth_format(VK_FORMAT_UNDEFINED);

    //finally build the pipeline
    m_PlainPipeline = pipelineBuilder.build_pipeline(m_Device);

    //clean structures
    vkDestroyShaderModule(m_Device, triangleFragShader, nullptr);
    vkDestroyShaderModule(m_Device, triangleVertexShader, nullptr);

    m_MainDeletionQueue.PushFunction([&]()
    {
        vkDestroyPipelineLayout(m_Device, m_PlainPipelineLayout, nullptr);
        vkDestroyPipeline(m_Device, m_PlainPipeline, nullptr);
    });
}

void VulkanRenderer::InitTexturedPipeline()
{
    VkShaderModule triangleFragShader;
    if (!vkutil::load_shader_module("../shaders/compiled/textured.frag.spv", m_Device, &triangleFragShader))
    {
        printf("Error when building the triangle fragment shader module \n");
    }
    else
    {
        printf("Triangle fragment shader succesfully loaded \n");
    }

    VkShaderModule triangleVertexShader;
    if (!vkutil::load_shader_module("../shaders/compiled/textured.vert.spv", m_Device, &triangleVertexShader))
    {
        printf("Error when building the triangle vertex shader module \n");
    }
    else
    {
        printf("Triangle vertex shader succesfully loaded \n");
    }

    VkPushConstantRange bufferRange{};
    bufferRange.offset = 0;
    bufferRange.size = sizeof(GPUDrawPushConstants);
    bufferRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkPipelineLayoutCreateInfo pipeline_layout_info = vkinit::pipeline_layout_create_info();
    pipeline_layout_info.pPushConstantRanges = &bufferRange;
    pipeline_layout_info.pushConstantRangeCount = 1;

    VK_CHECK(vkCreatePipelineLayout(m_Device, &pipeline_layout_info, nullptr, &m_TexturedPipelineLayout));

    PipelineBuilder pipelineBuilder;

    //use the triangle layout we created
    pipelineBuilder._pipelineLayout = m_TexturedPipelineLayout;
    //connecting the vertex and pixel shaders to the pipeline
    pipelineBuilder.set_shaders(triangleVertexShader, triangleFragShader);
    //it will draw triangles
    pipelineBuilder.set_input_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    //filled triangles
    pipelineBuilder.set_polygon_mode(VK_POLYGON_MODE_FILL);
    //no backface culling
    pipelineBuilder.set_cull_mode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
    //no multisampling
    pipelineBuilder.set_multisampling_none();
    //no blending
    pipelineBuilder.disable_blending();

    pipelineBuilder.disable_depthtest();

    //connect the image format we will draw into, from draw image
    pipelineBuilder.set_color_attachment_format(m_DrawImage.ImageFormat);
    pipelineBuilder.set_depth_format(VK_FORMAT_UNDEFINED);

    //finally build the pipeline
    m_TexturedPipeline = pipelineBuilder.build_pipeline(m_Device);

    //clean structures
    vkDestroyShaderModule(m_Device, triangleFragShader, nullptr);
    vkDestroyShaderModule(m_Device, triangleVertexShader, nullptr);

    m_MainDeletionQueue.PushFunction([&]()
    {
        vkDestroyPipelineLayout(m_Device, m_TexturedPipelineLayout, nullptr);
        vkDestroyPipeline(m_Device, m_TexturedPipeline, nullptr);
    });
}

void VulkanRenderer::CreateSwapchain(uint32_t width, uint32_t height)
{
    vkb::SwapchainBuilder swapchainBuilder(m_PhysicalDevice, m_Device, m_Surface);

    m_SwapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

    // Building the swapchain
    // The most important detail here is the present mode, which we have set to VK_PRESENT_MODE_FIFO_KHR.
    // This way we are doing a hard VSync, which will limit the FPS of the entire engine to the refresh-rate of the monitor.

    vkb::Swapchain vkbSwapchain = swapchainBuilder
                                  //.use_default_format_selection()
                                  .set_desired_format(VkSurfaceFormatKHR{
                                      .format = m_SwapchainImageFormat, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
                                  })
                                  // Use vsync present mode
                                  .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
                                  .set_desired_extent(width, height)
                                  .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
                                  .build()
                                  .value();

    m_SwapchainExtent = vkbSwapchain.extent;
    // Store swapchain and its related images
    m_Swapchain = vkbSwapchain.swapchain;
    m_SwapchainImages = vkbSwapchain.get_images().value();
    m_SwapchainImageViews = vkbSwapchain.get_image_views().value();
}

void VulkanRenderer::DestroySwapchain()
{
    vkDestroySwapchainKHR(m_Device, m_Swapchain, nullptr);

    // Destroy swapchain resources
    for (auto& _swapchainImageView : m_SwapchainImageViews)
    {
        vkDestroyImageView(m_Device, _swapchainImageView, nullptr);
    }
}

void VulkanRenderer::ResizeSwapchain()
{
    vkDeviceWaitIdle(m_Device);

    DestroySwapchain();

    int w, h;


    glfwGetWindowSize(Engine::Get().GetWindow(), &w, &h);

    m_WindowExtent.width = w;
    m_WindowExtent.height = h;

    CreateSwapchain(m_WindowExtent.width, m_WindowExtent.height);

    ResizeRequested = false;
}

GPUMeshBuffers VulkanRenderer::UploadMesh(std::span<uint32_t> indices, std::span<Vertex> vertices)
{
    const size_t vertexBufferSize = vertices.size() * sizeof(Vertex);
    const size_t indexBufferSize = indices.size() * sizeof(uint32_t);

    GPUMeshBuffers newSurface;

    //create vertex buffer
    newSurface.vertexBuffer = CreateBuffer(vertexBufferSize,
                                           VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                           VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                                           VMA_MEMORY_USAGE_GPU_ONLY);

    //find the adress of the vertex buffer
    VkBufferDeviceAddressInfo deviceAdressInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, .buffer = newSurface.vertexBuffer.buffer
    };
    newSurface.vertexBufferAddress = vkGetBufferDeviceAddress(m_Device, &deviceAdressInfo);

    //create index buffer
    newSurface.indexBuffer = CreateBuffer(indexBufferSize,
                                          VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                          VMA_MEMORY_USAGE_GPU_ONLY);

    AllocatedBuffer staging = CreateBuffer(vertexBufferSize + indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                           VMA_MEMORY_USAGE_CPU_ONLY);

    void* data = staging.allocation->GetMappedData();

    // copy vertex buffer
    memcpy(data, vertices.data(), vertexBufferSize);
    // copy index buffer
    memcpy((char*)data + vertexBufferSize, indices.data(), indexBufferSize);

    ImmediateSubmit([&](VkCommandBuffer cmd)
    {
        VkBufferCopy vertexCopy{0};
        vertexCopy.dstOffset = 0;
        vertexCopy.srcOffset = 0;
        vertexCopy.size = vertexBufferSize;

        vkCmdCopyBuffer(cmd, staging.buffer, newSurface.vertexBuffer.buffer, 1, &vertexCopy);

        VkBufferCopy indexCopy{0};
        indexCopy.dstOffset = 0;
        indexCopy.srcOffset = vertexBufferSize;
        indexCopy.size = indexBufferSize;

        vkCmdCopyBuffer(cmd, staging.buffer, newSurface.indexBuffer.buffer, 1, &indexCopy);
    });

    DestroyBuffer(staging);

    return newSurface;
}


AllocatedBuffer VulkanRenderer::CreateBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
{
    // allocate buffer
    VkBufferCreateInfo bufferInfo = {.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferInfo.pNext = nullptr;
    bufferInfo.size = allocSize;

    bufferInfo.usage = usage;

    VmaAllocationCreateInfo vmaallocInfo = {};
    vmaallocInfo.usage = memoryUsage;
    vmaallocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
    AllocatedBuffer newBuffer;

    // allocate the buffer
    VK_CHECK(vmaCreateBuffer(m_Allocator, &bufferInfo, &vmaallocInfo, &newBuffer.buffer, &newBuffer.allocation,
        &newBuffer.info));

    return newBuffer;
}

void VulkanRenderer::DestroyBuffer(const AllocatedBuffer& buffer)
{
    vmaDestroyBuffer(m_Allocator, buffer.buffer, buffer.allocation);
}

void VulkanRenderer::ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function)
{
    VK_CHECK(vkResetFences(m_Device, 1, &m_ImmFence));
    VK_CHECK(vkResetCommandBuffer(m_ImmCommandBuffer, 0));

    VkCommandBuffer cmd = m_ImmCommandBuffer;

    VkCommandBufferBeginInfo cmdBeginInfo = vkinit::command_buffer_begin_info(
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

    function(cmd);

    VK_CHECK(vkEndCommandBuffer(cmd));

    VkCommandBufferSubmitInfo cmdinfo = vkinit::command_buffer_submit_info(cmd);
    VkSubmitInfo2 submit = vkinit::submit_info(&cmdinfo, nullptr, nullptr);

    // submit command buffer to the queue and execute it.
    //  _renderFence will now block until the graphic commands finish execution
    VK_CHECK(vkQueueSubmit2(m_GraphicsQueue, 1, &submit, m_ImmFence));

    VK_CHECK(vkWaitForFences(m_Device, 1, &m_ImmFence, true, 9999999999));
}
