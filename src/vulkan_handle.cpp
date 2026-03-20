#include <iostream>
#include <algorithm>
#include <chrono>
#include <cstdint>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <vulkan_handle.h>

#include <vulkan_acceleration_structure.h>
#include <vulkan_command_buffer.h>
#include <vulkan_compute_pipeline.h>
#include <vulkan_descriptor_sets.h>
#include <vulkan_frame_buffer.h>
#include <vulkan_graphics_pipeline.h>
#include <vulkan_index_buffer.h>
#include <vulkan_logical_device.h>
#include <vulkan_object.h>
#include <vulkan_object_loading.h>
#include <vulkan_physical_device.h>
#include <vulkan_scene.h>
#include <vulkan_storage_buffer.h>
#include <vulkan_storage_image.h>
#include <vulkan_swap_chain.h>
#include <vulkan_uniform_buffer.h>
#include <vulkan_validation_layers.h>
#include <vulkan_vertex_buffer.h>
#include <vulkan_window.h>

#ifdef NDEBUG
    const bool enable_validation_layers = false;
#else
    const bool enable_validation_layers = true;
#endif

#define MAX_FRAMES_IN_FLIGHT 2

void VulkanHandle::mouse_callback(GLFWwindow* window, double x_pos, double y_pos) {
    VulkanHandle* handle = static_cast<VulkanHandle*>(glfwGetWindowUserPointer(window));

    float x = static_cast<float>(x_pos);
    float y = static_cast<float>(y_pos);

    if (!handle->initial_mouse_position_set) {
        handle->last_mouse_position = {x, y};
        handle->initial_mouse_position_set = true;
        return;
    }

    const float sensitivity = 0.05f;
    float x_offset = (x - handle->last_mouse_position.x) * sensitivity;
    float y_offset = (handle->last_mouse_position.y - y) * sensitivity;
    handle->last_mouse_position = {x, y};

    handle->camera_rotation.x += x_offset;
    handle->camera_rotation.y += y_offset;

    if (handle->camera_rotation.y >  89.0f) handle->camera_rotation.y =  89.0f;
    if (handle->camera_rotation.y < -89.0f) handle->camera_rotation.y = -89.0f;
}

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

    const std::vector<VulkanObject*> objects = scene->get_objects();
        std::vector<VkAccelerationStructureKHR> blases;
    std::vector<glm::mat4> transforms;
    for (size_t i = 0; i < objects.size(); i++) {
        blases.push_back(objects[i]->get_blas());
        transforms.push_back(objects[i]->get_model_matrix());
    }

    update_uniform_buffer(frame_index, swap_chain_extent, camera_position, camera_rotation, uniform_buffers_mapped);
    update_storage_buffer(frame_index, transforms, sound_waves, storage_buffers_mapped);
    update_top_level_acceleration_structure(
        logical_device, physical_device,
        command_pool, graphics_queue,
        blases, transforms,
        tlases[frame_index]
    );

    VkSemaphore render_semaphore = render_semaphores[swap_chain_image_index];

    VkCommandBuffer command_buffer = command_buffers[frame_index];
    vkResetCommandBuffer(command_buffer, 0);

    VkCommandBuffer compute_command_buffer;
    begin_single_time_command(logical_device, command_pool, compute_command_buffer);
    vkCmdBindPipeline(compute_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipeline);
    vkCmdBindDescriptorSets(compute_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipeline_layout, 0, 1, &compute_descriptor_sets[frame_index], 0, nullptr);
    vkCmdDispatch(compute_command_buffer, swap_chain_extent.width / dispatch_group_size.width, swap_chain_extent.height / dispatch_group_size.height, 1);
    finish_single_time_command(logical_device, graphics_queue, command_pool, compute_command_buffer);

    draw_command_buffer(
        render_pass, frame_buffers, swap_chain_extent,
        swap_chain_image_index, frame_index, graphics_pipeline_layout,
        graphics_descriptor_sets, objects,
        graphics_pipeline, command_buffer
    );


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
    setup_window(window, this, mouse_callback);
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
    create_graphics_descriptor_set_layout(logical_device, graphics_descriptor_set_layout);
    create_compute_descriptor_set_layout(logical_device, compute_descriptor_set_layout);
    create_graphics_pipeline(logical_device, swap_chain_extent, swap_chain_image_format, render_pass, graphics_descriptor_set_layout, graphics_pipeline_layout, graphics_pipeline);
    create_compute_pipeline(logical_device, compute_descriptor_set_layout, compute_pipeline_layout, compute_pipeline);
    create_frame_buffers(logical_device, swap_chain_image_views, swap_chain_extent, render_pass, frame_buffers);
    create_command_pool(logical_device, queue_family_indices.graphics_family_index.value(), command_pool);
    create_storage_image(
        logical_device, physical_device, 
        command_pool, graphics_queue, 
        swap_chain_extent, 
        storage_image, storage_image_memory, 
        storage_image_view, storage_image_format
    );

    scene = new Scene("./../assets/scenes/scene.txt", logical_device, physical_device, command_pool, graphics_queue);
    const std::vector<VulkanObject*> objects = scene->get_objects();

    std::vector<VkAccelerationStructureKHR> blases;
    std::vector<glm::mat4> transforms;
    for (size_t i = 0; i < objects.size(); i++) {
        blases.push_back(objects[i]->get_blas());
        transforms.push_back(objects[i]->get_model_matrix());
    }

    tlas_buffer.resize(MAX_FRAMES_IN_FLIGHT);
    tlas_buffer_memory.resize(MAX_FRAMES_IN_FLIGHT);
    tlases.resize(MAX_FRAMES_IN_FLIGHT);
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        create_top_level_acceleration_structure(
            logical_device, physical_device,
            command_pool, graphics_queue,
            blases, transforms,
            tlas_buffer[i], tlas_buffer_memory[i], tlases[i]
        );
    }

    create_uniform_buffers(
        logical_device, physical_device, MAX_FRAMES_IN_FLIGHT, 
        uniform_buffers, uniform_buffers_memory, uniform_buffers_mapped
    );
    create_storage_buffers(
        logical_device, physical_device, MAX_FRAMES_IN_FLIGHT, 
        storage_buffers, storage_buffers_memory, storage_buffers_mapped
    );
    create_graphics_descriptor_pool(logical_device, MAX_FRAMES_IN_FLIGHT, graphics_descriptor_pool);
    create_compute_descriptor_pool(logical_device, MAX_FRAMES_IN_FLIGHT, compute_descriptor_pool);
    create_graphics_descriptor_sets(
        logical_device, 
        MAX_FRAMES_IN_FLIGHT, 
        graphics_descriptor_pool, 
        storage_image_sampler, 
        storage_image_view, 
        uniform_buffers, 
        storage_buffers,
        graphics_descriptor_set_layout, 
        graphics_descriptor_sets
    );
    create_compute_descriptor_sets(
        logical_device,
        MAX_FRAMES_IN_FLIGHT,
        compute_descriptor_pool,
        storage_image_view,
        tlases,
        uniform_buffers,
        storage_buffers,
        compute_descriptor_set_layout,
        compute_descriptor_sets
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

    vkDestroyPipeline(logical_device, compute_pipeline, nullptr);
    vkDestroyPipelineLayout(logical_device, compute_pipeline_layout, nullptr);

    vkDestroyPipeline(logical_device, graphics_pipeline, nullptr);
    vkDestroyPipelineLayout(logical_device, graphics_pipeline_layout, nullptr);
    vkDestroyRenderPass(logical_device, render_pass, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        cleanup_acceleration_structure(logical_device, tlas_buffer[i], tlas_buffer_memory[i], tlases[i]);
    }

    cleanup_storage_image(logical_device, storage_image, storage_image_memory, storage_image_view);

    delete scene;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyBuffer(logical_device, uniform_buffers[i], nullptr);
        vkFreeMemory(logical_device, uniform_buffers_memory[i], nullptr);
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyBuffer(logical_device, storage_buffers[i], nullptr);
        vkFreeMemory(logical_device, storage_buffers_memory[i], nullptr);
    }

    vkDestroyDescriptorPool(logical_device, compute_descriptor_pool, nullptr);
    vkDestroyDescriptorSetLayout(logical_device, compute_descriptor_set_layout, nullptr);
    vkDestroyDescriptorPool(logical_device, graphics_descriptor_pool, nullptr);
    vkDestroyDescriptorSetLayout(logical_device, graphics_descriptor_set_layout, nullptr);
    vkDestroyCommandPool(logical_device, command_pool, nullptr);
    
    vkDestroySampler(logical_device, storage_image_sampler, nullptr);

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
    auto previous_time = std::chrono::high_resolution_clock::now();
    while(!glfwWindowShouldClose(window)) {
        auto current_time = std::chrono::high_resolution_clock::now();
        float delta_time = std::chrono::duration<float>(current_time - previous_time).count();
        previous_time = current_time;

        const bool flying = true;

        glm::vec3 forward;
        glm::vec3 right;

        if (flying) {
            forward.x = cos(glm::radians(camera_rotation.x)) * cos(glm::radians(camera_rotation.y));
            forward.y = sin(glm::radians(camera_rotation.y));
            forward.z = sin(glm::radians(camera_rotation.x)) * cos(glm::radians(camera_rotation.y));
            right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
        } else {
            forward.x = cos(glm::radians(camera_rotation.x));
            forward.y = 0;
            forward.z = sin(glm::radians(camera_rotation.x));
            right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
        }


        const float move_speed = delta_time * 2.0f;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            camera_position += forward * move_speed;
        }

        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            camera_position -= forward * move_speed;
        }

        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            camera_position += right * move_speed;
        }

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            camera_position -= right * move_speed;
        }

        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            if (flying) {
                camera_position.y += move_speed;
            }
        }

        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
            if (flying) {
                camera_position.y -= move_speed;
            }
        }

        for  (size_t i = 0; i < sound_waves.size(); i++) {
            sound_waves[i].w += delta_time * 6.0f;
        }
        sound_waves.erase(
            std::remove_if(sound_waves.begin(), sound_waves.end(), 
                [&](const glm::vec4 sound_wave) { return sound_wave.w > 20.f; }
            ),
            sound_waves.end()
        );

        bool q_pressed = glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS;
        if (q_pressed && !q_held_down) {
            if (sound_waves.size() < MAX_SOUND_WAVES) {
                sound_waves.push_back(glm::vec4(camera_position, 0.0f));
            }
        }
        q_held_down = q_pressed;

        glfwPollEvents();
        draw_frame();
    }
    vkDeviceWaitIdle(logical_device);
}