#ifndef VULKAN_SWAP_CHAIN_H
#define VULKAN_SWAP_CHAIN_H

#include <vector>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

typedef struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR surface_capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;

    bool supports_swap_chain() {
        return !formats.empty() && !present_modes.empty();
    }
} SwapChainSupportDetails;

SwapChainSupportDetails get_swap_chain_support_details(VkPhysicalDevice physical_device, VkSurfaceKHR surface);
void create_swap_chain(
    GLFWwindow* window, VkSurfaceKHR surface, 
    VkPhysicalDevice physical_device, VkDevice logical_device, 
    const QueueFamilyIndices& queue_family_indices, 
    VkSwapchainKHR& swap_chain, std::vector<VkImage>& swap_chain_images,
    VkFormat& swap_chain_image_format, VkExtent2D& swap_chain_extent
);
void create_swap_chain_image_views(
    VkDevice logical_device, 
    const std::vector<VkImage>& swap_chain_images, 
    std::vector<VkImageView>& swap_chain_image_views, 
    VkFormat swap_chain_image_format
);

#endif