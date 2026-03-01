#include <iostream>

#include <vector>
#include <limits>
#include <algorithm>
#include <cstdint>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <vulkan_physical_device.h>
#include <vulkan_swap_chain.h>

// EFFECTS: Returns a struct containing relevant swapchain information for the surface
SwapChainSupportDetails get_swap_chain_support_details(VkPhysicalDevice physical_device, VkSurfaceKHR surface) {
    SwapChainSupportDetails swap_chain_support_details{};

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &swap_chain_support_details.surface_capabilities);

    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, nullptr);

    if (format_count != 0) {
        swap_chain_support_details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, swap_chain_support_details.formats.data());
    }

    uint32_t present_modes_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_modes_count, nullptr);

    if (present_modes_count != 0) {
        swap_chain_support_details.present_modes.resize(present_modes_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_modes_count, swap_chain_support_details.present_modes.data());
    }

    return swap_chain_support_details;
}

// EFFECTS: Returns the best swap chain surface format
VkSurfaceFormatKHR get_best_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& formats) {
    for (const VkSurfaceFormatKHR& cur_format : formats) {
        if (cur_format.format == VK_FORMAT_B8G8R8A8_SRGB && cur_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return cur_format;
        }
    }

    return formats[0];
}

// EFFECTS: Returns the best swap chain present mode
VkPresentModeKHR get_best_swap_present_mode(const std::vector<VkPresentModeKHR>& present_modes) {
    for (const VkPresentModeKHR& cur_present_mode : present_modes) {
        if (cur_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return cur_present_mode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

// EFFECTS: 
VkExtent2D choose_swap_excent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& surface_capabilities) {
    if (surface_capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return surface_capabilities.currentExtent;
    }
    
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    VkExtent2D actual_extent = {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
    };

    actual_extent.width = std::clamp(actual_extent.width, surface_capabilities.minImageExtent.width, surface_capabilities.maxImageExtent.width);
    actual_extent.height = std::clamp(actual_extent.height, surface_capabilities.minImageExtent.height, surface_capabilities.maxImageExtent.height);

    return actual_extent;
}

// MODIFIES: swap_chain, swap_chain_images, swap_chain_image_format, swap_chain_extent
// Effects: Creates swap chain and array of swap chain images
// Sets the swap chain's format and extent
void create_swap_chain(
    GLFWwindow* window, VkSurfaceKHR surface, 
    VkPhysicalDevice physical_device, VkDevice logical_device, 
    QueueFamilyIndices queue_family_indices, 
    VkSwapchainKHR& swap_chain, std::vector<VkImage>& swap_chain_images,
    VkFormat& swap_chain_image_format, VkExtent2D& swap_chain_extent
) {
    SwapChainSupportDetails swap_chain_details = get_swap_chain_support_details(physical_device, surface);

    VkSurfaceFormatKHR surface_format = get_best_swap_surface_format(swap_chain_details.formats);
    VkPresentModeKHR present_mode = get_best_swap_present_mode(swap_chain_details.present_modes);
    VkExtent2D swap_extent = choose_swap_excent(window, swap_chain_details.surface_capabilities);

    uint32_t image_count = swap_chain_details.surface_capabilities.minImageCount + 1;
    if (swap_chain_details.surface_capabilities.maxImageCount > 0 && image_count > swap_chain_details.surface_capabilities.maxImageCount) {
        image_count = swap_chain_details.surface_capabilities.maxImageCount;
    }


    VkSwapchainCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.pNext = nullptr;
    create_info.surface = surface;

    create_info.minImageCount = image_count;

    create_info.imageFormat = surface_format.format;
    create_info.imageColorSpace = surface_format.colorSpace;

    create_info.imageExtent = swap_extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t p_queue_family_indices[] = {queue_family_indices.graphics_family_index.value(), queue_family_indices.present_family_index.value()};
    if (queue_family_indices.graphics_family_index.value() != queue_family_indices.present_family_index.value()) {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = p_queue_family_indices;
    } else {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
            
    create_info.preTransform = swap_chain_details.surface_capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = present_mode;
    create_info.clipped = VK_TRUE;

    create_info.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(logical_device, &create_info, nullptr, &swap_chain) != VK_SUCCESS) {
        throw std::runtime_error("create_swap_chain(): Failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(logical_device, swap_chain, &image_count, nullptr);
    swap_chain_images.resize(image_count);
    vkGetSwapchainImagesKHR(logical_device, swap_chain, &image_count, swap_chain_images.data());

    swap_chain_image_format = surface_format.format;
    swap_chain_extent = swap_extent;
}