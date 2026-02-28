#include <iostream>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <vulkan_handle.h>

#include <vulkan_physical_device.h>
#include <vulkan_validation_layers.h>

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

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
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
    init_vulkan();
    if (enable_validation_layers) {
        setup_debug_messenger(vk_instance, debug_messenger);
    }

    setup_physical_device(vk_instance, physical_device);
}

VulkanHandle::~VulkanHandle() {
    if (enable_validation_layers) {
        destroy_debug_messenger(vk_instance, debug_messenger);
    }

    vkDestroyInstance(vk_instance, nullptr);
}