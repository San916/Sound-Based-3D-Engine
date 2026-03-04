#include <iostream>
#include <cstdint>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <vulkan_handle.h>

#include <vulkan_command_buffer.h>
#include <vulkan_frame_buffer.h>
#include <vulkan_graphics_pipeline.h>
#include <vulkan_logical_device.h>
#include <vulkan_physical_device.h>
#include <vulkan_swap_chain.h>
#include <vulkan_uniform_buffer.h>
#include <vulkan_validation_layers.h>
#include <vulkan_window.h>

#ifdef NDEBUG
    const bool enable_validation_layers = false;
#else
    const bool enable_validation_layers = true;
#endif

#define MAX_FRAMES_IN_FLIGHT 2

void VulkanHandle::init_vulkan() {
    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "3D Engine";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_3;


    VkInstanceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;

    uint32_t glfw_extension_count = 0;
    const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

    std::vector<const char*> extensions(glfw_extensions, glfw_extensions + glfw_extension_count);
    VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};
    if (enable_validation_layers) {
        add_validation_layers(create_info, extensions, debug_create_info);
    }

    create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    create_info.ppEnabledExtensionNames = extensions.data();


    if (vkCreateInstance(&create_info, nullptr, &vk_instance) != VK_SUCCESS) {
        throw std::runtime_error("init_vulkan(): Failed to create VkInstance!");
    }
}

void VulkanHandle::create_sync_objects() {
    frame_fences.resize(MAX_FRAMES_IN_FLIGHT);
    acquire_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
    render_semaphores.resize(swap_chain_images.size());

    VkSemaphoreCreateInfo semaphore_create_info{};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphore_create_info.pNext = nullptr;

    VkFenceCreateInfo fence_create_info{};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_create_info.pNext = nullptr;
    fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(logical_device, &semaphore_create_info, nullptr, &acquire_semaphores[i]) != VK_SUCCESS ||
            vkCreateFence(logical_device, &fence_create_info, nullptr, &frame_fences[i]) != VK_SUCCESS) {
            throw std::runtime_error("create_sync_objects(): Failed to create synchronization objects!");
        }
    }

    for (size_t i = 0; i < swap_chain_images.size(); i++) {
        if (vkCreateSemaphore(logical_device, &semaphore_create_info, nullptr, &render_semaphores[i]) != VK_SUCCESS) {
            throw std::runtime_error("create_sync_objects(): Failed to create synchronization objects!");
        }
    }
}

void VulkanHandle::draw_frame() {
    VkFence frame_fence = frame_fences[frame_index];
    vkWaitForFences(logical_device, 1, &frame_fence, VK_TRUE, UINT64_MAX);
    vkResetFences(logical_device, 1, &frame_fence);

    uint32_t swap_chain_image_index;
    VkSemaphore acquire_semaphore = acquire_semaphores[frame_index];
    if (vkAcquireNextImageKHR(logical_device, swap_chain, UINT64_MAX, acquire_semaphore, VK_NULL_HANDLE, &swap_chain_image_index) != VK_SUCCESS) {
        throw std::runtime_error("draw_frame(): Failed to acquire next swap chain image!");
    }

    // update_uniform_buffer(camera_uniform_buffers, swap_chain_image_index)

    VkSemaphore render_semaphore = render_semaphores[swap_chain_image_index];

    VkCommandBuffer command_buffer = command_buffers[frame_index];
    vkResetCommandBuffer(command_buffer, 0);
    draw_command_buffer(render_pass, frame_buffers, swap_chain_extent, swap_chain_image_index, graphics_pipeline, command_buffer);

    
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pNext = nullptr;

    VkSemaphore wait_semaphores[] = {acquire_semaphore};
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;

    VkSemaphore signal_semaphores[] = {render_semaphore};
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;

    submit_info.pCommandBuffers = &command_buffer;
    submit_info.commandBufferCount = 1;

    if (vkQueueSubmit(graphics_queue, 1, &submit_info, frame_fence) != VK_SUCCESS) {
        throw std::runtime_error("draw_frame(): Failed to submit draw command buffer!");
    }


    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;

    VkSwapchainKHR swap_chains[] = {swap_chain};
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swap_chains;

    present_info.pImageIndices = &swap_chain_image_index;

    vkQueuePresentKHR(present_queue, &present_info);

    frame_index = (frame_index + 1) % MAX_FRAMES_IN_FLIGHT;
}

