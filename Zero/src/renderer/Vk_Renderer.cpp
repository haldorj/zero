#include "Vk_Renderer.h"

#include "core/Engine.h"

#include <shared/vk_initializers.h>
#include <shared/vk_types.h>

#include "VkBootstrap.h"

#include <chrono>
#include <thread>
#include <iostream>
#include <shared/vk_images.h>

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

void Vk_Renderer::Shutdown()
{
    vkDeviceWaitIdle(_device);

    for (int i = 0; i < FRAME_OVERLAP; i++) {

        //already written from before
        vkDestroyCommandPool(_device, _frames[i]._commandPool, nullptr);

        //destroy sync objects
        vkDestroyFence(_device, _frames[i]._renderFence, nullptr);
        vkDestroySemaphore(_device, _frames[i]._renderSemaphore, nullptr);
        vkDestroySemaphore(_device, _frames[i]._swapchainSemaphore, nullptr);
    }

    destroy_swapchain();

    vkDestroySurfaceKHR(_instance, _surface, nullptr);
    vkDestroyDevice(_device, nullptr);

    vkb::destroy_debug_utils_messenger(_instance, _debug_messenger);
    vkDestroyInstance(_instance, nullptr);
}

void Vk_Renderer::Draw()
{
    // We use vkWaitForFences() to wait for the GPU to finish its work.
    // After we reset the fence.
    VK_CHECK(vkWaitForFences(_device, 1, &get_current_frame()._renderFence, true, 1000000000));
    VK_CHECK(vkResetFences(_device, 1, &get_current_frame()._renderFence));

    //request image from the swapchain
    uint32_t swapchainImageIndex;
    VK_CHECK(vkAcquireNextImageKHR(_device, _swapchain, 1000000000, get_current_frame()._swapchainSemaphore, nullptr, &swapchainImageIndex));

    //naming it cmd for shorter writing
    VkCommandBuffer cmd = get_current_frame()._mainCommandBuffer;

    // now that we are sure that the commands finished executing, we can safely
    // reset the command buffer to begin recording again.
    VK_CHECK(vkResetCommandBuffer(cmd, 0));

    //begin the command buffer recording. We will use this command buffer exactly once, so we want to let vulkan know that
    VkCommandBufferBeginInfo cmdBeginInfo = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    //start the command buffer recording
    VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

    //make the swapchain image into writeable mode before rendering
    vkutil::transition_image(cmd, _swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

    //make a clear-color from frame number. This will flash with a 120 frame period.
    VkClearColorValue clearValue{ _clearColor.r, _clearColor.g, _clearColor.b, _clearColor.a };
    VkImageSubresourceRange clearRange = vkinit::image_subresource_range(VK_IMAGE_ASPECT_COLOR_BIT);

    //clear image
    vkCmdClearColorImage(cmd, _swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);

    //make the swapchain image into presentable mode
    vkutil::transition_image(cmd, _swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    //finalize the command buffer (we can no longer add commands, but it can now be executed)
    VK_CHECK(vkEndCommandBuffer(cmd));

    //prepare the submission to the queue. 
    //we want to wait on the _presentSemaphore, as that semaphore is signaled when the swapchain is ready
    //we will signal the _renderSemaphore, to signal that rendering has finished

    VkCommandBufferSubmitInfo cmdinfo = vkinit::command_buffer_submit_info(cmd);

    VkSemaphoreSubmitInfo waitInfo = vkinit::semaphore_submit_info(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, get_current_frame()._swapchainSemaphore);
    VkSemaphoreSubmitInfo signalInfo = vkinit::semaphore_submit_info(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, get_current_frame()._renderSemaphore);

    VkSubmitInfo2 submit = vkinit::submit_info(&cmdinfo, &signalInfo, &waitInfo);

    //submit command buffer to the queue and execute it.
    // _renderFence will now block until the graphic commands finish execution
    VK_CHECK(vkQueueSubmit2(_graphicsQueue, 1, &submit, get_current_frame()._renderFence));

    //prepare present
    // this will put the image we just rendered to into the visible window.
    // we want to wait on the _renderSemaphore for that, 
    // as its necessary that drawing commands have finished before the image is displayed to the user
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;
    presentInfo.pSwapchains = &_swapchain;
    presentInfo.swapchainCount = 1;

    presentInfo.pWaitSemaphores = &get_current_frame()._renderSemaphore;
    presentInfo.waitSemaphoreCount = 1;

    presentInfo.pImageIndices = &swapchainImageIndex;

    VK_CHECK(vkQueuePresentKHR(_graphicsQueue, &presentInfo));

    //increase the number of frames drawn
    _frameNumber++;
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
}

void Vk_Renderer::init_swapchain()
{
    create_swapchain(_windowExtent.width, _windowExtent.height);
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

        // allocate the default command buffer that we will use for rendering
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

    // destroy swapchain resources
    for (auto& _swapchainImageView : _swapchainImageViews)
    {
        vkDestroyImageView(_device, _swapchainImageView, nullptr);
    }
}
