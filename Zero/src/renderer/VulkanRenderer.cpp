#include "VulkanRenderer.h"

#include "Application.h"

#include <Renderer/Vulkan/vk_initializers.h>
#include <Renderer/Vulkan/vk_types.h>

#include "VkBootstrap.h"

#include <chrono>
#include <iostream>
#include <Renderer/Vulkan/vk_images.h>

#define VMA_IMPLEMENTATION
#include <Renderer/Vulkan/vk_pipelines.h>
#include "vk_mem_alloc.h"

#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

#include <stb_image.h>
#include <glm/gtx/quaternion.hpp>
#include <Renderer/Vulkan/VulkanBuffer.h>

#include "imgui.h"
#include "ImGui/imgui_impl_glfw.h"
#include "ImGui/imgui_impl_vulkan.h"

namespace Zero
{
#ifdef _DEBUG
    constexpr bool UseValidationLayers = true;
#else
    constexpr bool UseValidationLayers = false;
#endif


    bool ResizeRequested{};

    void VulkanRenderer::Init()
    {
        std::cout << "ZeroEngine Vulkan \n";
        UseValidationLayers ? std::cout << "Validation Layers Enabled \n" : std::cout << "Validation Layers Disabled \n";

        InitVulkan();
        InitSwapchain();
        InitCommands();
        InitSyncStructures();
        InitDescriptors();
        InitPipelines();
        InitTextures();
    }

    void VulkanRenderer::InitImGui()
    {
        // 1: create descriptor pool for IMGUI
        //  the size of the pool is very oversize, but it's copied from imgui demo
        //  itself.
        VkDescriptorPoolSize pool_sizes[] = { { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 } };

        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        poolInfo.maxSets = 1000;
        poolInfo.poolSizeCount = static_cast<uint32_t>(std::size(pool_sizes));
        poolInfo.pPoolSizes = pool_sizes;

        VkDescriptorPool imguiPool;
        VK_CHECK(vkCreateDescriptorPool(m_Device, &poolInfo, nullptr, &imguiPool));

        // 2: initialize imgui library

        // this initializes the core structures of imgui
        ImGui::CreateContext();

        // this initializes imgui for SDL
        GLFWwindow* window = Application::Get().GetWindow();
        ImGui_ImplGlfw_InitForVulkan(window, true);

        // this initializes imgui for Vulkan
        ImGui_ImplVulkan_InitInfo initInfo = {};
        initInfo.Instance = m_Instance;
        initInfo.PhysicalDevice = m_PhysicalDevice;
        initInfo.Device = m_Device;
        initInfo.Queue = m_GraphicsQueue;
        initInfo.DescriptorPool = imguiPool;
        initInfo.MinImageCount = 3;
        initInfo.ImageCount = 3;
        initInfo.UseDynamicRendering = true;

        //dynamic rendering parameters for imgui to use
        initInfo.PipelineRenderingCreateInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
        initInfo.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
        initInfo.PipelineRenderingCreateInfo.pColorAttachmentFormats = &m_SwapchainImageFormat;

        initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

        ImGui_ImplVulkan_Init(&initInfo);

        ImGui_ImplVulkan_CreateFontsTexture();

        // add the destroy the imgui created structures
        m_MainDeletionQueue.PushFunction([=]() {
            ImGui_ImplVulkan_Shutdown();
            vkDestroyDescriptorPool(m_Device, imguiPool, nullptr);
            });
    }

    void VulkanRenderer::InitTextures()
    {
        m_DefaultTexture = VulkanTexture("../assets/images/plain.png", "texture_diffuse", true);

        VkSamplerCreateInfo samplerCreateInfo = {.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};

        samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
        samplerCreateInfo.minFilter = VK_FILTER_NEAREST;

        vkCreateSampler(m_Device, &samplerCreateInfo, nullptr, &m_DefaultSamplerNearest);

        samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
        samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
        vkCreateSampler(m_Device, &samplerCreateInfo, nullptr, &m_DefaultSamplerLinear);

        m_MainDeletionQueue.PushFunction([&]()
        {
            vkDestroySampler(m_Device, m_DefaultSamplerNearest, nullptr);
            vkDestroySampler(m_Device, m_DefaultSamplerLinear, nullptr);

            //DestroyImage(m_ErrorCheckerboardImage);
            DestroyImage(m_DefaultTexture.GetImage());
        });
    }

