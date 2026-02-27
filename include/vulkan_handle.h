#ifndef VULKAN_HANDLE_H
#define VULKAN_HANDLE_H

#include <vector>

typedef VkInstance VkInstance;

class VulkanHandle {
private:
    VkInstance vk_instance;
    VkDebugUtilsMessengerEXT debug_messenger;

    void init_vulkan();
public:
    VulkanHandle();
    VulkanHandle(const VulkanHandle&) = delete;
    VulkanHandle& operator=(const VulkanHandle&) = delete;
    ~VulkanHandle();
};

#endif