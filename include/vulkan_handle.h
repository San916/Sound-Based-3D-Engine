#ifndef VULKAN_HANDLE_H
#define VULKAN_HANDLE_H

#include <vector>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

class VulkanHandle {
private:
    GLFWwindow* window;
    
    VkInstance vk_instance;
    VkDebugUtilsMessengerEXT debug_messenger;
    VkSurfaceKHR surface;
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    VkDevice logical_device;

    VkQueue graphics_queue;
    VkQueue present_queue;

    VkSwapchainKHR swap_chain;
    std::vector<VkImage> swap_chain_images;
    VkFormat swap_chain_image_format;
    VkExtent2D swap_chain_extent;

    void init_vulkan();
public:
    VulkanHandle();
    VulkanHandle(const VulkanHandle&) = delete;
    VulkanHandle& operator=(const VulkanHandle&) = delete;
    ~VulkanHandle();

    void run();
};

#endif