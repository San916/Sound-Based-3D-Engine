#ifndef VULKAN_HANDLE_H
#define VULKAN_HANDLE_H

#include <vector>

#include <vulkan/vulkan.h>

class VulkanHandle {
private:
    VkInstance vk_instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    VkDevice logical_device = VK_NULL_HANDLE;

    void init_vulkan();
public:
    VulkanHandle();
    VulkanHandle(const VulkanHandle&) = delete;
    VulkanHandle& operator=(const VulkanHandle&) = delete;
    ~VulkanHandle();
};

#endif