VulkanHandle::VulkanHandle() {
    setup_window(window);
    init_vulkan();
    if (enable_validation_layers) {
        setup_debug_messenger(vk_instance, debug_messenger);
    }
    setup_window_surface(vk_instance, window, surface);

    QueueFamilyIndices queue_family_indices;
    setup_physical_device(vk_instance, physical_device, surface, queue_family_indices);
    setup_logical_device(physical_device, logical_device, queue_family_indices, graphics_queue, present_queue);
    create_swap_chain(window, surface, physical_device, logical_device, queue_family_indices, swap_chain, swap_chain_images, swap_chain_image_format, swap_chain_extent);
    create_swap_chain_image_views(logical_device, swap_chain_images, swap_chain_image_views, swap_chain_image_format);
    create_descriptor_set_layout_camera(logical_device, descriptor_set_layout_camera);
    create_graphics_pipeline(logical_device, swap_chain_extent, swap_chain_image_format, pipeline_layout, render_pass, descriptor_set_layout_camera, graphics_pipeline);
    create_frame_buffers(logical_device, swap_chain_image_views, swap_chain_extent, render_pass, frame_buffers);
    create_command_pool(logical_device, queue_family_indices.graphics_family_index.value(), command_pool);
    create_uniform_buffers(
        logical_device, physical_device, MAX_FRAMES_IN_FLIGHT, 
        uniform_buffers, uniform_buffers_memory, uniform_buffers_mapped
    );
    create_command_buffers(logical_device, command_pool, command_buffers, MAX_FRAMES_IN_FLIGHT);
    create_sync_objects();
}
VulkanHandle::~VulkanHandle() {
    if (enable_validation_layers) {
        destroy_debug_messenger(vk_instance, debug_messenger);
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(logical_device, acquire_semaphores[i], nullptr);
        vkDestroyFence(logical_device, frame_fences[i], nullptr);
    }
    for (size_t i = 0; i < swap_chain_images.size(); i++) {
        vkDestroySemaphore(logical_device, render_semaphores[i], nullptr);    
    }

    vkDestroyPipeline(logical_device, graphics_pipeline, nullptr);
    vkDestroyPipelineLayout(logical_device, pipeline_layout, nullptr);
    vkDestroyRenderPass(logical_device, render_pass, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyBuffer(logical_device, uniform_buffers[i], nullptr);
        vkFreeMemory(logical_device, uniform_buffers_memory[i], nullptr);
    }

    vkDestroyDescriptorSetLayout(logical_device, descriptor_set_layout_camera, nullptr);
    vkDestroyCommandPool(logical_device, command_pool, nullptr);

    for (VkFramebuffer frame_buffer : frame_buffers) {
        vkDestroyFramebuffer(logical_device, frame_buffer, nullptr);
    }


    for (VkImageView image_view : swap_chain_image_views) {
        vkDestroyImageView(logical_device, image_view, nullptr);
    }

    vkDestroySwapchainKHR(logical_device, swap_chain, nullptr);
    vkDestroyDevice(logical_device, nullptr);
    vkDestroySurfaceKHR(vk_instance, surface, nullptr);
    vkDestroyInstance(vk_instance, nullptr);

    glfwDestroyWindow(window);
    glfwTerminate();
}

void VulkanHandle::run() {
    while(!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        glfwPollEvents();
        draw_frame();
    }
    vkDeviceWaitIdle(logical_device);
}