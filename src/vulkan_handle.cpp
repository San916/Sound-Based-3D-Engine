#include <iostream>
#include <cstdint>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <vulkan_handle.h>

#include <vulkan_logical_device.h>
#include <vulkan_physical_device.h>
#include <vulkan_swap_chain.h>
#include <vulkan_validation_layers.h>
#include <vulkan_window.h>

#ifdef NDEBUG
    const bool enable_validation_layers = false;
#else
    const bool enable_validation_layers = true;
#endif

void VulkanHandle::init_vulkan() {
    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "3D Engine";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_4;


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
}

VulkanHandle::~VulkanHandle() {
    if (enable_validation_layers) {
        destroy_debug_messenger(vk_instance, debug_messenger);
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
    }
}