    void VulkanRenderer::Draw(Scene* scene)
    {
        NewFrameImGui();

        if (ResizeRequested)
            ResizeSwapchain();

        // Wait for the render fence: 
        // This ensures that the previous frame's rendering is complete before starting the next frame.
        VK_CHECK(vkWaitForFences(m_Device, 1, &GetCurrentFrame().RenderFence, true, 1000000000));

        GetCurrentFrame().DeletionQueue.Flush();
        GetCurrentFrame().FrameDescriptors.ClearPools(m_Device);

        // The deletion queue is flushed to clean up any resources that were marked for deletion in the previous frame.
        GetCurrentFrame().DeletionQueue.Flush();

        // The fence used for rendering is reset using vkResetFences() to prepare it for the current frame.
        VK_CHECK(vkResetFences(m_Device, 1, &GetCurrentFrame().RenderFence));

        // Retrieve the index of the image that will be rendered to in the current frame.
        uint32_t swapchainImageIndex;
        VkResult e = vkAcquireNextImageKHR(m_Device, m_Swapchain, 1000000000, GetCurrentFrame().SwapchainSemaphore,
                                           nullptr, &swapchainImageIndex);
        if (e == VK_ERROR_OUT_OF_DATE_KHR)
        {
            ResizeRequested = true;
            return;
        }

        VkCommandBuffer cmd = GetCurrentFrame().MainCommandBuffer;

        VK_CHECK(vkResetCommandBuffer(cmd, 0));

        // This informs Vulkan that the command buffer will be used only once for this frame.
        VkCommandBufferBeginInfo cmdBeginInfo = VkInit::CommandBufferBeginInfo(
            VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        m_DrawExtent.width = m_DrawImage.ImageExtent.width;
        m_DrawExtent.height = m_DrawImage.ImageExtent.height;

        // The command buffer recording begins by calling vkBeginCommandBuffer().
        VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

        // The main draw image is transitioned into the general layout using vkutil::transition_image().This allows writing into the image.
        VkUtil::TransitionImage(cmd, m_DrawImage.Image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

        // The DrawBackground() function is called to clear the draw image with a specified clear color.
        DrawBackground(cmd);

        VkUtil::TransitionImage(cmd, m_DrawImage.Image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
        VkUtil::TransitionImage(cmd, m_DepthImage.Image, VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
        //DrawGeometry(cmd);

        if (scene)
        {
            DrawGeometryTextured(scene, cmd);
        }
       

        // vkCmdEndRendering(cmd);

        // Both the draw image and the swapchain image are transitioned into their respective transfer layouts using vkutil::transition_image(). 
        // This prepares them for the copy operation.
        VkUtil::TransitionImage(cmd, m_DrawImage.Image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        VkUtil::TransitionImage(cmd, m_SwapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED,
                                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        // The vkutil::copy_image_to_image() function is called to copy the contents of the draw image to the swapchain image. 
        // This effectively renders the frame onto the swapchain image.
        VkUtil::CopyImageToImage(cmd, m_DrawImage.Image, m_SwapchainImages[swapchainImageIndex], m_DrawExtent,
                                 m_SwapchainExtent);



        // The swapchain image layout is set to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR using vkutil::transition_image(). 
        // This prepares the image to be presented on the screen.
        VkUtil::TransitionImage(cmd, m_SwapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

        DrawImGui(cmd, m_SwapchainImageViews[swapchainImageIndex]);

        // The command buffer recording is finalized using vkEndCommandBuffer(). 
        // At this point, no more commands can be added to the buffer, but it is ready for execution.
        VK_CHECK(vkEndCommandBuffer(cmd));

        // The command buffer is submitted to the graphics queue for execution using vkQueueSubmit2().
        // The _renderFence is used to block until the graphic commands finish execution.
        VkCommandBufferSubmitInfo cmdinfo = VkInit::CommandBufferSubmitInfo(cmd);

        VkSemaphoreSubmitInfo waitInfo = VkInit::SemaphoreSubmitInfo(
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
            GetCurrentFrame().SwapchainSemaphore);
        VkSemaphoreSubmitInfo signalInfo = VkInit::SemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
                                                                       GetCurrentFrame().RenderSemaphore);

        VkSubmitInfo2 submit = VkInit::SubmitInfo(&cmdinfo, &signalInfo, &waitInfo);

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
        if (presentResult == VK_ERROR_OUT_OF_DATE_KHR)
        {
            ResizeRequested = true;
        }

        m_FrameNumber++;
    }

    void VulkanRenderer::DrawBackground(VkCommandBuffer cmd)
    {
        VkClearColorValue clearValue{m_ClearColor.r, m_ClearColor.g, m_ClearColor.b, m_ClearColor.a};
        VkImageSubresourceRange clearRange = VkInit::ImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);
        vkCmdClearColorImage(cmd, m_DrawImage.Image, VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);
    }

    void VulkanRenderer::DrawGeometryTextured(Scene* scene, VkCommandBuffer cmd)
    {
        //begin a render pass  connected to our draw image
        VkRenderingAttachmentInfo colorAttachment = VkInit::AttachmentInfo(m_DrawImage.ImageView, nullptr,
                                                                           VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        VkRenderingAttachmentInfo depthAttachment = VkInit::DepthAttachmentInfo(
            m_DepthImage.ImageView, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

        VkRenderingInfo renderInfo = VkInit::RenderingInfo(m_DrawExtent, &colorAttachment, &depthAttachment);
        vkCmdBeginRendering(cmd, &renderInfo);

        //set dynamic viewport and scissor
        VkViewport viewport = {};
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = static_cast<float>(m_DrawExtent.width);
        viewport.height = static_cast<float>(m_DrawExtent.height);
        viewport.minDepth = 1.f;
        viewport.maxDepth = 0.f;

        vkCmdSetViewport(cmd, 0, 1, &viewport);

        VkRect2D scissor = {};
        scissor.offset.x = 0;
        scissor.offset.y = 0;
        scissor.extent.width = m_DrawExtent.width;
        scissor.extent.height = m_DrawExtent.height;

        vkCmdSetScissor(cmd, 0, 1, &scissor);

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_TexturedPipeline);

        glm::mat4 view = Application::Get().GetActiveCamera().GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(Application::Get().GetActiveCamera().GetFOV()),
            (float)m_DrawExtent.width / (float)m_DrawExtent.height, 0.1f, 10000.f);

        projection[1][1] *= -1;

        //allocate a new uniform buffer for the scene data
        AllocatedBuffer gpuSceneDataBuffer = VulkanBufferManager::CreateBuffer(m_Allocator, sizeof(GPUSceneData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

        //add it to the deletion queue of this frame so it gets deleted once its been used
        GetCurrentFrame().DeletionQueue.PushFunction([=, this]() {
            VulkanBufferManager::DestroyBuffer(m_Allocator, gpuSceneDataBuffer);
            });

        //write the buffer
        GPUSceneData* sceneUniformData = (GPUSceneData*)gpuSceneDataBuffer.Allocation->GetMappedData();
        *sceneUniformData = m_SceneData;

        sceneUniformData->Viewproj = projection * view;
        sceneUniformData->DirLight.Color = scene->GetDirectionalLight()->GetColor();
        sceneUniformData->DirLight.AmbientIntensity = scene->GetDirectionalLight()->GetAmbientIntensity();
        sceneUniformData->DirLight.Direction = scene->GetDirectionalLight()->GetDirection();
        sceneUniformData->DirLight.DiffuseIntensity = scene->GetDirectionalLight()->GetDiffuseIntensity();

        //create a descriptor set that binds that buffer and update it
        VkDescriptorSet globalDescriptor = GetCurrentFrame().FrameDescriptors.Allocate(m_Device, m_GpuSceneDataDescriptorLayout);

        DescriptorWriter writer;
        writer.WriteBuffer(0, gpuSceneDataBuffer.Buffer, sizeof(GPUSceneData), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        writer.UpdateSet(m_Device, globalDescriptor);

        for (const auto& gameObj : scene->GetGameObjects())
        {
            GPUDrawPushConstants pushConstants;
            pushConstants.ModelMatrix = gameObj->GetTransform().GetMatrix();
            pushConstants.CameraPos = Application::Get().GetActiveCamera().GetPosition();
            gameObj->GetModel()->Draw(cmd, writer, m_TexturedPipelineLayout, m_DrawExtent, m_DefaultSamplerLinear, pushConstants);
        }

        vkCmdEndRendering(cmd);
    }

    void VulkanRenderer::NewFrameImGui()
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        Application::Get().UpdateImGui();

        //make imgui calculate internal draw structures
        ImGui::Render();
    }

    void VulkanRenderer::DrawImGui(VkCommandBuffer cmd, VkImageView targetImageView)
    {
        VkRenderingAttachmentInfo colorAttachment = VkInit::AttachmentInfo(targetImageView, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        VkRenderingInfo renderInfo = VkInit::RenderingInfo(m_SwapchainExtent, &colorAttachment, nullptr);

        vkCmdBeginRendering(cmd, &renderInfo);

        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

        vkCmdEndRendering(cmd);
    }

    void VulkanRenderer::Shutdown()
    {
        vkDeviceWaitIdle(m_Device);

        // Free per-frame structures and deletion queue
        for (auto& frame : m_Frames)
        {
            vkDestroyCommandPool(m_Device, frame.CommandPool, nullptr);

            // Destroy sync objects
            vkDestroyFence(m_Device, frame.RenderFence, nullptr);
            vkDestroySemaphore(m_Device, frame.RenderSemaphore, nullptr);
            vkDestroySemaphore(m_Device, frame.SwapchainSemaphore, nullptr);

            frame.DeletionQueue.Flush();
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
                        .request_validation_layers(UseValidationLayers)
                        .use_default_debug_messenger()
                        .require_api_version(1, 3, 0)
                        .build();

        vkb::Instance vkb_inst = inst_ret.value();

        // store instance handle
        m_Instance = vkb_inst.instance;
        m_DebugMessenger = vkb_inst.debug_messenger;

        // Create the surface
        VK_CHECK(glfwCreateWindowSurface(m_Instance, Application::Get().GetWindow(), nullptr, &m_Surface));

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
        VkImageCreateInfo rimgInfo =
            VkInit::ImageCreateInfo(m_DrawImage.ImageFormat, drawImageUsages, drawImageExtent);

        VmaAllocationCreateInfo rimgAllocinfo = {};
        rimgAllocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        rimgAllocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        vmaCreateImage(m_Allocator, &rimgInfo, &rimgAllocinfo, &m_DrawImage.Image, &m_DrawImage.Allocation, nullptr);

        // Create a VkImageViewCreateInfo structure rview_info to specify the format, image, and aspect (how the image will be viewed) for the drawImage.
        VkImageViewCreateInfo rviewInfo = VkInit::ImageviewCreateInfo(m_DrawImage.ImageFormat, m_DrawImage.Image,
                                                                      VK_IMAGE_ASPECT_COLOR_BIT);

        // Calls vkCreateImageView() to create an image view for the drawImage using the specified image view info.
        // The image view is stored in the _drawImage.imageView variable.
        VK_CHECK(vkCreateImageView(m_Device, &rviewInfo, nullptr, &m_DrawImage.ImageView));

        m_DepthImage.ImageFormat = VK_FORMAT_D32_SFLOAT;
        m_DepthImage.ImageExtent = drawImageExtent;
        VkImageUsageFlags depthImageUsages{};
        depthImageUsages |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

        VkImageCreateInfo dimgInfo = VkInit::ImageCreateInfo(m_DepthImage.ImageFormat, depthImageUsages,
                                                             drawImageExtent);

        //allocate and create the image
        vmaCreateImage(m_Allocator, &dimgInfo, &rimgAllocinfo, &m_DepthImage.Image, &m_DepthImage.Allocation, nullptr);

        //build an image-view for the draw image to use for rendering
        VkImageViewCreateInfo dviewInfo = VkInit::ImageviewCreateInfo(m_DepthImage.ImageFormat, m_DepthImage.Image,
                                                                      VK_IMAGE_ASPECT_DEPTH_BIT);

        VK_CHECK(vkCreateImageView(m_Device, &dviewInfo, nullptr, &m_DepthImage.ImageView));

        // Push a function to the _mainDeletionQueue to destroy the image view 
        // and deallocate the memory for the drawImage when the renderer is shut down.
        m_MainDeletionQueue.PushFunction([=]()
        {
            vkDestroyImageView(m_Device, m_DrawImage.ImageView, nullptr);
            vmaDestroyImage(m_Allocator, m_DrawImage.Image, m_DrawImage.Allocation);

            vkDestroyImageView(m_Device, m_DepthImage.ImageView, nullptr);
            vmaDestroyImage(m_Allocator, m_DepthImage.Image, m_DepthImage.Allocation);
        });
    }

    void VulkanRenderer::InitCommands()
    {
        // Create a command pool for commands submitted to the graphics queue.
        // We also want the pool to allow for resetting of individual command buffers
        VkCommandPoolCreateInfo commandPoolInfo = VkInit::CommandPoolCreateInfo(
            m_GraphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

        for (auto& m_Frame : m_Frames)
        {
            VK_CHECK(vkCreateCommandPool(m_Device, &commandPoolInfo, nullptr, &m_Frame.CommandPool));

            // Allocate the default command buffer that we will use for rendering
            VkCommandBufferAllocateInfo cmdAllocInfo = VkInit::CommandBufferAllocateInfo(m_Frame.CommandPool, 1);

            VK_CHECK(vkAllocateCommandBuffers(m_Device, &cmdAllocInfo, &m_Frame.MainCommandBuffer));
        }

        VK_CHECK(vkCreateCommandPool(m_Device, &commandPoolInfo, nullptr, &m_ImmCommandPool));

        // allocate the command buffer for immediate submits
        VkCommandBufferAllocateInfo cmdAllocInfo = VkInit::CommandBufferAllocateInfo(m_ImmCommandPool, 1);

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
        VkFenceCreateInfo fenceCreateInfo = VkInit::FenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
        VkSemaphoreCreateInfo semaphoreCreateInfo = VkInit::SemaphoreCreateInfo();

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

        m_GlobalDescriptorAllocator.InitPool(m_Device, 10, sizes);

        {
            DescriptorLayoutBuilder builder;
            builder.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
            m_DrawImageDescriptorLayout = builder.Build(m_Device, VK_SHADER_STAGE_COMPUTE_BIT);
        }

        {
            DescriptorLayoutBuilder builder;
            builder.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
            m_GpuSceneDataDescriptorLayout = builder.Build(m_Device, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
        }

        // Allocate a descriptor set for our draw image
        m_DrawImageDescriptors = m_GlobalDescriptorAllocator.Allocate(m_Device, m_DrawImageDescriptorLayout);

        DescriptorWriter writer;
        writer.WriteImage(0, m_DrawImage.ImageView, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_GENERAL,
                          VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);

        writer.UpdateSet(m_Device, m_DrawImageDescriptors);

        // Make sure both the descriptor allocator and the new layout get cleaned up properly
        m_MainDeletionQueue.PushFunction([&]()
        {
            m_GlobalDescriptorAllocator.DestroyPool(m_Device);

            vkDestroyDescriptorSetLayout(m_Device, m_DrawImageDescriptorLayout, nullptr);
            vkDestroyDescriptorSetLayout(m_Device, m_SingleImageDescriptorLayout, nullptr);
            vkDestroyDescriptorSetLayout(m_Device, m_GpuSceneDataDescriptorLayout, nullptr);
        });


        for (int i = 0; i < FRAME_OVERLAP; i++)
        {
            // create a descriptor pool
            std::vector<DescriptorAllocatorGrowable::PoolSizeRatio> frame_sizes = {
                {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 3},
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3},
                {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3},
                {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4},
            };

            m_Frames[i].FrameDescriptors = DescriptorAllocatorGrowable{};
            m_Frames[i].FrameDescriptors.Init(m_Device, 1000, frame_sizes);

            m_MainDeletionQueue.PushFunction([&, i]()
            {
                m_Frames[i].FrameDescriptors.DestroyPools(m_Device);
            });
        }
    }

    void VulkanRenderer::InitPipelines()
    {
        // InitPlainPipeline();
        InitTexturedPipeline();
    }

    void VulkanRenderer::InitPlainPipeline()
    {
        VkShaderModule triangleFragShader;
        if (!VkUtil::LoadShaderModule("../shaders/compiled/plain.frag.spv", m_Device, &triangleFragShader))
        {
            printf("Error when building the triangle fragment shader module \n");
        }
        else
        {
            printf("Triangle fragment shader successfully loaded \n");
        }

        VkShaderModule triangleVertexShader;
        if (!VkUtil::LoadShaderModule("../shaders/compiled/textured.vert.spv", m_Device, &triangleVertexShader))
        {
            printf("Error when building the triangle vertex shader module \n");
        }
        else
        {
            printf("Triangle vertex shader successfully loaded \n");
        }

        VkPushConstantRange bufferRange{};
        bufferRange.offset = 0;
        bufferRange.size = sizeof(GPUDrawPushConstants);
        bufferRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkPipelineLayoutCreateInfo pipelineLayoutInfo = VkInit::PipelineLayoutCreateInfo();
        pipelineLayoutInfo.pPushConstantRanges = &bufferRange;
        pipelineLayoutInfo.pushConstantRangeCount = 1;

        VK_CHECK(vkCreatePipelineLayout(m_Device, &pipelineLayoutInfo, nullptr, &m_PlainPipelineLayout));

        PipelineBuilder pipelineBuilder;

        //use the triangle layout we created
        pipelineBuilder.PipelineLayout = m_PlainPipelineLayout;
        //connecting the vertex and pixel shaders to the pipeline
        pipelineBuilder.SetShaders(triangleVertexShader, triangleFragShader);
        //it will draw triangles
        pipelineBuilder.SetInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        //filled triangles
        pipelineBuilder.SetPolygonMode(VK_POLYGON_MODE_FILL);
        //no backface culling
        pipelineBuilder.SetCullMode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
        //no multisampling
        pipelineBuilder.SetMultisamplingNone();
        //no blending
        pipelineBuilder.DisableBlending();

        pipelineBuilder.EnableDepthTest(true, VK_COMPARE_OP_LESS);

        //connect the image format we will draw into, from draw image
        pipelineBuilder.SetColorAttachmentFormat(m_DrawImage.ImageFormat);
        pipelineBuilder.SetDepthFormat(m_DepthImage.ImageFormat);

        //finally build the pipeline
        m_PlainPipeline = pipelineBuilder.BuildPipeline(m_Device);

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
        if (!VkUtil::LoadShaderModule("../shaders/vulkan/compiled/default_vk.frag.spv", m_Device, &triangleFragShader))
        {
            printf("Error when building the fragment shader module \n");
        }
        else
        {
            printf("Fragment shader successfully loaded \n");
        }

        VkShaderModule triangleVertexShader;
        if (!VkUtil::LoadShaderModule("../shaders/vulkan/compiled/default_vk.vert.spv", m_Device, &triangleVertexShader))
        {
            printf("Error when building the vertex shader module \n");
        }
        else
        {
            printf("Vertex shader successfully loaded \n");
        }

        VkPushConstantRange bufferRange{};
        bufferRange.offset = 0;
        bufferRange.size = sizeof(GPUDrawPushConstants);
        bufferRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        DescriptorLayoutBuilder layoutBuilder;
        layoutBuilder.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);         // Scene data
        layoutBuilder.AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER); // Texture

        m_SingleImageDescriptorLayout = layoutBuilder.Build(m_Device, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

        VkDescriptorSetLayout layouts[] = { m_SingleImageDescriptorLayout,  m_GpuSceneDataDescriptorLayout};

        VkPipelineLayoutCreateInfo pipelineLayoutInfo = VkInit::PipelineLayoutCreateInfo();
        pipelineLayoutInfo.pPushConstantRanges = &bufferRange;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pSetLayouts = layouts;
        pipelineLayoutInfo.setLayoutCount = 2;
        VK_CHECK(vkCreatePipelineLayout(m_Device, &pipelineLayoutInfo, nullptr, &m_TexturedPipelineLayout));

        PipelineBuilder pipelineBuilder;
        //use the triangle layout we created
        pipelineBuilder.PipelineLayout = m_TexturedPipelineLayout;
        //connecting the vertex and pixel shaders to the pipeline
        pipelineBuilder.SetShaders(triangleVertexShader, triangleFragShader);
        //it will draw triangles
        pipelineBuilder.SetInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        //filled triangles
        pipelineBuilder.SetPolygonMode(VK_POLYGON_MODE_FILL);
        //no backface culling
        pipelineBuilder.SetCullMode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
        //no multisampling
        pipelineBuilder.SetMultisamplingNone();
        //no blending
        //pipelineBuilder.disable_blending();
        pipelineBuilder.DisableBlending();
        //no depth testing
        pipelineBuilder.EnableDepthTest(true, VK_COMPARE_OP_GREATER_OR_EQUAL);

        //connect the image format we will draw into, from draw image
        pipelineBuilder.SetColorAttachmentFormat(m_DrawImage.ImageFormat);
        pipelineBuilder.SetDepthFormat(m_DepthImage.ImageFormat);

        //finally build the pipeline
        m_TexturedPipeline = pipelineBuilder.BuildPipeline(m_Device);

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
                                          .format = m_SwapchainImageFormat,
                                          .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
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

    void VulkanRenderer::DestroySwapchain() const
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

        glfwGetWindowSize(Application::Get().GetWindow(), &w, &h);

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
        newSurface.VertexBuffer = VulkanBufferManager::CreateBuffer(m_Allocator, vertexBufferSize,
                                               VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                               VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                                               VMA_MEMORY_USAGE_GPU_ONLY);

        //find the address of the vertex buffer
        VkBufferDeviceAddressInfo deviceAdressInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, .buffer = newSurface.VertexBuffer.Buffer
        };
        newSurface.VertexBufferAddress = vkGetBufferDeviceAddress(m_Device, &deviceAdressInfo);

        //create index buffer
        newSurface.IndexBuffer = VulkanBufferManager::CreateBuffer(m_Allocator, indexBufferSize,
                                              VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                              VMA_MEMORY_USAGE_GPU_ONLY);

        AllocatedBuffer staging = VulkanBufferManager::CreateBuffer(m_Allocator, vertexBufferSize + indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                               VMA_MEMORY_USAGE_CPU_ONLY);

        void* data = staging.Allocation->GetMappedData();

        // copy vertex buffer
        memcpy(data, vertices.data(), vertexBufferSize);
        // copy index buffer
        memcpy(static_cast<char*>(data) + vertexBufferSize, indices.data(), indexBufferSize);

        ImmediateSubmit([&](const VkCommandBuffer cmd)
        {
            VkBufferCopy vertexCopy{0};
            vertexCopy.dstOffset = 0;
            vertexCopy.srcOffset = 0;
            vertexCopy.size = vertexBufferSize;

            vkCmdCopyBuffer(cmd, staging.Buffer, newSurface.VertexBuffer.Buffer, 1, &vertexCopy);

            VkBufferCopy indexCopy{0};
            indexCopy.dstOffset = 0;
            indexCopy.srcOffset = vertexBufferSize;
            indexCopy.size = indexBufferSize;

            vkCmdCopyBuffer(cmd, staging.Buffer, newSurface.IndexBuffer.Buffer, 1, &indexCopy);
        });

        VulkanBufferManager::DestroyBuffer(m_Allocator, staging);

        return newSurface;
    }

    void VulkanRenderer::ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function)
    {
        VK_CHECK(vkResetFences(m_Device, 1, &m_ImmFence));
        VK_CHECK(vkResetCommandBuffer(m_ImmCommandBuffer, 0));

        VkCommandBuffer cmd = m_ImmCommandBuffer;

        VkCommandBufferBeginInfo cmdBeginInfo = VkInit::CommandBufferBeginInfo(
            VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

        function(cmd);

        VK_CHECK(vkEndCommandBuffer(cmd));

        VkCommandBufferSubmitInfo commandBufferSubmitInfo = VkInit::CommandBufferSubmitInfo(cmd);
        VkSubmitInfo2 submit = VkInit::SubmitInfo(&commandBufferSubmitInfo, nullptr, nullptr);

        // submit command buffer to the queue and execute it.
        //  _renderFence will now block until the graphic commands finish execution
        VK_CHECK(vkQueueSubmit2(m_GraphicsQueue, 1, &submit, m_ImmFence));

        VK_CHECK(vkWaitForFences(m_Device, 1, &m_ImmFence, true, 9999999999));
    }

    void VulkanRenderer::DestroyImage(const AllocatedImage& image) const
    {
        vkDestroyImageView(m_Device, image.ImageView, nullptr);
        vmaDestroyImage(m_Allocator, image.Image, image.Allocation);
    }
}
