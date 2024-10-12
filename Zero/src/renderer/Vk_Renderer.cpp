#include "Vk_Renderer.h"

#include "core/Engine.h"

#include <shared/vk_initializers.h>
#include <shared/vk_types.h>

#include "VkBootstrap.h"

#include <chrono>
#include <thread>
#include <iostream>
#include <shared/vk_images.h>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#ifdef NDEBUG
constexpr bool bUseValidationLayers = false;
#else
constexpr bool bUseValidationLayers = true;
#endif

void Vk_Renderer::Init()
{
    printf("ZeroEngine Vulkan \n");

    init_vulkan();
    init_swapchain();
    init_commands();
    init_sync_structures();
}

void Vk_Renderer::Draw()
{
    // Wait for the render fence: 
    // This ensures that the previous frame's rendering is complete before starting the next frame.
    VK_CHECK(vkWaitForFences(_device, 1, &get_current_frame()._renderFence, true, 1000000000));

    // The deletion queue is flushed to clean up any resources that were marked for deletion in the previous frame.
    get_current_frame()._deletionQueue.flush();

    // The fence used for rendering is reset using vkResetFences() to prepare it for the current frame.
    VK_CHECK(vkResetFences(_device, 1, &get_current_frame()._renderFence));

    // Retrieve the index of the image that will be rendered to in the current frame.
    uint32_t swapchainImageIndex;
    VK_CHECK(vkAcquireNextImageKHR(_device, _swapchain, 1000000000, get_current_frame()._swapchainSemaphore, nullptr, &swapchainImageIndex));

    VkCommandBuffer cmd = get_current_frame()._mainCommandBuffer;

    VK_CHECK(vkResetCommandBuffer(cmd, 0));

    // This informs Vulkan that the command buffer will be used only once for this frame.
    VkCommandBufferBeginInfo cmdBeginInfo = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    _drawExtent.width = _drawImage.imageExtent.width;
    _drawExtent.height = _drawImage.imageExtent.height;

    // The command buffer recording begins by calling vkBeginCommandBuffer().
    VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

    // The main draw image is transitioned into the general layout using vkutil::transition_image().This allows writing into the image.
    vkutil::transition_image(cmd, _drawImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

    // The DrawBackground() function is called to clear the draw image with a specified clear color.
    DrawBackground(cmd);

    // Both the draw image and the swapchain image are transitioned into their respective transfer layouts using vkutil::transition_image(). 
    // This prepares them for the copy operation.
    vkutil::transition_image(cmd, _drawImage.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    vkutil::transition_image(cmd, _swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    // The vkutil::copy_image_to_image() function is called to copy the contents of the draw image to the swapchain image. 
    // This effectively renders the frame onto the swapchain image.
    vkutil::copy_image_to_image(cmd, _drawImage.image, _swapchainImages[swapchainImageIndex], _drawExtent, _swapchainExtent);

    // The swapchain image layout is set to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR using vkutil::transition_image(). 
    // This prepares the image to be presented on the screen.
    vkutil::transition_image(cmd, _swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    // The command buffer recording is finalized using vkEndCommandBuffer(). 
    // At this point, no more commands can be added to the buffer, but it is ready for execution.
    VK_CHECK(vkEndCommandBuffer(cmd));

    // The command buffer is submitted to the graphics queue for execution using vkQueueSubmit2().
    // The _renderFence is used to block until the graphic commands finish execution.
    VkCommandBufferSubmitInfo cmdinfo = vkinit::command_buffer_submit_info(cmd);

    VkSemaphoreSubmitInfo waitInfo = vkinit::semaphore_submit_info(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, get_current_frame()._swapchainSemaphore);
    VkSemaphoreSubmitInfo signalInfo = vkinit::semaphore_submit_info(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, get_current_frame()._renderSemaphore);

    VkSubmitInfo2 submit = vkinit::submit_info(&cmdinfo, &signalInfo, &waitInfo);

    VK_CHECK(vkQueueSubmit2(_graphicsQueue, 1, &submit, get_current_frame()._renderFence));

    // The present operation is prepared by setting up the VkPresentInfoKHR structure. 
    // The _renderSemaphore is waited upon to ensure that the drawing commands have finished before presenting the image to the user.
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;
    presentInfo.pSwapchains = &_swapchain;
    presentInfo.swapchainCount = 1;

    presentInfo.pWaitSemaphores = &get_current_frame()._renderSemaphore;
    presentInfo.waitSemaphoreCount = 1;

    // The image index of the swapchain image is specified in the VkPresentInfoKHR structure.
    presentInfo.pImageIndices = &swapchainImageIndex;

    // The image is presented to the screen by calling vkQueuePresentKHR().
    VK_CHECK(vkQueuePresentKHR(_graphicsQueue, &presentInfo));

    _frameNumber++;
}

void Vk_Renderer::DrawBackground(VkCommandBuffer cmd)
{
    VkClearColorValue clearValue{ _clearColor.r, _clearColor.g, _clearColor.b, _clearColor.a };
    VkImageSubresourceRange clearRange = vkinit::image_subresource_range(VK_IMAGE_ASPECT_COLOR_BIT);
    vkCmdClearColorImage(cmd, _drawImage.image, VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);
}

void Vk_Renderer::Shutdown()
{			
    vkDeviceWaitIdle(_device);

    // Free per-frame structures and deletion queue
    for (int i = 0; i < FRAME_OVERLAP; i++) {

        vkDestroyCommandPool(_device, _frames[i]._commandPool, nullptr);

        // Destroy sync objects
        vkDestroyFence(_device, _frames[i]._renderFence, nullptr);
        vkDestroySemaphore(_device, _frames[i]._renderSemaphore, nullptr);
        vkDestroySemaphore(_device, _frames[i]._swapchainSemaphore, nullptr);

        _frames[i]._deletionQueue.flush();
    }

    // Flush the global deletion queue
    _mainDeletionQueue.flush();

    destroy_swapchain();

    vkDestroySurfaceKHR(_instance, _surface, nullptr);
    vkDestroyDevice(_device, nullptr);

    vkb::destroy_debug_utils_messenger(_instance, _debug_messenger);
    vkDestroyInstance(_instance, nullptr);
}

void Vk_Renderer::init_vulkan()
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
    _instance = vkb_inst.instance;
    _debug_messenger = vkb_inst.debug_messenger;

    // Create the surface
    VK_CHECK(glfwCreateWindowSurface(_instance, Engine::Get().GetWindow(), nullptr, &_surface));

    //vulkan 1.3 features
    VkPhysicalDeviceVulkan13Features features{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
    features.dynamicRendering = true;
    features.synchronization2 = true;

    //vulkan 1.2 features
    VkPhysicalDeviceVulkan12Features features12{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
    features12.bufferDeviceAddress = true;
    features12.descriptorIndexing = true;

    // Use vkbootstrap to select a gpu. 
    // We want a gpu that can write to the surface and supports vulkan 1.3 with the correct features
    vkb::PhysicalDeviceSelector selector{ vkb_inst };
    vkb::PhysicalDevice physicalDevice = selector
        .set_minimum_version(1, 3)
        .set_required_features_13(features)
        .set_required_features_12(features12)
        .set_surface(_surface)
        .select()
        .value();

    //create the final vulkan device
    vkb::DeviceBuilder deviceBuilder{ physicalDevice };

    vkb::Device vkbDevice = deviceBuilder.build().value();

    // Store the VkDevice handle used in the rest of a vulkan application
    _device = vkbDevice.device;
    _physicalDevice = physicalDevice.physical_device;

    VkPhysicalDeviceProperties Properties{};

    vkGetPhysicalDeviceProperties(_physicalDevice, &Properties);
    std::cout << "Chosen GPU: " << "\n";
    std::cout << "\t" << Properties.deviceName << "\n";

    // use vkbootstrap to get a Graphics queue
    _graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
    _graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

    // initialize the memory allocator
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = _physicalDevice;
    allocatorInfo.device = _device;
    allocatorInfo.instance = _instance;
    allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    vmaCreateAllocator(&allocatorInfo, &_allocator);

    _mainDeletionQueue.push_function([&]() {
        vmaDestroyAllocator(_allocator);
        });
}

void Vk_Renderer::init_swapchain()
{
    create_swapchain(_windowExtent.width, _windowExtent.height);

    // Define the drawImageExtent variable, which represents the extent (size) of the image that will be used for drawing. 
    // The width and height of the image are set to _windowExtent.width and _windowExtent.height, respectively. 
    // The 1 represents the depth of the image, which is set to 1 because it's a 2D image.
    VkExtent3D drawImageExtent = {
        _windowExtent.width,
        _windowExtent.height,
        1
    };

    // Set the format of the drawImage to VK_FORMAT_R16G16B16A16_SFLOAT, 
    // which represents a 16-bit floating-point format with 4 color channels (red, green, blue, and alpha).
    _drawImage.imageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
    // Set the extent of the drawImage to the previously defined drawImageExtent.
    _drawImage.imageExtent = drawImageExtent;

    // Define the drawImageUsages variable, which represents the usage flags for the drawImage. 
    // These flags specify how the image will be used, 
    // such as for transferring data, storing data, or as a color attachment for rendering.
    VkImageUsageFlags drawImageUsages{};
    drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
    drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // Create a VkImageCreateInfo structure rimg_info with the specified format, usage flags, and extent for the drawImage.
    VkImageCreateInfo rimg_info = vkinit::image_create_info(_drawImage.imageFormat, drawImageUsages, drawImageExtent);

    VmaAllocationCreateInfo rimg_allocinfo = {};
    rimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    rimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vmaCreateImage(_allocator, &rimg_info, &rimg_allocinfo, &_drawImage.image, &_drawImage.allocation, nullptr);

    // Create a VkImageViewCreateInfo structure rview_info to specify the format, image, and aspect (how the image will be viewed) for the drawImage.
    VkImageViewCreateInfo rview_info = vkinit::imageview_create_info(_drawImage.imageFormat, _drawImage.image, VK_IMAGE_ASPECT_COLOR_BIT);


    // Calls vkCreateImageView() to create an image view for the drawImage using the specified image view info.
    // The image view is stored in the _drawImage.imageView variable.
    VK_CHECK(vkCreateImageView(_device, &rview_info, nullptr, &_drawImage.imageView));

    // Push a function to the _mainDeletionQueue to destroy the image view 
    // and deallocate the memory for the drawImage when the renderer is shut down.
    _mainDeletionQueue.push_function([=]() {
        vkDestroyImageView(_device, _drawImage.imageView, nullptr);
        vmaDestroyImage(_allocator, _drawImage.image, _drawImage.allocation);
        });
}

void Vk_Renderer::init_commands()
{
    // Create a command pool for commands submitted to the graphics queue.
    // We also want the pool to allow for resetting of individual command buffers
    VkCommandPoolCreateInfo commandPoolInfo = vkinit::command_pool_create_info(
        _graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    for (int i = 0; i < FRAME_OVERLAP; i++)
    {
        VK_CHECK(vkCreateCommandPool(_device, &commandPoolInfo, nullptr, &_frames[i]._commandPool));

        // Allocate the default command buffer that we will use for rendering
        VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::command_buffer_allocate_info(_frames[i]._commandPool, 1);

        VK_CHECK(vkAllocateCommandBuffers(_device, &cmdAllocInfo, &_frames[i]._mainCommandBuffer));
    }
}

void Vk_Renderer::init_sync_structures()
{
    // Create syncronization structures
    // One fence to control when the gpu has finished rendering the frame,
    // And 2 semaphores to syncronize rendering with swapchain
    // We want the fence to start signalled so we can wait on it on the first frame
    VkFenceCreateInfo fenceCreateInfo = vkinit::fence_create_info(VK_FENCE_CREATE_SIGNALED_BIT);
    VkSemaphoreCreateInfo semaphoreCreateInfo = vkinit::semaphore_create_info();

    for (int i = 0; i < FRAME_OVERLAP; i++) {
        VK_CHECK(vkCreateFence(_device, &fenceCreateInfo, nullptr, &_frames[i]._renderFence));

        VK_CHECK(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_frames[i]._swapchainSemaphore));
        VK_CHECK(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_frames[i]._renderSemaphore));
    }
}

void Vk_Renderer::create_swapchain(uint32_t width, uint32_t height)
{
    vkb::SwapchainBuilder swapchainBuilder(_physicalDevice, _device, _surface);

    _swapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

    // Building the swapchain
    // The most important detail here is the present mode, which we have set to VK_PRESENT_MODE_FIFO_KHR.
    // This way we are doing a hard VSync, which will limit the FPS of the entire engine to the refresh-rate of the monitor.

    vkb::Swapchain vkbSwapchain = swapchainBuilder
        //.use_default_format_selection()
        .set_desired_format(VkSurfaceFormatKHR{
            .format = _swapchainImageFormat, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
            })
        // Use vsync present mode
        .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
        .set_desired_extent(width, height)
        .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
        .build()
        .value();

    _swapchainExtent = vkbSwapchain.extent;
    // Store swapchain and its related images
    _swapchain = vkbSwapchain.swapchain;
    _swapchainImages = vkbSwapchain.get_images().value();
    _swapchainImageViews = vkbSwapchain.get_image_views().value();
}

void Vk_Renderer::destroy_swapchain()
{
    vkDestroySwapchainKHR(_device, _swapchain, nullptr);

    // Destroy swapchain resources
    for (auto& _swapchainImageView : _swapchainImageViews)
    {
        vkDestroyImageView(_device, _swapchainImageView, nullptr);
    }
}
